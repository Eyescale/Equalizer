/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "aglMessagePump.h"
#include "global.h"

#include <eq/base/debug.h>
#include <eq/base/log.h>

using namespace std;

namespace eq
{
AGLMessagePump::AGLMessagePump()
        : _receiverQueue( 0 )
{
}

void AGLMessagePump::postWakeup()
{
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
}

void AGLMessagePump::_initReceiverQueue()
{
    if( !_receiverQueue )
        _receiverQueue = GetCurrentEventQueue();

    EQASSERTINFO( _receiverQueue == GetCurrentEventQueue(),
                  "MessagePump::pop() called from two different threads" );
}

void AGLMessagePump::dispatchOne()
{
    _initReceiverQueue();

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
}

void AGLMessagePump::dispatchAll()
{
    _initReceiverQueue();

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
}
}
