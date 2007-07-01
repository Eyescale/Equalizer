/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "aglEventThread.h"

#include "window.h"

using namespace eq;
using namespace eqBase;
using namespace std;

AGLEventThread AGLEventThread::_thread;

AGLEventThread* AGLEventThread::get()
{
    return &_thread;
}

AGLEventThread::AGLEventThread()
        : _used( 0 )
{
}

bool AGLEventThread::init()
{
    CHECK_THREAD( _eventThread );
    EQINFO << "Initialized agl event thread" << endl;
    return true;
}

void AGLEventThread::exit()
{
    EQINFO << "Exiting agl event thread" << endl;
    CHECK_THREAD( _eventThread );

    eqBase::Thread::exit();
}

void AGLEventThread::addWindow( Window* window )
{
    CHECK_NOT_THREAD( _eventThread );
    _startMutex.set();
    ++_used;
    if( isStopped( ))
        start();
    _startMutex.unset();

    EventHandlerUPP eventHandler = NewEventHandlerUPP( 
        eq::AGLEventThread::_handleEventUPP );
    EventTypeSpec   eventType    = 
        { kEventClassWindow, kEventWindowBoundsChanged };

    InstallWindowEventHandler( window->getCarbonWindow(), eventHandler, 
                               1, &eventType, window, 0 );
}

void AGLEventThread::removeWindow( Window* window )
{
    CHECK_NOT_THREAD( _eventThread );
    EQASSERT( isRunning( ));

    // TODO send remove window event, wait for remove?

    _startMutex.set();
    EQASSERT( _used );
    --_used;
    if( !_used )
        join();
    CHECK_THREAD_RESET( _eventThread );
    _startMutex.unset();
}

//===========================================================================
// Event thread methods
//===========================================================================

void* AGLEventThread::run()
{
    CHECK_THREAD( _eventThread );
    EQINFO << "AGLEventThread running" << endl;

    const EventTargetRef theTarget = GetEventDispatcherTarget();
    while( _used )
    {
        EventRef theEvent;
        const OSStatus status = ReceiveNextEvent( 0, 0, kEventDurationForever, 
                                                  true, &theEvent );
        if( status != noErr )
        {
            EQWARN << "ReceiveNextEvent failed: " << status << endl;
            continue;
        }

        SendEventToEventTarget( theEvent, theTarget );
        ReleaseEvent( theEvent );
    }

    return 0;
}

pascal OSStatus AGLEventThread::_handleEventUPP( 
    EventHandlerCallRef nextHandler, EventRef event, void* userData )
{
    EQINFO << "Event from " << userData << endl;
    return CallNextEventHandler( nextHandler, event );
}

#if 0
uint32_t AGLEventThread::_getButtonState( XEvent& event )
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

uint32_t AGLEventThread::_getButtonAction( XEvent& event )
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


uint32_t AGLEventThread::_getKey( XEvent& event )
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
