
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <co/command.h>
#include <lunchbox/clock.h>

namespace eq
{
namespace
{
static lunchbox::Clock _clock;
}

CommandQueue::CommandQueue()
        : _messagePump( 0 )
        , _waitTime( 0 )
{}

CommandQueue::~CommandQueue()
{
    LBASSERT( !_messagePump );
    delete _messagePump;
    _messagePump = 0;
}

void CommandQueue::push( const co::Command& command )
{
    co::CommandQueue::push( command );
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::pushFront( const co::Command& command )
{
    co::CommandQueue::pushFront( command );
    if( _messagePump )
        _messagePump->postWakeup();
}

co::Command CommandQueue::pop()
{
    int64_t start = -1;
    while( true )
    {
        if( _messagePump )
            _messagePump->dispatchAll(); // non-blocking

        // Poll for a command
        if( !isEmpty( ))
        {
            if( start > -1 )
                _waitTime += ( _clock.getTime64() - start );
            return co::CommandQueue::pop();
        }

        if( _messagePump )
        {
            if( start == -1 )
                start = _clock.getTime64();
            _messagePump->dispatchOne(); // blocking - push will send wakeup
        }
        else
        {
            start = _clock.getTime64();
            const co::Command& command = co::CommandQueue::pop(); // blocking
            _waitTime += ( _clock.getTime64() - start );
            return command;
        }
    }
}

co::Command CommandQueue::tryPop()
{
    if( _messagePump )
        _messagePump->dispatchAll(); // non-blocking

    return co::CommandQueue::tryPop();
}

}
