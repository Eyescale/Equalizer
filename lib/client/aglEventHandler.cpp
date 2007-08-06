/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "aglEventHandler.h"

#include "log.h"
#include "window.h"

using namespace eq;
using namespace eqBase;
using namespace std;

AGLEventHandler AGLEventHandler::_handler;

AGLEventHandler* AGLEventHandler::get()
{
    return &_handler;
}

AGLEventHandler::AGLEventHandler()
        : _lastDX( 0 ),
          _lastDY( 0 )
{
}

void AGLEventHandler::addWindow( Window* window )
{
    const WindowRef carbonWindow = window->getCarbonWindow();
    if( !carbonWindow )
    {
        EQWARN
            << "Adding window without native Carbon window to AGL event handler"
            << endl;
        return;
    }

    EventHandlerUPP eventHandler = NewEventHandlerUPP( 
        eq::AGLEventHandler::_handleEventUPP );
    EventTypeSpec   eventType[]    = {
        { kEventClassWindow,   kEventWindowBoundsChanged },
        { kEventClassWindow,   kEventWindowUpdate },
        { kEventClassWindow,   kEventWindowDrawContent },
        { kEventClassWindow,   kEventWindowClosed },
        { kEventClassMouse,    kEventMouseMoved },
        { kEventClassMouse,    kEventMouseDragged },
        { kEventClassMouse,    kEventMouseDown },
        { kEventClassMouse,    kEventMouseUp },
        { kEventClassKeyboard, kEventRawKeyDown },
        { kEventClassKeyboard, kEventRawKeyUp },
        { kEventClassKeyboard, kEventRawKeyRepeat },
        { kEventClassKeyboard, kEventRawKeyModifiersChanged }
    };

    InstallWindowEventHandler( carbonWindow, eventHandler, 
                               sizeof( eventType ) / sizeof( EventTypeSpec ),
                               eventType, window, &window->_carbonHandler );
    EQINFO << "Installed event handler for carbon window " << carbonWindow
           << endl;
}

void AGLEventHandler::removeWindow( Window* window )
{
    RemoveEventHandler( window->_carbonHandler );
    window->_carbonHandler = 0;
}

pascal OSStatus AGLEventHandler::_handleEventUPP( 
    EventHandlerCallRef nextHandler, EventRef event, void* userData )
{
    AGLEventHandler* handler = get();

    handler->_handleEvent( event, static_cast< eq::Window* >( userData ));

    // Always pass events to the default handler. Most events require some
    // action, which is not the case on other window systems.
    return CallNextEventHandler( nextHandler, event );
}

bool AGLEventHandler::_handleEvent( EventRef event, eq::Window* window )
{
    switch( GetEventClass( event ))
    {
        case kEventClassWindow:
            return _handleWindowEvent( event, window );
        case kEventClassMouse:
            return _handleMouseEvent( event, window );
        case kEventClassKeyboard:
            return _handleKeyEvent( event, window );
        default:
            EQINFO << "Unknown event class " << GetEventClass( event ) << endl;
            return false;
    }
}

bool AGLEventHandler::_handleWindowEvent( EventRef event, eq::Window* window )
{
    WindowEvent windowEvent;
    windowEvent.carbonEventRef = event;

    switch( GetEventKind( event ))
    {
        case kEventWindowBoundsChanged:
        {
            Rect rect;
            GetEventParameter( event, kEventParamCurrentBounds, 
                               typeQDRectangle, 0, sizeof( rect ), 0, 
                               &rect );

                windowEvent.type = WindowEvent::RESIZE;
                windowEvent.resize.x = rect.top;
                windowEvent.resize.y = rect.left;
                windowEvent.resize.h = rect.bottom - rect.top;
                windowEvent.resize.w = rect.right  - rect.left;
                break;
        }

        case kEventWindowUpdate:
        case kEventWindowDrawContent:
            windowEvent.type = WindowEvent::EXPOSE;
            break;

        case kEventWindowClosed:
            windowEvent.type = WindowEvent::CLOSE;
            break;

        default:
            EQINFO << "Unhandled window event " << GetEventKind( event ) <<endl;
            windowEvent.type = WindowEvent::UNHANDLED;
            break;
    }

    EQLOG( LOG_EVENTS ) << "received event: " << windowEvent << endl;
    return window->processEvent( windowEvent );
}

bool AGLEventHandler::_handleMouseEvent( EventRef event, eq::Window* window )
{
    HIPoint     pos;
    WindowEvent windowEvent;

    windowEvent.carbonEventRef = event;

    switch( GetEventKind( event ))
    {
        case kEventMouseMoved:
        case kEventMouseDragged:
            windowEvent.type                  = WindowEvent::POINTER_MOTION;
            windowEvent.pointerMotion.button  = PTR_BUTTON_NONE;
            // Note: Luckily GetCurrentEventButtonState returns the same bits as
            // our button definitions.
            windowEvent.pointerMotion.buttons = GetCurrentEventButtonState();

            GetEventParameter( event, kEventParamWindowMouseLocation, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            windowEvent.pointerMotion.x = static_cast< int32_t >( pos.x );
            windowEvent.pointerMotion.y = static_cast< int32_t >( pos.y );

            GetEventParameter( event, kEventParamMouseDelta, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            windowEvent.pointerMotion.dx = static_cast< int32_t >( pos.x );
            windowEvent.pointerMotion.dy = static_cast< int32_t >( pos.y );

            _lastDX = windowEvent.pointerMotion.dx;
            _lastDY = windowEvent.pointerMotion.dy;
            break;

        case kEventMouseDown:
            windowEvent.type = WindowEvent::POINTER_BUTTON_PRESS;
            windowEvent.pointerButtonPress.buttons=GetCurrentEventButtonState();
            windowEvent.pointerButtonPress.button = _getButtonAction( event );

            GetEventParameter( event, kEventParamWindowMouseLocation, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            windowEvent.pointerButtonPress.x = static_cast< int32_t >( pos.x );
            windowEvent.pointerButtonPress.y = static_cast< int32_t >( pos.y );

            windowEvent.pointerButtonPress.dx = _lastDX;
            windowEvent.pointerButtonPress.dy = _lastDY;
            _lastDX = 0;
            _lastDY = 0;
            break;

        case kEventMouseUp:
            windowEvent.type = WindowEvent::POINTER_BUTTON_RELEASE;
            windowEvent.pointerButtonRelease.buttons =
                GetCurrentEventButtonState();
            windowEvent.pointerButtonRelease.button = _getButtonAction( event );

            GetEventParameter( event, kEventParamWindowMouseLocation, 
                               typeHIPoint, 0, sizeof( pos ), 0, 
                               &pos );
            windowEvent.pointerButtonRelease.x = static_cast< int32_t>( pos.x );
            windowEvent.pointerButtonRelease.y = static_cast< int32_t>( pos.y );

            windowEvent.pointerButtonRelease.dx = _lastDX;
            windowEvent.pointerButtonRelease.dy = _lastDY;
            _lastDX = 0;
            _lastDY = 0;
            break;

        default:
            EQINFO << "Unhandled mouse event " << GetEventKind( event ) << endl;
            windowEvent.type = WindowEvent::UNHANDLED;
            break;
    }

    EQLOG( LOG_EVENTS ) << "received event: " << windowEvent << endl;
    return window->processEvent( windowEvent );
}

bool AGLEventHandler::_handleKeyEvent( EventRef event, eq::Window* window )
{
    EQINFO << "Unhandled key event" << endl;
    return false;
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
        case kEventMouseButtonSecondary: return PTR_BUTTON2;
        case kEventMouseButtonTertiary:  return PTR_BUTTON3;
        default: return PTR_BUTTON_NONE;
    }
}


#if 0
uint32_t AGLEventHandler::_getKey( XEvent& event )
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
#endif
