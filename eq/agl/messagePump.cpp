
/* Copyright (c) 2007-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "messagePump.h"
#include <eq/os.h>

#ifdef AGL
#include <eq/global.h>
#include <lunchbox/debug.h>
#include <lunchbox/log.h>

namespace eq
{
namespace agl
{

MessagePump::MessagePump()
        : _receiverQueue( 0 )
        , _needGlobalLock( true )
{
    const OSStatus status = CreateEvent( 0, 0, 0, 0, kEventAttributeNone,
                                         &_wakeupEvent );
    if( status != noErr )
    {
        LBWARN << "CreateEvent failed: " << status << std::endl;
        LBUNREACHABLE;
    }
}

MessagePump::~MessagePump()
{
    ReleaseEvent( _wakeupEvent );
}

void MessagePump::postWakeup()
{
    if( _receiverQueue )
        PostEventToQueue( _receiverQueue, _wakeupEvent, kEventPriorityStandard);
}

void MessagePump::_initReceiverQueue()
{
    if( !_receiverQueue )
    {
        _receiverQueue = GetCurrentEventQueue();
        _needGlobalLock = ( _receiverQueue == GetMainEventQueue( ));
        LBASSERT( _receiverQueue );
    }

    LBASSERTINFO( _receiverQueue == GetCurrentEventQueue(),
                  "MessagePump::dispatch() called from two different threads" );
}

void MessagePump::dispatchOne( const uint32_t timeout )
{
    _initReceiverQueue();
    _clock.reset();

    for( int64_t timeLeft = timeout; timeLeft >= 0;
         timeLeft = timeout - _clock.getTime64( ))
    {
        if( _needGlobalLock )
            Global::enterCarbon();

        EventRef event;
        const float wait = LB_MIN( float( timeLeft ) * .001f, .05f /* 50ms */ );
        const OSStatus status = ReceiveNextEvent( 0, 0, wait, true, &event );
        if( status == noErr )
        {
            LBVERB << "Dispatch Carbon event " << event << std::endl;

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
            LBWARN << "ReceiveNextEvent failed: " << status << std::endl;
            return;
        }
    }
}

void MessagePump::dispatchAll()
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
            LBWARN << "ReceiveNextEvent failed: " << status << std::endl;
            break;
        }

        LBVERB << "Dispatch Carbon event " << event << std::endl;

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
}
#endif // AGL
