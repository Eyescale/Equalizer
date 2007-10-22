/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "messagePump.h"
#include "global.h"

#include <eq/base/debug.h>
#include <eq/base/log.h>

using namespace std;

namespace eq
{
MessagePump::MessagePump()
#ifdef WIN32
        : _win32ThreadID(0)
#endif
#ifdef AGL
        : _receiverQueue(0)
#endif
{
}

void MessagePump::postWakeup()
{
#ifdef WIN32
    if( !_win32ThreadID )
    {
        EQWARN << "Receiver thread not waiting?" << endl;
        return;
    }

    PostThreadMessage( _win32ThreadID, WM_APP, 0, 0 ); // Wake up pop()

#elif defined (AGL)

    if( !_receiverQueue )
    {
        EQWARN << "Receiver thread not waiting?" << endl;
        return;
    }

    EventRef       event;
    const OSStatus status = CreateEvent( 0, 0, 0, 0, kEventAttributeNone, 
                                         &event );

    if( status != noErr )
    {
        EQWARN << "CreateEvent failed: " << status << endl;
        return;
    }

    PostEventToQueue( _receiverQueue, event, kEventPriorityStandard );

#else
    EQUNIMPLEMENTED;
#endif
}

void MessagePump::_initReceiverQueue()
{
#ifdef WIN32
    if( !_win32ThreadID )
    {
        MSG msg;
        PeekMessage( &msg, 0, WM_USER, WM_USER, PM_NOREMOVE );
        _win32ThreadID = GetCurrentThreadId();
    }
    EQASSERTINFO( _win32ThreadID == GetCurrentThreadId(),
                  "MessagePump::pop() called from two different threads" );

#elif defined (AGL)

    if( !_receiverQueue )
        _receiverQueue = GetCurrentEventQueue();
    EQASSERTINFO( _receiverQueue == GetCurrentEventQueue(),
                  "MessagePump::pop() called from two different threads" );

#else
    EQUNIMPLEMENTED;
#endif
}

void MessagePump::dispatchOne()
{
    _initReceiverQueue();
#ifdef WIN32
    MSG msg;
    if( GetMessage( &msg, 0, 0, 0 ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

#elif defined (AGL)

    while( true )
    {
        Global::enterCarbon();
        const EventTargetRef target = GetEventDispatcherTarget();
        EventRef             event;
        const OSStatus       status = ReceiveNextEvent( 0, 0, .05 /* 50ms */,
                                                        true, &event );
        if( status == noErr )
        {
            EQVERB << "Dispatch Carbon event " << event << endl;
            SendEventToEventTarget( event, target );
            ReleaseEvent( event );
            Global::leaveCarbon();
            return;
        }
        Global::leaveCarbon();

        if( status != eventLoopTimedOutErr )
        {
            EQWARN << "ReceiveNextEvent failed: " << status << endl;
            return;
        }
    }


#else
    EQUNIMPLEMENTED;
#endif
}

void MessagePump::dispatchAll()
{
    _initReceiverQueue();
#ifdef WIN32
    MSG msg;
    while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

#elif defined (AGL)

    Global::enterCarbon();
    const EventTargetRef target = GetEventDispatcherTarget();

    while( true )
    {
        EventRef       event;
        const OSStatus status = ReceiveNextEvent( 0, 0, 0.0, true, &event );

        if( status == eventLoopTimedOutErr )
            break;

        if( status != noErr )
        {
            EQWARN << "ReceiveNextEvent failed: " << status << endl;
            break;
        }

        EQVERB << "Dispatch Carbon event " << event << endl;
        SendEventToEventTarget( event, target );
        ReleaseEvent( event );
    }

    Global::leaveCarbon();

#else
    EQUNIMPLEMENTED;
#endif
}
}
