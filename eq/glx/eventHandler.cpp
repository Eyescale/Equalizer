
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "eventHandler.h"

#include "window.h"
#include "windowEvent.h"
#include "../config.h"
#include "../global.h"
#include "../log.h"
#include "../os.h"
#include "../pipe.h"
#include "../window.h"

#include <eq/fabric/event.h>
#include <lunchbox/lockable.h>
#include <lunchbox/perThread.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/spinLock.h>

#include <X11/keysym.h>
#include <X11/XKBlib.h>

#ifdef EQUALIZER_USE_MAGELLAN_GLX
#  include <spnav.h>
#endif

namespace eq
{
namespace glx
{
namespace
{
typedef std::vector< EventHandler* > EventHandlers;
static lunchbox::PerThread< EventHandlers > _eventHandlers;

struct MagellanData
{
    MagellanData() : display( 0 ), used( 0 ) {}

    Display* display;
    size_t used;
};

static lunchbox::Lockable< MagellanData, lunchbox::SpinLock > _magellan;
}

EventHandler::EventHandler( WindowIF* window )
        : _window( window )
        , _magellanUsed( false )
{
    LBASSERT( window );

    if( !_eventHandlers )
        _eventHandlers = new EventHandlers;
    _eventHandlers->push_back( this );

#ifdef EQUALIZER_USE_MAGELLAN_GLX
    Display* display = window->getXDisplay();
    LBASSERT( display );
    lunchbox::ScopedFastWrite mutex( _magellan );
    if( !_magellan->display )
    {
        if( spnav_x11_open( display, window->getXDrawable( )) == -1 )
        {
            LBDEBUG << "Cannot connect to the space navigator daemon"
                    << std::endl;
            return;
        }
        _magellan->display = display;
    }
    else if( _magellan->display != display )
    {
        LBINFO << "Multi-display space mouse support incomplete" << std::endl;
        return;
    }
    else if( spnav_x11_window( window->getXDrawable( )) == -1 )
    {
        LBWARN << "Failed to register window with the space navigator daemon"
               << std::endl;
        return;
    }

    ++_magellan->used;
    _magellanUsed = true;
#endif
}

EventHandler::~EventHandler()
{
    if( _magellanUsed )
    {
#ifdef EQUALIZER_USE_MAGELLAN_GLX
        lunchbox::ScopedFastWrite mutex( _magellan );
        if( --_magellan->used == 0 )
        {
            if( spnav_close() == -1 )
            {
                LBWARN
                    << "Couldn't close connection to the space navigator daemon"
                    << std::endl;
            }
        }
#endif
        _magellanUsed = false;
    }

    EventHandlers::iterator i = lunchbox::find( *_eventHandlers, this );
    LBASSERT( i != _eventHandlers->end( ));
    _eventHandlers->erase( i );
    if( _eventHandlers->empty( ))
    {
        delete _eventHandlers.get();
        _eventHandlers = 0;
    }
}

void EventHandler::dispatch()
{
    if( !_eventHandlers )
        return;

    for( EventHandlers::const_iterator i = _eventHandlers->begin();
         i != _eventHandlers->end(); ++i )
    {
        (*i)->_dispatch();
    }
}

void EventHandler::_dispatch()
{
    Display* display = _window->getXDisplay();
    LBASSERT( display );
    if( !display )
        return;

    while( XPending( display ))
    {
        WindowEvent event;
        XEvent& xEvent = event.xEvent;

        XNextEvent( display, &xEvent );

        for( EventHandlers::const_iterator i = _eventHandlers->begin();
             i != _eventHandlers->end(); ++i )
        {
            EventHandler* handler = *i;
            handler->_processEvent( event );
        }
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

void EventHandler::_processEvent( WindowEvent& event )
{
    LB_TS_THREAD( _thread );

    XEvent& xEvent = event.xEvent;
    XID drawable = xEvent.xany.window;

    if( _window->getXDrawable() != drawable )
        return;

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
#ifdef EQUALIZER_USE_MAGELLAN_GLX
            spnav_event spev;

            /* spacenav event */
            if( spnav_x11_event( &xEvent, &spev ))
            {
                switch( spev.type )
                {
                  case SPNAV_EVENT_MOTION:
                    event.type = Event::MAGELLAN_AXIS;
                    event.magellan.xAxis =  spev.motion.x;
                    event.magellan.yAxis =  spev.motion.y;
                    event.magellan.zAxis = -spev.motion.z;
                    event.magellan.xRotation = -spev.motion.rx;
                    event.magellan.yRotation = -spev.motion.ry;
                    event.magellan.zRotation =  spev.motion.rz;
                    break;

                  case SPNAV_EVENT_BUTTON:
                    event.type = Event::MAGELLAN_BUTTON;
                    event.magellan.buttons = spev.button.press;
                    event.magellan.button = spev.button.bnum;
                    break;

                  default:
                    LBUNIMPLEMENTED;
                    return;
                }

                /* remove any other queued motion events */
                //? spnav_remove_events( SPNAV_EVENT_MOTION );
                break;
            }
 #endif

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
            event.type = Event::WINDOW_POINTER_MOTION;
            event.pointerMotion.x = xEvent.xmotion.x;
            event.pointerMotion.y = xEvent.xmotion.y;
            event.pointerMotion.buttons = _getButtonState( xEvent );
            event.pointerMotion.button  = PTR_BUTTON_NONE;

            _computePointerDelta( event );
            break;

        case ButtonPress:
            event.type = Event::WINDOW_POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x = xEvent.xbutton.x;
            event.pointerButtonPress.y = xEvent.xbutton.y;
            event.pointerButtonPress.buttons = _getButtonState( xEvent );
            event.pointerButtonPress.button  = _getButtonAction( xEvent );

            // Translate wheel events
            switch( event.pointerButtonPress.button )
            {
              case PTR_BUTTON4: event.pointerWheel.yAxis = 1; break;
              case PTR_BUTTON5: event.pointerWheel.yAxis = -1; break;
              case PTR_BUTTON6: event.pointerWheel.xAxis = 1; break;
              case PTR_BUTTON7: event.pointerWheel.xAxis = -1; break;
            }
            switch( event.pointerButtonPress.button )
            {
              case PTR_BUTTON4:
              case PTR_BUTTON5:
              case PTR_BUTTON6:
              case PTR_BUTTON7:
                event.type = Event::WINDOW_POINTER_WHEEL;
                event.pointerWheel.button = PTR_BUTTON_NONE;
            }

            _computePointerDelta( event );
            break;

        case ButtonRelease:
            event.type = Event::WINDOW_POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x = xEvent.xbutton.x;
            event.pointerButtonRelease.y = xEvent.xbutton.y;
            event.pointerButtonRelease.buttons = _getButtonState( xEvent );
            event.pointerButtonRelease.button  = _getButtonAction( xEvent);

            _computePointerDelta( event );
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
            LBVERB << "Ignored X event, type " << xEvent.type << std::endl;
            break;

        default:
            event.type = Event::UNKNOWN;
            LBWARN << "Unhandled X event, type " << xEvent.type << std::endl;
            break;
    }

    _window->processEvent( event );
}

uint32_t EventHandler::_getButtonState( XEvent& event )
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

uint32_t EventHandler::_getButtonAction( XEvent& event )
{
    switch( event.xbutton.button )
    {
        case Button1: return PTR_BUTTON1;
        case Button2: return PTR_BUTTON2;
        case Button3: return PTR_BUTTON3;
        case Button4: return PTR_BUTTON4;
        case Button5: return PTR_BUTTON5;
        case 6: return PTR_BUTTON6;
        case 7: return PTR_BUTTON7;
        default: return PTR_BUTTON_NONE;
    }
}


uint32_t EventHandler::_getKey( XEvent& event )
{
    int index = 0;
    if( event.xkey.state & ShiftMask )
        index = 1;

    const KeySym key = XkbKeycodeToKeysym( event.xany.display,
                                           event.xkey.keycode, 0, index );
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
            if( (key >= XK_space && key <= XK_asciitilde ) ||
                (key >= XK_nobreakspace && key <= XK_ydiaeresis))
            {
                return key;
            }
            LBWARN << "Unrecognized X11 key code " << key << std::endl;
            return KC_VOID;
    }
}


}
}
