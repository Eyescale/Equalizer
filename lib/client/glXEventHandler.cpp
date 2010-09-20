
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "glXEventHandler.h"

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

GLXEventHandler::GLXEventHandler( GLXPipe* pipe )
        : _pipe( pipe )
{
    if( !_pipeConnections )
        _pipeConnections = new GLXEventHandler::EventSet;
    
    _pipeConnections->addConnection( new X11Connection( pipe ));
}

GLXEventHandler::~GLXEventHandler()
{
    EQASSERT( _pipeConnections.isValid( ));
    EQASSERT( _windows.empty( ));

    const net::Connections& connections = _pipeConnections->getConnections();

    for( net::Connections::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        net::ConnectionPtr connection = *i;
        X11ConnectionPtr x11Connection = static_cast< X11Connection* >( 
                                             connection.get( ));
        if( x11Connection->pipe == _pipe )
        {
            _pipeConnections->removeConnection( connection );
            x11Connection = 0;
            EQASSERTINFO( connection->getRefCount() == 1,
                          connection->getRefCount( ));
            break;
        }
    }
}

GLXEventSetPtr GLXEventHandler::getEventSet()
{
    return _pipeConnections.get();
}

void GLXEventHandler::clearEventSet()
{
    EQASSERTINFO( !_pipeConnections || _pipeConnections->isEmpty(),
                  _pipeConnections->getConnections().size( ));
#if 0 // Asserts with more than one non-threaded pipe
    EQASSERTINFO( !_pipeConnections || _pipeConnections->getRefCount() == 1,
                  _pipeConnections->getRefCount( ));
#endif

    _pipeConnections = 0;
    EQINFO << "Cleared glX event set" << endl;
}

void GLXEventHandler::dispatchOne()
{
    _dispatch( -1 );
}

bool GLXEventHandler::_dispatch( const int timeout )
{
    GLXEventSetPtr connections = _pipeConnections.get();
    if( !connections )
        return false;

    const net::ConnectionSet::Event event = connections->select( timeout );
    switch( event )
    {
        case net::ConnectionSet::EVENT_DISCONNECT:
        {
            net::ConnectionPtr connection = connections->getConnection();
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
        case net::ConnectionSet::EVENT_ERROR:      
        default:
            EQWARN << "Error during select" << endl;
            break;

        case net::ConnectionSet::EVENT_TIMEOUT:
            return false;
    }

    return true;
}

void GLXEventHandler::dispatchAll()
{
    GLXEventSetPtr pipeConnections = _pipeConnections.get();
    if( !pipeConnections )
        return;

    const net::Connections& connections = pipeConnections->getConnections();

    for( net::Connections::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        net::ConnectionPtr connection = *i;
        X11ConnectionPtr x11Connection = static_cast< X11Connection* >( 
                                             connection.get( ));
        
        _handleEvents( x11Connection );
    }

    while( _dispatch( 0 )) // handle disconnect and interrupt events
        ;
}

void GLXEventHandler::registerWindow( GLXWindowIF* window )
{
    EQ_TS_THREAD( _thread );
    const XID xid = window->getXDrawable();
    EQASSERT( _windows.find( xid ) == _windows.end( ));

    _windows[ xid ] = window;
}

void GLXEventHandler::deregisterWindow( GLXWindowIF* window )
{
    EQ_TS_THREAD( _thread );
    const XID xid = window->getXDrawable();
    EQASSERT( xid );
    
    if( xid )
    {
        WindowMap::iterator i = _windows.find( xid );
        EQASSERT( i != _windows.end( ));
        if( i != _windows.end( ))
            _windows.erase( i );
        return;
    }

    for( WindowMap::iterator i = _windows.begin(); i != _windows.end(); ++i)
    {
        if( i->second == window )
        {
            _windows.erase( i );
            return;
        }
    }
}

void GLXEventHandler::_handleEvents( X11ConnectionPtr connection )
{
    GLXPipe*         glXPipe = connection->pipe;
    Display*         display = glXPipe->getXDisplay();
    GLXEventHandler* handler = glXPipe->getGLXEventHandler();
    EQASSERT( handler );

    while( XPending( display ))
    {
        GLXWindowEvent event;
        XEvent&        xEvent = event.xEvent;
        XNextEvent( display, &xEvent );
        
        handler->_processEvent( event );
    }
}

namespace
{
void _getWindowSize( Display* display, XID drawable, ResizeEvent& event )
{
    // Get window coordinates from X11, the event data is relative to window
    // parent, but we report pvp relative to root window.
    XWindowAttributes windowAttrs;
    XGetWindowAttributes( display, drawable, &windowAttrs );
                
    XID child;
    XTranslateCoordinates( display, drawable, 
                           RootWindowOfScreen( windowAttrs.screen ),
                           windowAttrs.x, windowAttrs.y,
                           &event.x, &event.y, &child );

    event.w = windowAttrs.width;
    event.h = windowAttrs.height;
}
}

void GLXEventHandler::_processEvent( GLXWindowEvent& event )
{
    EQ_TS_THREAD( _thread );

    XEvent& xEvent = event.xEvent;
    XID drawable = xEvent.xany.window;

    WindowMap::const_iterator i = _windows.find( drawable );
    GLXWindowIF* const glXWindow = (i == _windows.end()) ? 0 : i->second;
    Window* const window = glXWindow ? glXWindow->getWindow() : 0;

    if( !window )
    {
        EQINFO << "Can't match window to received X event" << endl;
        return;
    }
    EQASSERT( glXWindow );

    switch( xEvent.type )
    {
        case Expose:
            if( xEvent.xexpose.count ) // Only report last expose event
                return;
                
            event.type = Event::WINDOW_EXPOSE;
            break;

        case ConfigureNotify:
            event.type = Event::WINDOW_RESIZE;
            _getWindowSize( xEvent.xany.display, drawable, event.resize );
            break;

        case UnmapNotify:
            event.type = Event::WINDOW_HIDE;
            _getWindowSize( xEvent.xany.display, drawable, event.resize );
            break;

        case MapNotify:
            event.type = Event::WINDOW_SHOW;
            _getWindowSize( xEvent.xany.display, drawable, event.resize );
            break;

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
            if( (event.xkey.state & ShiftMask) && key >= 'a' && key <= 'z' )
                return key + 'A' - 'a'; // capitalize letter

            if( (key >= XK_space && key <= XK_asciitilde ) ||
                (key >= XK_nobreakspace && key <= XK_ydiaeresis))
            {
                return key;
            }
            EQWARN << "Unrecognized X11 key code " << key << endl;
            return KC_VOID;
    }
}

}
