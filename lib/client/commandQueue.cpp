
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

namespace eq
{
CommandQueue::CommandQueue()
        : _messagePump( 0 )
{
}

CommandQueue::~CommandQueue()
{
    EQASSERT( !_messagePump );
    delete _messagePump;
    _messagePump = 0;
}

void CommandQueue::push(net::Command& inCommand)
{
    net::CommandQueue::push(inCommand);
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::pushFront(net::Command& inCommand)
{
    net::CommandQueue::pushFront(inCommand);
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::wakeup()
{
    net::CommandQueue::wakeup();
    if( _messagePump )
        _messagePump->postWakeup();
}

net::Command* CommandQueue::pop()
{
    while( true )
    {
        if( _messagePump )
            _messagePump->dispatchAll(); // non-blocking

        // Poll for a command
        if( !isEmpty( ))
            return net::CommandQueue::pop();

        if( _messagePump )
            _messagePump->dispatchOne(); // blocking - push will send wakeup
        else
            return net::CommandQueue::pop();
    }
}

net::Command* CommandQueue::tryPop()
{
    if( _messagePump )
        _messagePump->dispatchAll(); // non-blocking

    return net::CommandQueue::tryPop();
}

void CommandQueue::flush()
{
    if( _messagePump )
        _messagePump->dispatchDone();

    net::CommandQueue::flush();
}

}
