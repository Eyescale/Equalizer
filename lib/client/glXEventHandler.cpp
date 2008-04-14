/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glXEventHandler.h"

#include "commands.h"
#include "event.h"
#include "global.h"
#include "log.h"
#include "packets.h"
#include "pipe.h"
#include "window.h"
#include "windowEvent.h"
#include "X11Connection.h"

#include <X11/keysym.h>

using namespace eqBase;
using namespace std;

namespace eq
{
namespace
{
static PerThread< eqNet::ConnectionSet* > _pipeConnections;
}

GLXEventHandler::GLXEventHandler( Pipe* pipe )
{
    if( _pipeConnections == 0 )
        _pipeConnections = new eqNet::ConnectionSet;

    _pipeConnections->addConnection( new X11Connection( pipe ));
}

GLXEventHandler::~GLXEventHandler()
{
}

void GLXEventHandler::deregisterPipe( Pipe* pipe )
{
    EQASSERT( _pipeConnections.get( ));

    const eqNet::ConnectionVector& connections = 
        _pipeConnections->getConnections();

    for( eqNet::ConnectionVector::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        X11ConnectionPtr connection = 
            RefPtr_static_cast< eqNet::Connection, X11Connection >( *i );
        
        if( connection->pipe == pipe )
        {
            _pipeConnections->removeConnection( *i );
            break;
        }
    }

    delete this;
}

void GLXEventHandler::registerWindow( Window* window )
{
    Pipe*    pipe      = window->getPipe();
    Display* display   = pipe->getXDisplay();
    XID      drawable  = window->getXDrawable();
    long     eventMask = StructureNotifyMask | ExposureMask | KeyPressMask |
                         KeyReleaseMask | PointerMotionMask | ButtonPressMask |
                         ButtonReleaseMask;

    EQASSERT( display );
    EQASSERT( drawable );

    EQINFO << "Start collecting events for window @" << (void*)window << "('"
           << window->getName() << "')" << endl;
    
    XSelectInput( display, drawable, eventMask );

    // Grab keyboard focus in fullscreen mode
    if( window->getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
        XGrabKeyboard( display, drawable, True, GrabModeAsync, GrabModeAsync, 
                       CurrentTime );

    XFlush( display );
}

void GLXEventHandler::deregisterWindow( Window* window )
{
    EQINFO << "Stop collecting events for window  " << window->getName()
           << endl;
    
    Pipe*    pipe      = window->getPipe();
    Display* display   = pipe->getXDisplay();
    XID      drawable  = window->getXDrawable();

    EQASSERT( display );
    EQASSERT( drawable );

    XSelectInput( display, drawable, 0l );
    XFlush( display );
}

eqNet::ConnectionSet* GLXEventHandler::getEventSet()
{
    return _pipeConnections.get();
}

void GLXEventHandler::dispatchOne()
{
    if( _pipeConnections == 0 )
        _pipeConnections = new eqNet::ConnectionSet;

    eqNet::ConnectionSet* connections = _pipeConnections.get();

    const eqNet::ConnectionSet::Event event = connections->select( );
    switch( event )
    {
        case eqNet::ConnectionSet::EVENT_DISCONNECT:
        {
            RefPtr<eqNet::Connection> connection = connections->getConnection();
            connections->removeConnection( connection );
            EQERROR << "Display connection shut down" << endl;
            break;
        }
            
        case eqNet::ConnectionSet::EVENT_DATA:
            GLXEventHandler::dispatchAll();
            break;
                
        case eqNet::ConnectionSet::EVENT_INTERRUPT:      
            break;

        case eqNet::ConnectionSet::EVENT_CONNECT:
        case eqNet::ConnectionSet::EVENT_TIMEOUT:   
        case eqNet::ConnectionSet::EVENT_ERROR:      
        default:
            EQWARN << "Error during select" << endl;
            break;
                
    }
}

void GLXEventHandler::dispatchAll()
{
    if( _pipeConnections == 0 )
        _pipeConnections = new eqNet::ConnectionSet;

    const eqNet::ConnectionVector& connections =
        _pipeConnections->getConnections();

    for( eqNet::ConnectionVector::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        X11ConnectionPtr connection = 
            RefPtr_static_cast< eqNet::Connection, X11Connection >( *i );
        
        _handleEvents( connection );
    }
}

void GLXEventHandler::_handleEvents( RefPtr< X11Connection > connection )
{
    Display*         display = connection->getDisplay();
    Pipe*            pipe    = connection->pipe;
    GLXEventHandler* handler = 
        static_cast< GLXEventHandler* >( pipe->getEventHandler( ));
    EQASSERT( handler );

    while( XPending( display ))
    {
        WindowEvent event;
        XEvent&     xEvent = event.xEvent;
        XNextEvent( display, &xEvent );
        
        handler->_processEvent( event, pipe );
    }
}

void GLXEventHandler::_processEvent( WindowEvent& event, Pipe* pipe )
{
    XEvent&                  xEvent   = event.xEvent;
    XID                      drawable = xEvent.xany.window;
    const vector< Window* >& windows  = pipe->getWindows();

    event.window   = 0;
    for( vector< Window* >::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        Window* window = *i;
        if( window->getXDrawable() == drawable )
        {
            event.window = window;
            break;
        }
    }

    if( !event.window )
    {
        EQWARN << "Can't match window to received X event" << endl;
        return;
    }

    switch( xEvent.type )
    {
        case Expose:
            if( xEvent.xexpose.count ) // Only report last expose event
                return;
                
            event.data.type = Event::EXPOSE;
            break;

        case ConfigureNotify:
        {
            // Get window coordinates from X11, the event data is relative to
            // window parent, but we report pvp relative to root window.
            XWindowAttributes windowAttrs;
                
            XGetWindowAttributes( xEvent.xany.display, drawable, &windowAttrs );
                
            XID child;
            XTranslateCoordinates( xEvent.xany.display, drawable, 
                                   RootWindowOfScreen( windowAttrs.screen ),
                                   windowAttrs.x, windowAttrs.y,
                                   &event.data.resize.x, &event.data.resize.y,
                                   &child );

            event.data.type = Event::RESIZE;
            event.data.resize.w = windowAttrs.width;
            event.data.resize.h = windowAttrs.height;
            break;
        }

        case ClientMessage:
        {
            Atom deleteAtom = XInternAtom( xEvent.xany.display,
                                           "WM_DELETE_WINDOW", False );

            if( static_cast<Atom>( xEvent.xclient.data.l[0] ) != deleteAtom )
                return; // not a delete message, ignore.
        }
        // else: delete message, fall through
        case DestroyNotify:
            event.data.type = Event::WINDOW_CLOSE;
            break;

        case MotionNotify:
            event.data.type = Event::POINTER_MOTION;
            event.data.pointerMotion.x = xEvent.xmotion.x;
            event.data.pointerMotion.y = xEvent.xmotion.y;
            event.data.pointerMotion.buttons = _getButtonState( xEvent );
            event.data.pointerMotion.button  = PTR_BUTTON_NONE;

            _computePointerDelta( event );
            _getRenderContext( event );
            break;

        case ButtonPress:
            event.data.type = Event::POINTER_BUTTON_PRESS;
            event.data.pointerButtonPress.x = xEvent.xbutton.x;
            event.data.pointerButtonPress.y = xEvent.xbutton.y;
            event.data.pointerButtonPress.buttons = _getButtonState( xEvent );
            event.data.pointerButtonPress.button  = _getButtonAction( xEvent );
            
            _computePointerDelta( event );
            _getRenderContext( event );
            break;
            
        case ButtonRelease:
            event.data.type = Event::POINTER_BUTTON_RELEASE;
            event.data.pointerButtonRelease.x = xEvent.xbutton.x;
            event.data.pointerButtonRelease.y = xEvent.xbutton.y;
            event.data.pointerButtonRelease.buttons = _getButtonState( xEvent );
            event.data.pointerButtonRelease.button  = _getButtonAction( xEvent);
            
            _computePointerDelta( event );
            _getRenderContext( event );
            break;
            
        case KeyPress:
            event.data.type = Event::KEY_PRESS;
            event.data.keyPress.key = _getKey( xEvent );
            break;
                
        case KeyRelease:
            event.data.type = Event::KEY_RELEASE;
            event.data.keyPress.key = _getKey( xEvent );
            break;

        case UnmapNotify:
        case MapNotify:
        case ReparentNotify:
        case VisibilityNotify:
            event.data.type = Event::UNKNOWN;
            EQINFO << "Ignored X event, type " << xEvent.type << endl;
            break;

        default:
            event.data.type = Event::UNKNOWN;
            EQWARN << "Unhandled X event, type " << xEvent.type << endl;
            break;
    }

    EQLOG( LOG_EVENTS ) << "received event: " << event << endl;
    event.window->processEvent( event );
}

uint32_t GLXEventHandler::_getButtonState( XEvent& event )
{
    const int xState = event.xbutton.state;
    uint32_t   state  = 0;
    
    if( xState & Button1Mask ) state |= PTR_BUTTON1;
    if( xState & Button2Mask ) state |= PTR_BUTTON2;
    if( xState & Button3Mask ) state |= PTR_BUTTON3;
    if( xState & Button4Mask ) state |= PTR_BUTTON4;
    if( xState & Button5Mask ) state |= PTR_BUTTON5;
    
    switch( event.type )
    {   // state is state before event
        case ButtonPress:
            state |= _getButtonAction( event );
            break;

        case ButtonRelease:
            state &= ~_getButtonAction( event );
            break;

        default:
            break;
    }
    return state;
}

uint32_t GLXEventHandler::_getButtonAction( XEvent& event )
{
    switch( event.xbutton.button )
    {    
        case Button1: return PTR_BUTTON1;
        case Button2: return PTR_BUTTON2;
        case Button3: return PTR_BUTTON3;
        case Button4: return PTR_BUTTON4;
        case Button5: return PTR_BUTTON5;
        default: return PTR_BUTTON_NONE;
    }
}


uint32_t GLXEventHandler::_getKey( XEvent& event )
{
    const KeySym key = XKeycodeToKeysym( event.xany.display, 
                                         event.xkey.keycode, 0);
    switch( key )
    {
        case XK_Escape:    return KC_ESCAPE;    
        case XK_BackSpace: return KC_BACKSPACE; 
        case XK_Return:    return KC_RETURN;    
        case XK_Tab:       return KC_TAB;       
        case XK_Home:      return KC_HOME;       
        case XK_Left:      return KC_LEFT;       
        case XK_Up:        return KC_UP;         
        case XK_Right:     return KC_RIGHT;      
        case XK_Down:      return KC_DOWN;       
        case XK_Page_Up:   return KC_PAGE_UP;    
        case XK_Page_Down: return KC_PAGE_DOWN;  
        case XK_End:       return KC_END;        
        case XK_F1:        return KC_F1;         
        case XK_F2:        return KC_F2;         
        case XK_F3:        return KC_F3;         
        case XK_F4:        return KC_F4;         
        case XK_F5:        return KC_F5;         
        case XK_F6:        return KC_F6;         
        case XK_F7:        return KC_F7;         
        case XK_F8:        return KC_F8;         
        case XK_F9:        return KC_F9;         
        case XK_F10:       return KC_F10;        
        case XK_F11:       return KC_F11;        
        case XK_F12:       return KC_F12;        
        case XK_F13:       return KC_F13;        
        case XK_F14:       return KC_F14;        
        case XK_F15:       return KC_F15;        
        case XK_F16:       return KC_F16;        
        case XK_F17:       return KC_F17;        
        case XK_F18:       return KC_F18;        
        case XK_F19:       return KC_F19;        
        case XK_F20:       return KC_F20;        
        case XK_Shift_L:   return KC_SHIFT_L;    
        case XK_Shift_R:   return KC_SHIFT_R;    
        case XK_Control_L: return KC_CONTROL_L;  
        case XK_Control_R: return KC_CONTROL_R;  
        case XK_Alt_L:     return KC_ALT_L;      
        case XK_Alt_R:     return KC_ALT_R;
            
        default: 
            // 'Useful' Latin1 characters
            if( (key >= XK_space && key <= XK_asciitilde ) ||
                (key >= XK_nobreakspace && key <= XK_ydiaeresis))

                return key;

            EQWARN << "Unrecognized X11 key code " << key << endl;
            return KC_VOID;
    }
}

}
