/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glXEventHandler.h"

#include "commands.h"
#include "event.h"
#include "glXWindow.h"
#include "glXWindowEvent.h"
#include "global.h"
#include "log.h"
#include "packets.h"
#include "pipe.h"
#include "window.h"
#include "X11Connection.h"

#include <eq/base/perThreadRef.h>

#include <X11/keysym.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace
{
static PerThreadRef< GLXEventHandler::EventSet > _pipeConnections;
}

GLXEventHandler::GLXEventHandler( Pipe* pipe )
{
    if( !_pipeConnections )
        _pipeConnections = new GLXEventHandler::EventSet;

    _pipeConnections->addConnection( new X11Connection( pipe ));
}

GLXEventHandler::~GLXEventHandler()
{
}

void GLXEventHandler::deregisterPipe( Pipe* pipe )
{
    EQASSERT( _pipeConnections.isValid( ));

    const net::ConnectionVector& connections = 
        _pipeConnections->getConnections();

    for( net::ConnectionVector::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        X11ConnectionPtr connection = 
            RefPtr_static_cast< net::Connection, X11Connection >( *i );
        
        if( connection->pipe == pipe )
        {
            _pipeConnections->removeConnection( *i );

            // TODO EQASSERTINFO( connection->getRefCount() == 1,
            //                    connection->getRefCount( ));
            break;
        }
    }

    delete this;
}

GLXEventSetPtr GLXEventHandler::getEventSet()
{
    return _pipeConnections.get();
}

void GLXEventHandler::dispatchOne()
{
    if( !_pipeConnections )
        _pipeConnections = new GLXEventHandler::EventSet;

    GLXEventSetPtr connections = _pipeConnections.get();

    const net::ConnectionSet::Event event = connections->select( );
    switch( event )
    {
        case net::ConnectionSet::EVENT_DISCONNECT:
        {
            RefPtr<net::Connection> connection = connections->getConnection();
            connections->removeConnection( connection );
            EQERROR << "Display connection shut down" << endl;
            break;
        }
            
        case net::ConnectionSet::EVENT_DATA:
            GLXEventHandler::dispatchAll();
            break;
                
        case net::ConnectionSet::EVENT_INTERRUPT:      
            break;

        case net::ConnectionSet::EVENT_CONNECT:
        case net::ConnectionSet::EVENT_TIMEOUT:   
        case net::ConnectionSet::EVENT_ERROR:      
        default:
            EQWARN << "Error during select" << endl;
            break;
                
    }
}

void GLXEventHandler::dispatchAll()
{
    if( !_pipeConnections )
        _pipeConnections = new GLXEventHandler::EventSet;

    const net::ConnectionVector& connections =
        _pipeConnections->getConnections();

    for( net::ConnectionVector::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        X11ConnectionPtr connection = 
            RefPtr_static_cast< net::Connection, X11Connection >( *i );
        
        _handleEvents( connection );
    }
}

void GLXEventHandler::_handleEvents( X11ConnectionPtr connection )
{
    Display*         display = connection->getDisplay();
    Pipe*            pipe    = connection->pipe;
    GLXEventHandler* handler = 
        static_cast< GLXEventHandler* >( pipe->getEventHandler( ));
    EQASSERT( handler );

    while( XPending( display ))
    {
        GLXWindowEvent event;
        XEvent&        xEvent = event.xEvent;
        XNextEvent( display, &xEvent );
        
        handler->_processEvent( event, pipe );
    }
}

namespace
{
static GLXWindowIF* _getGLXWindow( Window* window )
{
    EQASSERT( dynamic_cast< GLXWindowIF* >( window->getOSWindow( )));
    return static_cast< GLXWindowIF* >( window->getOSWindow( ));
}
}

void GLXEventHandler::_processEvent( GLXWindowEvent& event, Pipe* pipe )
{
    XEvent&             xEvent    = event.xEvent;
    XID                 drawable  = xEvent.xany.window;
    const WindowVector& windows   = pipe->getWindows();
    GLXWindowIF*        glXWindow = 0;

    Window* window   = 0;
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        Window* const candidate = *i;
        glXWindow = _getGLXWindow( candidate );

        if( glXWindow && glXWindow->getXDrawable() == drawable )
        {
            window = candidate;
            break;
        }
    }

    if( !window )
    {
        EQWARN << "Can't match window to received X event" << endl;
        return;
    }
    EQASSERT( glXWindow );

    switch( xEvent.type )
    {
        case Expose:
            if( xEvent.xexpose.count ) // Only report last expose event
                return;
                
            event.type = Event::EXPOSE;
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
                                   &event.resize.x, &event.resize.y,
                                   &child );

            event.type = Event::RESIZE;
            event.resize.w = windowAttrs.width;
            event.resize.h = windowAttrs.height;
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
            event.type = Event::WINDOW_CLOSE;
            break;

        case MotionNotify:
            event.type = Event::POINTER_MOTION;
            event.pointerMotion.x = xEvent.xmotion.x;
            event.pointerMotion.y = xEvent.xmotion.y;
            event.pointerMotion.buttons = _getButtonState( xEvent );
            event.pointerMotion.button  = PTR_BUTTON_NONE;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case ButtonPress:
            event.type = Event::POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x = xEvent.xbutton.x;
            event.pointerButtonPress.y = xEvent.xbutton.y;
            event.pointerButtonPress.buttons = _getButtonState( xEvent );
            event.pointerButtonPress.button  = _getButtonAction( xEvent );
            
            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;
            
        case ButtonRelease:
            event.type = Event::POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x = xEvent.xbutton.x;
            event.pointerButtonRelease.y = xEvent.xbutton.y;
            event.pointerButtonRelease.buttons = _getButtonState( xEvent );
            event.pointerButtonRelease.button  = _getButtonAction( xEvent);
            
            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;
            
        case KeyPress:
            event.type = Event::KEY_PRESS;
            event.keyPress.key = _getKey( xEvent );
            break;
                
        case KeyRelease:
            event.type = Event::KEY_RELEASE;
            event.keyPress.key = _getKey( xEvent );
            break;

        case UnmapNotify:
        case MapNotify:
        case ReparentNotify:
        case VisibilityNotify:
            event.type = Event::UNKNOWN;
            EQINFO << "Ignored X event, type " << xEvent.type << endl;
            break;

        default:
            event.type = Event::UNKNOWN;
            EQWARN << "Unhandled X event, type " << xEvent.type << endl;
            break;
    }

    event.originator = window->getID();

    EQLOG( LOG_EVENTS ) << "received event: " << event << endl;
    
    glXWindow->processEvent( event );
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
