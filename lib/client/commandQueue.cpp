/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "commandQueue.h"

using namespace std;

namespace eq
{
CommandQueue::CommandQueue()
#ifdef WIN32
        : _win32ThreadID(0)
#endif
#ifdef Darwin
        : _receiverQueue(0)
#endif
{
}

void CommandQueue::push( eqNet::Command& inCommand )
{
    eqNet::CommandQueue::push( inCommand );
    _postWakeupEvent();
}

void CommandQueue::pushFront( eqNet::Command& inCommand )
{
    eqNet::CommandQueue::pushFront( inCommand );
    _postWakeupEvent();
}

void CommandQueue::_postWakeupEvent()
{
#ifdef WIN32
    if( _win32ThreadID )
        PostThreadMessage( _win32ThreadID, WM_APP, 0, 0 ); // Wake up pop()
#elif defined (Darwin)
    if( !_receiverQueue )
    {
        EQWARN << "Receiver thread not running?" << endl;
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
#endif
}

eqNet::Command* CommandQueue::pop()
{
#ifdef WIN32
    if( !_win32ThreadID )
    {   // First call - force creation of thread message queue
        MSG msg;
        PeekMessage( &msg, 0, WM_USER, WM_USER, PM_NOREMOVE );
        _win32ThreadID = GetCurrentThreadId();
    }
    EQASSERTINFO( _win32ThreadID == GetCurrentThreadId(),
                  "CommandQueue::pop() called from two different threads" );

#elif defined (Darwin)

    if( !_receiverQueue )
        _receiverQueue = GetCurrentEventQueue();
    EQASSERTINFO( _receiverQueue == GetCurrentEventQueue(),
                  "CommandQueue::pop() called from two different threads" );
#endif

#if defined (WIN32) || defined (Darwin)
    while( true )
    {
        _pumpEvents(); // non-blocking

        // Poll for a command
        eqNet::Command* command = tryPop();
        if( command )
            return command;

        _pumpEvent(); // blocking - push will send 'fake' event
    }

#else

    return eqNet::CommandQueue::pop();
#endif
}

void CommandQueue::_pumpEvent()
{
#ifdef WIN32
    MSG msg;
    if( GetMessage( &msg, 0, 0, 0 ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

#elif defined (Darwin)

    const EventTargetRef target = GetEventDispatcherTarget();
    EventRef             event;
    const OSStatus       status = ReceiveNextEvent( 0, 0, kEventDurationForever,
                                                    true, &event );
    if( status != noErr )
    {
        EQWARN << "ReceiveNextEvent failed: " << status << endl;
        return;
    }

    EQVERB << "Dispatch Carbon event " << event << endl;
    SendEventToEventTarget( event, target );
    ReleaseEvent( event );
#endif
}

void CommandQueue::_pumpEvents()
{
#ifdef WIN32
    MSG msg;
    while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

#elif defined (Darwin)

    const EventTargetRef target  = GetEventDispatcherTarget();

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
#endif
}
}
