/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "aglMessagePump.h"
#include "global.h"

#include <eq/base/debug.h>
#include <eq/base/log.h>

namespace eq
{
AGLMessagePump::AGLMessagePump()
        : _receiverQueue( 0 )
        , _needGlobalLock( true )
{
    const OSStatus status = CreateEvent( 0, 0, 0, 0, kEventAttributeNone, 
                                         &_wakeupEvent );
    if( status != noErr )
    {
        EQWARN << "CreateEvent failed: " << status << std::endl;
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
        EQWARN << "Receiver thread not waiting?" << std::endl;
        return;
    }

    PostEventToQueue( _receiverQueue, _wakeupEvent, kEventPriorityStandard );
}

void AGLMessagePump::_initReceiverQueue()
{
    if( !_receiverQueue )
    {
        _receiverQueue = GetCurrentEventQueue();
        _needGlobalLock = ( _receiverQueue == GetMainEventQueue( ));
    }

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
            EQVERB << "Dispatch Carbon event " << event << std::endl;

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
            EQWARN << "ReceiveNextEvent failed: " << status << std::endl;
            return;
        }
    }
}

void AGLMessagePump::dispatchAll()
{
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
            EQWARN << "ReceiveNextEvent failed: " << status << std::endl;
            break;
        }

        EQVERB << "Dispatch Carbon event " << event << std::endl;

        if( !_needGlobalLock )
            Global::enterCarbon();
        const EventTargetRef target = GetEventDispatcherTarget();
        SendEventToEventTarget( event, target );
        Global::leaveCarbon();

        ReleaseEvent( event );
    }

    if( _needGlobalLock )
        Global::leaveCarbon();

    // Init the receiver queue (and disable _needGlobalLock) after the first
    // batch (first ReceiveNextEvent is not thread safe)
    _initReceiverQueue();
}
}
