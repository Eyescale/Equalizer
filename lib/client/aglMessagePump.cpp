/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
        , _needGlobalLock( GetCurrentEventQueue() == GetMainEventQueue( ))
{
    const OSStatus status = CreateEvent( 0, 0, 0, 0, kEventAttributeNone, 
                                         &_wakeupEvent );
    if( status != noErr )
    {
        EQWARN << "CreateEvent failed: " << status << endl;
        EQUNREACHABLE;
    }
}

AGLMessagePump::~AGLMessagePump()
{
    ReleaseEvent( _wakeupEvent );
}

void AGLMessagePump::postWakeup()
{
    if( !_receiverQueue )
    {
        EQWARN << "Receiver thread not waiting?" << endl;
        return;
    }

    PostEventToQueue( _receiverQueue, _wakeupEvent, kEventPriorityStandard );
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
        EventRef             event;

        if( _needGlobalLock )
            Global::enterCarbon();
            
        const OSStatus       status = ReceiveNextEvent( 0, 0, .05 /* 50ms */,
                                                        true, &event );
        if( status == noErr )
        {
            EQVERB << "Dispatch Carbon event " << event << endl;

            if( !_needGlobalLock )
                Global::enterCarbon();
            const EventTargetRef target = GetEventDispatcherTarget();
            SendEventToEventTarget( event, target );
            Global::leaveCarbon();

            ReleaseEvent( event );
            return;
        }
        
        if( _needGlobalLock )
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

    while( true )
    {
        EventRef       event;

        if( _needGlobalLock )
            Global::enterCarbon(); 
        const OSStatus status = ReceiveNextEvent( 0, 0, 0.0, true, &event );

        if( status == eventLoopTimedOutErr )
            break;

        if( status != noErr )
        {
            EQWARN << "ReceiveNextEvent failed: " << status << endl;
            break;
        }

        EQVERB << "Dispatch Carbon event " << event << endl;

        if( !_needGlobalLock )
            Global::enterCarbon();
        const EventTargetRef target = GetEventDispatcherTarget();
        SendEventToEventTarget( event, target );
        Global::leaveCarbon();

        ReleaseEvent( event );
    }

    if( _needGlobalLock )
        Global::leaveCarbon();
}
}
