
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "commandQueue.h"

#include "messagePump.h"
#include <co/base/clock.h>

namespace eq
{
namespace
{
static co::base::Clock _clock;
}

CommandQueue::CommandQueue()
        : _messagePump( 0 )
        , _waitTime( 0 )
{}

CommandQueue::~CommandQueue()
{
    EQASSERT( !_messagePump );
    delete _messagePump;
    _messagePump = 0;
}

void CommandQueue::push(co::Command& inCommand)
{
    co::CommandQueue::push( inCommand );
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::pushFront(co::Command& inCommand)
{
    co::CommandQueue::pushFront( inCommand );
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::wakeup()
{
    co::CommandQueue::wakeup();
    if( _messagePump )
        _messagePump->postWakeup();
}

co::Command* CommandQueue::pop()
{
    const int64_t start = _clock.getTime64();
    while( true )
    {
        if( _messagePump )
            _messagePump->dispatchAll(); // non-blocking

        // Poll for a command
        if( !isEmpty( ))
        {
            _waitTime += ( _clock.getTime64() - start );
            return co::CommandQueue::pop();
        }

        if( _messagePump )
            _messagePump->dispatchOne(); // blocking - push will send wakeup
        else
        {
            _waitTime += ( _clock.getTime64() - start );
            return co::CommandQueue::pop(); // blocking
        }
    }
}

co::Command* CommandQueue::tryPop()
{
    if( _messagePump )
        _messagePump->dispatchAll(); // non-blocking

    return co::CommandQueue::tryPop();
}

}
