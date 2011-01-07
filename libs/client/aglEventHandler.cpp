
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "aglEventHandler.h"

#include "aglWindow.h"
#include "aglWindowEvent.h"
#include "config.h"
#include "configEvent.h"
#include "global.h"
#include "log.h"
#include "pipe.h"
#include "window.h"

#include <co/global.h>
#include <co/base/file.h>

#ifdef EQ_USE_MAGELLAN
#  include <3DconnexionClient/ConnexionClientAPI.h>
#endif

namespace eq
{
AGLEventHandler::AGLEventHandler( AGLWindowIF* window )
        : _window( window )
        , _eventHandler( 0 )
        , _eventDispatcher( 0 )
        , _lastDX( 0 )
        , _lastDY( 0 )
{
    const bool fullscreen = 
        window->getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON;
    const WindowRef carbonWindow = window->getCarbonWindow();

    if( !carbonWindow && !fullscreen )
    {
        EQWARN << "Can't install event handler without native Carbon window"
               << std::endl;
        return;
    }
    
    Global::enterCarbon();
    EventHandlerUPP eventHandler = NewEventHandlerUPP( 
        eq::AGLEventHandler::_handleEventUPP );
    EventTypeSpec   events[]    = {
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

    if( _window->getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
        InstallApplicationEventHandler( eventHandler, nEvents, events,
                                        this, &_eventHandler );
    else
        InstallWindowEventHandler( carbonWindow, eventHandler, nEvents, events,
                                   this, &_eventHandler );


    const Pipe* pipe = window->getPipe();
    if( pipe->isThreaded( ))
    {
        EQASSERT( GetCurrentEventQueue() != GetMainEventQueue( ));

        // dispatches to pipe thread queue
        EventHandlerUPP eventDispatcher = NewEventHandlerUPP( 
            eq::AGLEventHandler::_dispatchEventUPP );
        EventQueueRef target = GetCurrentEventQueue();

        if( _window->getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
        {
            InstallApplicationEventHandler( eventDispatcher, nEvents,
                                            events, target, &_eventDispatcher );
        }
        else
            InstallWindowEventHandler( carbonWindow, eventDispatcher, nEvents,
                                       events, target, &_eventDispatcher );
    }
    else
        _eventDispatcher = 0;

    Global::leaveCarbon();

    if( fullscreen )
        EQINFO << "Installed event handlers for carbon window " << carbonWindow
               << std::endl;
    else
        EQINFO << "Installed event handlers for fullscreen carbon context"
               << std::endl;
}

AGLEventHandler::~AGLEventHandler()
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

pascal OSStatus AGLEventHandler::_dispatchEventUPP( 
    EventHandlerCallRef nextHandler, EventRef event, void* userData )
{
    EventQueueRef target = static_cast< EventQueueRef >( userData );
    
    if( GetCurrentEventQueue() != target )
    {
#if 0 // some events pop up on pipe thread queues...
        EQASSERTINFO( GetCurrentEventQueue() == GetMainEventQueue(),
                      "target " << target << " current " << 
                      GetCurrentEventQueue() << " main " << 
                      GetMainEventQueue( ));
#endif
        PostEventToQueue( target, event, kEventPriorityStandard );
    }
    return CallNextEventHandler( nextHandler, event );
}

pascal OSStatus AGLEventHandler::_handleEventUPP( 
    EventHandlerCallRef nextHandler, EventRef event, void* userData )
{
    AGLEventHandler* handler = static_cast< AGLEventHandler* >( userData );
    AGLWindowIF*     window  = handler->_window;

    if( GetCurrentEventQueue() == GetMainEventQueue( )) // main thread
    {
        const Pipe* pipe = window->getPipe();
        if( !pipe->isThreaded( ))
            // non-threaded window, handle from main thread
            handler->_handleEvent( event );

        return CallNextEventHandler( nextHandler, event );
    }

    handler->_handleEvent( event );
    return noErr;
}

bool AGLEventHandler::_handleEvent( EventRef event )
{
    switch( GetEventClass( event ))
    {
        case kEventClassWindow:
            return _handleWindowEvent( event );
        case kEventClassMouse:
            return _handleMouseEvent( event );
        case kEventClassKeyboard:
            return _handleKeyEvent( event );
        default:
            EQINFO << "Unknown event class " << GetEventClass( event ) << std::endl;
            return false;
    }
}

bool AGLEventHandler::_handleWindowEvent( EventRef event )
{
    AGLWindowEvent windowEvent;
    windowEvent.carbonEventRef = event;
    Window* const window       = _window->getWindow();

    Rect      rect;
    WindowRef carbonWindow = _window->getCarbonWindow();

    GetWindowBounds( carbonWindow, kWindowContentRgn, &rect );
    windowEvent.resize.x = rect.top;
    windowEvent.resize.y = rect.left;
    windowEvent.resize.h = rect.bottom - rect.top;
    windowEvent.resize.w = rect.right  - rect.left;

    if( window->getIAttribute( Window::IATTR_HINT_DECORATION ) != OFF )
        windowEvent.resize.y -= EQ_AGL_MENUBARHEIGHT;

    switch( GetEventKind( event ))
    {
        case kEventWindowBoundsChanged:
        case kEventWindowZoomed:
            windowEvent.type = Event::WINDOW_RESIZE;
            break;

        case kEventWindowUpdate:
            BeginUpdate( carbonWindow );
            EndUpdate( carbonWindow );
            // no break;
        case kEventWindowDrawContent:
            windowEvent.type = Event::WINDOW_EXPOSE;
            break;

        case kEventWindowClosed:
            windowEvent.type = Event::WINDOW_CLOSE;
            break;

        case kEventWindowHidden:
        case kEventWindowCollapsed:
            windowEvent.type = Event::WINDOW_HIDE;
            break;
            
        case kEventWindowShown:
        case kEventWindowExpanded:
            windowEvent.type = Event::WINDOW_SHOW;
            if( carbonWindow == FrontNonFloatingWindow( ))
                SetUserFocusWindow( carbonWindow );
            break;

        default:
            EQINFO << "Unhandled window event " << GetEventKind( event )
                   << std::endl;
            windowEvent.type = Event::UNKNOWN;
            break;
    }

    windowEvent.originator = window->getID();
    return _window->processEvent( windowEvent );
}

bool AGLEventHandler::_handleMouseEvent( EventRef event )
{
    HIPoint        pos;
    AGLWindowEvent windowEvent;

    windowEvent.carbonEventRef = event;
    Window* const window       = _window->getWindow();
    
    const bool    decoration =
        window->getIAttribute( Window::IATTR_HINT_DECORATION ) != OFF;
    const int32_t menuHeight = decoration ? EQ_AGL_MENUBARHEIGHT : 0 ;

    switch( GetEventKind( event ))
    {
        case kEventMouseMoved:
        case kEventMouseDragged:
            windowEvent.type                  = Event::WINDOW_POINTER_MOTION;
            windowEvent.pointerMotion.button  = PTR_BUTTON_NONE;
            // Note: Luckily GetCurrentEventButtonState returns the same bits as
            // our button definitions.
            windowEvent.pointerMotion.buttons = _getButtonState();

            if( windowEvent.pointerMotion.buttons == PTR_BUTTON1 )
            {   // Only left button pressed: implement apple-style middle/right
                // button if modifier keys are used.
                uint32_t keys = 0;
                GetEventParameter( event, kEventParamKeyModifiers, 
                                   typeUInt32, 0, sizeof( keys ), 0, &keys );
                if( keys & controlKey )
                    windowEvent.pointerMotion.buttons = PTR_BUTTON3;
                else if( keys & optionKey )
                    windowEvent.pointerMotion.buttons = PTR_BUTTON2;
            }

            GetEventParameter( event, kEventParamWindowMouseLocation, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            if( pos.y < menuHeight )
                return false; // ignore pointer events on the menu bar

            windowEvent.pointerMotion.x = static_cast< int32_t >( pos.x );
            windowEvent.pointerMotion.y = static_cast< int32_t >( pos.y ) -
                                               menuHeight;

            GetEventParameter( event, kEventParamMouseDelta, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            windowEvent.pointerMotion.dx = static_cast< int32_t >( pos.x );
            windowEvent.pointerMotion.dy = static_cast< int32_t >( pos.y );

            _lastDX = windowEvent.pointerMotion.dx;
            _lastDY = windowEvent.pointerMotion.dy;

            _getRenderContext( window, windowEvent );
            break;

        case kEventMouseDown:
            windowEvent.type = Event::WINDOW_POINTER_BUTTON_PRESS;
            windowEvent.pointerMotion.buttons = _getButtonState();
            windowEvent.pointerButtonPress.button  =
                _getButtonAction( event );

            if( windowEvent.pointerMotion.buttons == PTR_BUTTON1 )
            {   // Only left button pressed: implement apple-style middle/right
                // button if modifier keys are used.
                uint32_t keys = 0;
                GetEventParameter( event, kEventParamKeyModifiers, 
                                   typeUInt32, 0, sizeof( keys ), 0, &keys );
                if( keys & controlKey )
                    windowEvent.pointerMotion.buttons = PTR_BUTTON3;
                else if( keys & optionKey )
                    windowEvent.pointerMotion.buttons = PTR_BUTTON2;
            }

            GetEventParameter( event, kEventParamWindowMouseLocation, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            if( pos.y < menuHeight )
                return false; // ignore pointer events on the menu bar

            windowEvent.pointerButtonPress.x = 
                static_cast< int32_t >( pos.x );
            windowEvent.pointerButtonPress.y = 
                static_cast< int32_t >( pos.y ) - menuHeight;

            windowEvent.pointerButtonPress.dx = _lastDX;
            windowEvent.pointerButtonPress.dy = _lastDY;
            _lastDX = 0;
            _lastDY = 0;

            _getRenderContext( window, windowEvent );
            break;

        case kEventMouseUp:
            windowEvent.type = Event::WINDOW_POINTER_BUTTON_RELEASE;
            windowEvent.pointerMotion.buttons = _getButtonState();
            windowEvent.pointerButtonRelease.button = 
                _getButtonAction( event );

            if( windowEvent.pointerMotion.buttons == PTR_BUTTON1 )
            {   // Only left button pressed: implement apple-style middle/right
                // button if modifier keys are used.
                uint32_t keys = 0;
                GetEventParameter( event, kEventParamKeyModifiers, 
                                   typeUInt32, 0, sizeof( keys ), 0, &keys );
                if( keys & controlKey )
                    windowEvent.pointerMotion.buttons = PTR_BUTTON3;
                else if( keys & optionKey )
                    windowEvent.pointerMotion.buttons = PTR_BUTTON2;
            }

            GetEventParameter( event, kEventParamWindowMouseLocation, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            if( pos.y < menuHeight )
                return false; // ignore pointer events on the menu bar

            windowEvent.pointerButtonRelease.x = 
                static_cast< int32_t>( pos.x );
            windowEvent.pointerButtonRelease.y = 
                static_cast< int32_t>( pos.y ) - menuHeight;

            windowEvent.pointerButtonRelease.dx = _lastDX;
            windowEvent.pointerButtonRelease.dy = _lastDY;
            _lastDX = 0;
            _lastDY = 0;

            _getRenderContext( window, windowEvent );
            break;

        case kEventMouseWheelMoved:
        {
            windowEvent.type = Event::WINDOW_POINTER_WHEEL;
            windowEvent.pointerWheel.button  = PTR_BUTTON_NONE;
            windowEvent.pointerWheel.buttons = _getButtonState();
            windowEvent.pointerWheel.dx = _lastDX;
            windowEvent.pointerWheel.dy = _lastDY;

            EventMouseWheelAxis axis;
            SInt32 delta;
            GetEventParameter( event, kEventParamMouseWheelAxis,
                               typeMouseWheelAxis, 0, sizeof( axis ), 0, &axis);
            GetEventParameter( event, kEventParamMouseWheelDelta,
                               typeLongInteger, 0, sizeof( delta ), 0, &delta );
            
            switch( axis )
            {
                case kEventMouseWheelAxisY: // NO typo - y is the primary axis
                    windowEvent.pointerWheel.xAxis = delta;
                    break;
                case kEventMouseWheelAxisX:
                    windowEvent.pointerWheel.yAxis = delta;
                    break;
                default:
                    EQUNIMPLEMENTED;
            }
            break;
        }
        default:
            EQINFO << "Unhandled mouse event " << GetEventKind( event )
                   << std::endl;
            windowEvent.type = Event::UNKNOWN;
            break;
    }

    windowEvent.originator = window->getID();
    return _window->processEvent( windowEvent );
}

bool AGLEventHandler::_handleKeyEvent( EventRef event )
{
    AGLWindowEvent windowEvent;

    windowEvent.carbonEventRef = event;
    Window* const window       = _window->getWindow();

    switch( GetEventKind( event ))
    {
        case kEventRawKeyDown:
        case kEventRawKeyRepeat:
            windowEvent.type         = Event::KEY_PRESS;
            windowEvent.keyPress.key = _getKey( event );
            break;

        case kEventRawKeyUp:
            windowEvent.type         = Event::KEY_RELEASE;
            windowEvent.keyPress.key = _getKey( event );
            break;

        default:
            EQINFO << "Unhandled keyboard event " << GetEventKind( event )
                   << std::endl;
            windowEvent.type = Event::UNKNOWN;
            break;
    }

    windowEvent.originator = window->getID();
    return _window->processEvent( windowEvent );
}

uint32_t AGLEventHandler::_getButtonState()
{
    const uint32 buttons = GetCurrentEventButtonState();
    
    // swap button 2&3
    return ( (buttons & 0xfffffff9u) +
             ((buttons & EQ_BIT3) >> 1) +
             ((buttons & EQ_BIT2) << 1) );
}


uint32_t AGLEventHandler::_getButtonAction( EventRef event )
{
    EventMouseButton button;
    GetEventParameter( event, kEventParamMouseButton, 
                               typeMouseButton, 0, sizeof( button ), 0, 
                               &button );

    switch( button )
    {    
        case kEventMouseButtonPrimary:   return PTR_BUTTON1;
        case kEventMouseButtonSecondary: return PTR_BUTTON3;
        case kEventMouseButtonTertiary:  return PTR_BUTTON2;
        default: return PTR_BUTTON_NONE;
    }
}

uint32_t AGLEventHandler::_getKey( EventRef event )
{
    unsigned char key;
    GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, 0,
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
            GetEventParameter( event, kEventParamKeyCode, typeUInt32, 0,
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

            EQWARN << "Unrecognized key " << key << std::endl;
            return KC_VOID;
    }
}


#ifdef EQ_USE_MAGELLAN
extern "C" OSErr InstallConnexionHandlers( ConnexionMessageHandlerProc, 
                                           ConnexionAddedHandlerProc, 
                                           ConnexionRemovedHandlerProc ) 
    __attribute__((weak_import));

namespace
{
static uint16_t _magellanID = 0;
static Node*    _magellanNode = 0;

void _magellanEventHandler( io_connect_t connection, natural_t messageType, 
                            void *messageArgument ) 
{ 
    switch (messageType) 
    { 
        case kConnexionMsgDeviceState: 
        {
            ConnexionDeviceState *state = static_cast< ConnexionDeviceState* >(
                                                              messageArgument );
            if( state->client == _magellanID ) 
            { 
                ConfigEvent event;
                event.data.originator = _magellanNode->getID();
                event.data.magellan.buttons = state->buttons;
                event.data.magellan.xAxis = state->axis[0];
                event.data.magellan.yAxis = state->axis[1];
                event.data.magellan.zAxis = state->axis[2];
                event.data.magellan.xRotation = state->axis[3];
                event.data.magellan.yRotation = state->axis[4];
                event.data.magellan.zRotation = state->axis[5];

                // decipher what command/event is being reported by the driver 
                switch( state->command ) 
                { 
                    case kConnexionCmdHandleAxis: 
                        event.data.type = Event::MAGELLAN_AXIS;
                        event.data.magellan.button = 0;
                        break; 

                    case kConnexionCmdHandleButtons:
                        event.data.type = Event::MAGELLAN_BUTTON;
                        event.data.magellan.button = state->value;
                        break; 
                        
                    default:
                        EQASSERTINFO( 0, "Unimplemented space mouse command " <<
                                      state->command );
                }                 
                        
                _magellanNode->getConfig()->sendEvent( event );
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

void AGLEventHandler::initMagellan( Node* node )
{
#ifdef EQ_USE_MAGELLAN
    if( _magellanNode )
        EQINFO << "Space Mouse already installed" << std::endl;
    else if( !InstallConnexionHandlers )
        EQWARN << "Space Mouse drivers not installed" << std::endl;
    else if( InstallConnexionHandlers( _magellanEventHandler, 0, 0 ) != noErr )
        EQWARN << "Can't install Space Mouse connexion handlers" << std::endl;
    else
    {
        std::string program( '\0' + 
                             base::getFilename( co::Global::getProgramName( )));
        program[0] = program.length() - 1;

        _magellanID = RegisterConnexionClient( 0, (uint8_t*)program.c_str( ),
                                               kConnexionClientModeTakeOver,
                                               kConnexionMaskAll );
        _magellanNode = node;
    }
#endif
}

void AGLEventHandler::exitMagellan( Node* node )
{
#ifdef EQ_USE_MAGELLAN
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
