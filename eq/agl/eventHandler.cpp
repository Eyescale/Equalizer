
/* Copyright (c) 2007-2014, Stefan Eilemann <eile@equalizergraphics.com>
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
#ifdef AGL

#include "window.h"
#include "windowEvent.h"

#include <eq/config.h>
#include <eq/configEvent.h>
#include <eq/global.h>
#include <eq/log.h>
#include <eq/os.h>
#include <eq/pipe.h>
#include <eq/window.h>

#include <co/global.h>
#include <lunchbox/file.h>

#ifdef EQUALIZER_USE_MAGELLAN
#  include <eq/node.h>
#  include <3DconnexionClient/ConnexionClientAPI.h>
#endif

namespace eq
{
namespace agl
{
static OSStatus _dispatchEventUPP( EventHandlerCallRef nextHandler,
                                   EventRef event, void* userData );

static OSStatus _handleEventUPP( EventHandlerCallRef nextHandler,
                                 EventRef event, void* userData );

EventHandler::EventHandler( agl::WindowIF* window )
        : _window( window )
        , _eventHandler( 0 )
        , _eventDispatcher( 0 )
        , _lastDX( 0 )
        , _lastDY( 0 )
{
    const bool fullscreen =
        window->getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON;
    const WindowRef carbonWindow = window->getCarbonWindow();

    if( !carbonWindow && !fullscreen )
    {
        LBWARN << "Can't install event handler without native Carbon window"
               << std::endl;
        return;
    }

    const EventTypeSpec events[] = {
        { kEventClassWindow,   kEventWindowBoundsChanged },
        { kEventClassWindow,   kEventWindowZoomed },
        { kEventClassWindow,   kEventWindowUpdate },
        { kEventClassWindow,   kEventWindowDrawContent },
        { kEventClassWindow,   kEventWindowClosed },
        { kEventClassWindow,   kEventWindowHidden },
        { kEventClassWindow,   kEventWindowCollapsed },
        { kEventClassWindow,   kEventWindowShown },
        { kEventClassWindow,   kEventWindowExpanded },
        { kEventClassMouse,    kEventMouseMoved },
        { kEventClassMouse,    kEventMouseDragged },
        { kEventClassMouse,    kEventMouseDown },
        { kEventClassMouse,    kEventMouseUp },
        { kEventClassMouse,    kEventMouseWheelMoved },
        { kEventClassKeyboard, kEventRawKeyDown },
        { kEventClassKeyboard, kEventRawKeyUp },
        { kEventClassKeyboard, kEventRawKeyRepeat }
    };
    const size_t nEvents = sizeof( events ) / sizeof( EventTypeSpec );

    Global::enterCarbon();
    EventHandlerUPP eventHandler = NewEventHandlerUPP( _handleEventUPP );
    if( fullscreen )
        InstallApplicationEventHandler( eventHandler, nEvents, events, this,
                                        &_eventHandler );
    else
        InstallWindowEventHandler( carbonWindow, eventHandler, nEvents, events,
                                   this, &_eventHandler );


    if( _window->isThreaded( ))
    {
        LBASSERT( GetCurrentEventQueue() != GetMainEventQueue( ));

        // dispatches to pipe thread queue
        EventHandlerUPP eventDispatcher = NewEventHandlerUPP(_dispatchEventUPP);
        EventQueueRef target = GetCurrentEventQueue();

        if( fullscreen )
            InstallApplicationEventHandler( eventDispatcher, nEvents, events,
                                            target, &_eventDispatcher );
        else
            InstallWindowEventHandler( carbonWindow, eventDispatcher, nEvents,
                                       events, target, &_eventDispatcher );
    }
    else
        _eventDispatcher = 0;

    Global::leaveCarbon();
}

EventHandler::~EventHandler()
{
    Global::enterCarbon();
    if( _eventDispatcher )
    {
        RemoveEventHandler( _eventDispatcher );
        _eventDispatcher = 0;
    }
    if( _eventHandler )
    {
        RemoveEventHandler( _eventHandler );
        _eventHandler = 0;
    }
    Global::leaveCarbon();
}

OSStatus _dispatchEventUPP(EventHandlerCallRef nextHandler, EventRef event,
                           void* userData )
{
    EventQueueRef target = static_cast< EventQueueRef >( userData );

    if( GetCurrentEventQueue() != target )
    {
#if 0 // some events pop up on pipe thread queues...
        LBASSERTINFO( GetCurrentEventQueue() == GetMainEventQueue(),
                      "target " << target << " current " <<
                      GetCurrentEventQueue() << " main " <<
                      GetMainEventQueue( ));
#endif
        PostEventToQueue( target, event, kEventPriorityStandard );
    }
    return CallNextEventHandler( nextHandler, event );
}

OSStatus _handleEventUPP( EventHandlerCallRef nextHandler, EventRef event,
                          void* userData )
{
    EventHandler* handler = static_cast< EventHandler* >( userData );
    agl::WindowIF* window = handler->getWindow();

    if( GetCurrentEventQueue() == GetMainEventQueue( )) // main thread
    {
        if( !window->isThreaded( ))
            // non-threaded window, handle from main thread
            handler->handleEvent( event );

        return CallNextEventHandler( nextHandler, event );
    }

    handler->handleEvent( event );
    return noErr;
}

bool EventHandler::handleEvent( EventRef eventRef )
{
    WindowEvent event;
    event.carbonEventRef = eventRef;

    switch( GetEventClass( eventRef ))
    {
        case kEventClassWindow:
            _processWindowEvent( event );
            break;

        case kEventClassMouse:
            if( !_processMouseEvent( event ))
                return false;
            break;

        case kEventClassKeyboard:
             _processKeyEvent( event );
             break;

        default:
            LBDEBUG << "Unknown event class " << GetEventClass( eventRef )
                   << std::endl;
            return false;
    }

    return _window->processEvent( event );
}

void EventHandler::_processWindowEvent( WindowEvent& event )
{
    WindowRef carbonWindow = _window->getCarbonWindow();
    Rect rect;
    GetWindowBounds( carbonWindow, kWindowContentRgn, &rect );

    event.resize.x = rect.top;
    event.resize.y = rect.left;
    event.resize.h = rect.bottom - rect.top;
    event.resize.w = rect.right  - rect.left;

    if( _window->getIAttribute( WindowSettings::IATTR_HINT_DECORATION ) != OFF )
        event.resize.y -= EQ_AGL_MENUBARHEIGHT;

    EventRef eventRef = event.carbonEventRef;
    switch( GetEventKind( eventRef ))
    {
        case kEventWindowBoundsChanged:
        case kEventWindowZoomed:
            event.type = Event::WINDOW_RESIZE;
            break;

        case kEventWindowUpdate:
            BeginUpdate( carbonWindow );
            EndUpdate( carbonWindow );
            // no break;
        case kEventWindowDrawContent:
            event.type = Event::WINDOW_EXPOSE;
            break;

        case kEventWindowClosed:
            event.type = Event::WINDOW_CLOSE;
            break;

        case kEventWindowHidden:
        case kEventWindowCollapsed:
            event.type = Event::WINDOW_HIDE;
            break;

        case kEventWindowShown:
        case kEventWindowExpanded:
            event.type = Event::WINDOW_SHOW;
            if( carbonWindow == FrontNonFloatingWindow( ))
                SetUserFocusWindow( carbonWindow );
            break;

        default:
            LBDEBUG << "Unhandled window event " << GetEventKind( eventRef )
                   << std::endl;
            event.type = Event::UNKNOWN;
            break;
    }
}

bool EventHandler::_processMouseEvent( WindowEvent& event )
{
    const bool decoration =
        _window->getIAttribute( WindowSettings::IATTR_HINT_DECORATION ) != OFF;
    const int32_t menuHeight = decoration ? EQ_AGL_MENUBARHEIGHT : 0 ;
    HIPoint pos;

    EventRef eventRef = event.carbonEventRef;
    switch( GetEventKind( eventRef ))
    {
        case kEventMouseMoved:
        case kEventMouseDragged:
            event.type                  = Event::WINDOW_POINTER_MOTION;
            event.pointerMotion.button  = PTR_BUTTON_NONE;
            // Note: Luckily GetCurrentEventButtonState returns the same bits as
            // our button definitions.
            event.pointerMotion.buttons = _getButtonState();

            if( event.pointerMotion.buttons == PTR_BUTTON1 )
            {   // Only left button pressed: implement apple-style middle/right
                // button if modifier keys are used.
                uint32_t keys = 0;
                GetEventParameter( eventRef, kEventParamKeyModifiers,
                                   typeUInt32, 0, sizeof( keys ), 0, &keys );
                if( keys & controlKey )
                    event.pointerMotion.buttons = PTR_BUTTON3;
                else if( keys & optionKey )
                    event.pointerMotion.buttons = PTR_BUTTON2;
            }

            GetEventParameter( eventRef, kEventParamWindowMouseLocation,
                               typeHIPoint, 0, sizeof( pos ), 0,
                               &pos );
            if( pos.y < menuHeight )
                return false; // ignore pointer events on the menu bar

            event.pointerMotion.x = static_cast< int32_t >( pos.x );
            event.pointerMotion.y = static_cast< int32_t >( pos.y ) -menuHeight;

            GetEventParameter( eventRef, kEventParamMouseDelta,
                               typeHIPoint, 0, sizeof( pos ), 0,
                               &pos );
            event.pointerMotion.dx = static_cast< int32_t >( pos.x );
            event.pointerMotion.dy = static_cast< int32_t >( pos.y );

            _lastDX = event.pointerMotion.dx;
            _lastDY = event.pointerMotion.dy;
            return true;

        case kEventMouseDown:
            event.type = Event::WINDOW_POINTER_BUTTON_PRESS;
            event.pointerMotion.buttons = _getButtonState();
            event.pointerButtonPress.button = _getButtonAction( eventRef );

            if( event.pointerMotion.buttons == PTR_BUTTON1 )
            {   // Only left button pressed: implement apple-style middle/right
                // button if modifier keys are used.
                uint32_t keys = 0;
                GetEventParameter( eventRef, kEventParamKeyModifiers,
                                   typeUInt32, 0, sizeof( keys ), 0, &keys );
                if( keys & controlKey )
                    event.pointerMotion.buttons = PTR_BUTTON3;
                else if( keys & optionKey )
                    event.pointerMotion.buttons = PTR_BUTTON2;
            }

            GetEventParameter( eventRef, kEventParamWindowMouseLocation,
                               typeHIPoint, 0, sizeof( pos ), 0,
                               &pos );
            if( pos.y < menuHeight )
                return false; // ignore pointer events on the menu bar

            event.pointerButtonPress.x = int32_t( pos.x );
            event.pointerButtonPress.y = int32_t( pos.y ) - menuHeight;
            event.pointerButtonPress.dx = _lastDX;
            event.pointerButtonPress.dy = _lastDY;
            _lastDX = 0;
            _lastDY = 0;
            return true;

        case kEventMouseUp:
            event.type = Event::WINDOW_POINTER_BUTTON_RELEASE;
            event.pointerMotion.buttons = _getButtonState();
            event.pointerButtonRelease.button = _getButtonAction( eventRef );

            if( event.pointerMotion.buttons == PTR_BUTTON1 )
            {   // Only left button pressed: implement apple-style middle/right
                // button if modifier keys are used.
                uint32_t keys = 0;
                GetEventParameter( eventRef, kEventParamKeyModifiers,
                                   typeUInt32, 0, sizeof( keys ), 0, &keys );
                if( keys & controlKey )
                    event.pointerMotion.buttons = PTR_BUTTON3;
                else if( keys & optionKey )
                    event.pointerMotion.buttons = PTR_BUTTON2;
            }

            GetEventParameter( eventRef, kEventParamWindowMouseLocation,
                               typeHIPoint, 0, sizeof( pos ), 0,
                               &pos );
            if( pos.y < menuHeight )
                return false; // ignore pointer events on the menu bar

            event.pointerButtonRelease.x = int32_t( pos.x );
            event.pointerButtonRelease.y = int32_t( pos.y ) - menuHeight;
            event.pointerButtonRelease.dx = _lastDX;
            event.pointerButtonRelease.dy = _lastDY;
            _lastDX = 0;
            _lastDY = 0;
            return true;

        case kEventMouseWheelMoved:
        {
            event.type = Event::WINDOW_POINTER_WHEEL;
            event.pointerWheel.button  = PTR_BUTTON_NONE;
            event.pointerWheel.buttons = _getButtonState();
            event.pointerWheel.dx = _lastDX;
            event.pointerWheel.dy = _lastDY;

            EventMouseWheelAxis axis;
            SInt32 delta;
            GetEventParameter( eventRef, kEventParamMouseWheelAxis,
                               typeMouseWheelAxis, 0, sizeof( axis ), 0, &axis);
            GetEventParameter( eventRef, kEventParamMouseWheelDelta,
                               typeLongInteger, 0, sizeof( delta ), 0, &delta );

            switch( axis )
            {
                case kEventMouseWheelAxisX:
                    event.pointerWheel.xAxis = delta;
                    return true;
                case kEventMouseWheelAxisY:
                    event.pointerWheel.yAxis = delta;
                    return true;
                default:
                    LBUNIMPLEMENTED;
            }
            return true;
        }
        default:
            LBDEBUG << "Unhandled mouse event " << GetEventKind( eventRef )
                   << std::endl;
            event.type = Event::UNKNOWN;
            return true;
    }
}

void EventHandler::_processKeyEvent( WindowEvent& event )
{
    EventRef eventRef = event.carbonEventRef;
    switch( GetEventKind( eventRef ))
    {
        case kEventRawKeyDown:
        case kEventRawKeyRepeat:
            event.type         = Event::KEY_PRESS;
            event.keyPress.key = _getKey( eventRef );
            break;

        case kEventRawKeyUp:
            event.type         = Event::KEY_RELEASE;
            event.keyPress.key = _getKey( eventRef );
            break;

        default:
            LBDEBUG << "Unhandled keyboard event " << GetEventKind( eventRef )
                   << std::endl;
            event.type = Event::UNKNOWN;
            break;
    }
}

uint32_t EventHandler::_getButtonState()
{
    const uint32 buttons = GetCurrentEventButtonState();

    // swap button 2&3
    return ( (buttons & 0xfffffff9u) +
             ((buttons & LB_BIT3) >> 1) +
             ((buttons & LB_BIT2) << 1) );
}


uint32_t EventHandler::_getButtonAction( EventRef event )
{
    EventMouseButton button;
    GetEventParameter( event, kEventParamMouseButton, typeMouseButton, 0,
                       sizeof( button ), 0, &button );

    switch( button )
    {
        case kEventMouseButtonPrimary:   return PTR_BUTTON1;
        case kEventMouseButtonSecondary: return PTR_BUTTON3;
        case kEventMouseButtonTertiary:  return PTR_BUTTON2;
        default: return PTR_BUTTON_NONE;
    }
}

uint32_t EventHandler::_getKey( EventRef eventRef )
{
    unsigned char key;
    GetEventParameter( eventRef, kEventParamKeyMacCharCodes, typeChar, 0,
                       sizeof( char ), 0, &key );
    switch( key )
    {
        case kEscapeCharCode:     return KC_ESCAPE;
        case kBackspaceCharCode:  return KC_BACKSPACE;
        case kReturnCharCode:     return KC_RETURN;
        case kTabCharCode:        return KC_TAB;
        case kHomeCharCode:       return KC_HOME;
        case kLeftArrowCharCode:  return KC_LEFT;
        case kUpArrowCharCode:    return KC_UP;
        case kRightArrowCharCode: return KC_RIGHT;
        case kDownArrowCharCode:  return KC_DOWN;
        case kPageUpCharCode:     return KC_PAGE_UP;
        case kPageDownCharCode:   return KC_PAGE_DOWN;
        case kEndCharCode:        return KC_END;
        case kFunctionKeyCharCode:
        {
            uint32_t keyCode;
            GetEventParameter( eventRef, kEventParamKeyCode, typeUInt32, 0,
                       sizeof( keyCode ), 0, &keyCode );

            switch( keyCode )
            {
                case kVK_F1:        return KC_F1;
                case kVK_F2:        return KC_F2;
                case kVK_F3:        return KC_F3;
                case kVK_F4:        return KC_F4;
                case kVK_F5:        return KC_F5;
                case kVK_F6:        return KC_F6;
                case kVK_F7:        return KC_F7;
                case kVK_F8:        return KC_F8;
                case kVK_F9:        return KC_F9;
                case kVK_F10:       return KC_F10;
                case kVK_F11:       return KC_F11;
                case kVK_F12:       return KC_F12;
                case kVK_F13:       return KC_F13;
                case kVK_F14:       return KC_F14;
                case kVK_F15:       return KC_F15;
                case kVK_F16:       return KC_F16;
                case kVK_F17:       return KC_F17;
                case kVK_F18:       return KC_F18;
                case kVK_F19:       return KC_F19;
                case kVK_F20:       return KC_F20;
#if 0
                case XK_Shift_L:   return KC_SHIFT_L;
                case XK_Shift_R:   return KC_SHIFT_R;
                case XK_Control_L: return KC_CONTROL_L;
                case XK_Control_R: return KC_CONTROL_R;
                case XK_Alt_L:     return KC_ALT_L;
                case XK_Alt_R:     return KC_ALT_R;
#endif
            }
        }

        default:
            // 'Useful' Latin1 characters
            if(( key >= ' ' && key <= '~' ) ||
               ( key >= 0xa0 /*XK_nobreakspace && key <= XK_ydiaeresis*/ ))

                return key;

            LBWARN << "Unrecognized key " << key << std::endl;
            return KC_VOID;
    }
}


#ifdef EQUALIZER_USE_MAGELLAN
extern "C" OSErr InstallConnexionHandlers( ConnexionMessageHandlerProc,
                                           ConnexionAddedHandlerProc,
                                           ConnexionRemovedHandlerProc )
    __attribute__((weak_import));

namespace
{
static uint16_t _magellanID = 0;
static Node*    _magellanNode = 0;

void _magellanEventHandler( io_connect_t, natural_t messageType,
                            void *messageArgument )
{
    switch( messageType )
    {
        case kConnexionMsgDeviceState:
        {
            ConnexionDeviceState *state = static_cast< ConnexionDeviceState* >(
                                                              messageArgument );
            if( state->client == _magellanID )
            {
                Event event;
                event.originator = _magellanNode->getID();
                event.serial = _magellanNode->getSerial();
                event.magellan.buttons = state->buttons;
                event.magellan.xAxis =  state->axis[0];
                event.magellan.yAxis = -state->axis[1];
                event.magellan.zAxis = -state->axis[2];
                event.magellan.xRotation = -state->axis[3];
                event.magellan.yRotation =  state->axis[4];
                event.magellan.zRotation =  state->axis[5];

                // decipher what command/event is being reported by the driver
                switch( state->command )
                {
                    case kConnexionCmdHandleAxis:
                        event.type = Event::MAGELLAN_AXIS;
                        event.magellan.button = 0;
                        break;

                    case kConnexionCmdHandleButtons:
                        event.type = Event::MAGELLAN_BUTTON;
                        event.magellan.button = state->value;
                        break;

                    default:
                        LBASSERTINFO( 0, "Unimplemented space mouse command " <<
                                      state->command );
                }

                _magellanNode->processEvent( event );
            }
            break;
        }
        default:
            // other messageTypes can happen and should be ignored
            break;
    }
}

}
#endif

void EventHandler::initMagellan( Node* node LB_UNUSED)
{
#ifdef EQUALIZER_USE_MAGELLAN
    if( _magellanNode )
        LBDEBUG << "Space Mouse already installed" << std::endl;
    else if( !InstallConnexionHandlers )
        LBWARN << "Space Mouse drivers not installed" << std::endl;
    else if( InstallConnexionHandlers( _magellanEventHandler, 0, 0 ) != noErr )
        LBWARN << "Can't install Space Mouse connexion handlers" << std::endl;
    else
    {
        std::string program( '\0' + lunchbox::getFilename(
                                        Global::getProgramName( )));
        program[0] = program.length() - 1;

        _magellanID = RegisterConnexionClient( 0, (uint8_t*)program.c_str( ),
                                               kConnexionClientModeTakeOver,
                                               kConnexionMaskAll );
        _magellanNode = node;
        LBDEBUG << "Space Mouse installed" << std::endl;
    }
#endif
}

void EventHandler::exitMagellan( Node* node LB_UNUSED)
{
#ifdef EQUALIZER_USE_MAGELLAN
    if( _magellanID && _magellanNode == node )
    {
        UnregisterConnexionClient( _magellanID );
        CleanupConnexionHandlers();
        _magellanID = 0;
        _magellanNode = 0;
    }
#endif
}

}
}
#endif // AGL
