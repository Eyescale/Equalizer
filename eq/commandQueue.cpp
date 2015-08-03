
/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <co/iCommand.h>
#include <lunchbox/clock.h>

namespace eq
{
namespace
{
static lunchbox::Clock _clock;
}

CommandQueue::CommandQueue( const size_t maxSize )
    : co::CommandQueue( maxSize )
    , _messagePump( 0 )
    , _waitTime( 0 )
{}

CommandQueue::~CommandQueue()
{
    LBASSERT( !_messagePump );
    delete _messagePump;
    _messagePump = 0;
}

void CommandQueue::push( const co::ICommand& command )
{
    co::CommandQueue::push( command );
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::pushFront( const co::ICommand& command )
{
    co::CommandQueue::pushFront( command );
    if( _messagePump )
        _messagePump->postWakeup();
}

co::ICommand CommandQueue::pop( const uint32_t timeout )
{
    const int64_t start = _clock.getTime64();
    int64_t waitBegin = -1;
    while( true )
    {
        if( _messagePump )
            _messagePump->dispatchAll(); // non-blocking

        // Poll for a command
        if( !isEmpty( ))
        {
            if( waitBegin > -1 )
                _waitTime += ( _clock.getTime64() - waitBegin );
            return co::CommandQueue::pop( 0 );
        }

        if( _messagePump )
        {
            if( waitBegin == -1 )
                waitBegin = _clock.getTime64();
            _messagePump->dispatchOne( timeout ); // blocks - push sends wakeup
        }
        else
        {
            waitBegin = _clock.getTime64();
            // blocking
            const co::ICommand& command = co::CommandQueue::pop( timeout );
            _waitTime += ( _clock.getTime64() - waitBegin );
            return command;
        }

        if( _clock.getTime64() - start > timeout )
            return co::ICommand();
    }
}

co::ICommands CommandQueue::popAll( const uint32_t timeout )
{
    const int64_t start = _clock.getTime64();
    int64_t waitBegin = -1;
    while( true )
    {
        if( _messagePump )
            _messagePump->dispatchAll(); // non-blocking

        // Poll for commands
        if( !isEmpty( ))
        {
            if( waitBegin > -1 )
                _waitTime += ( _clock.getTime64() - waitBegin );
            return co::CommandQueue::popAll( 0 );
        }

        if( _messagePump )
        {
            if( waitBegin == -1 )
                waitBegin = _clock.getTime64();
            _messagePump->dispatchOne( timeout ); // blocks - push sends wakeup
        }
        else
        {
            waitBegin = _clock.getTime64();
            // blocking
            const co::ICommands& commands = co::CommandQueue::popAll( timeout );
            _waitTime += ( _clock.getTime64() - waitBegin );
            return commands;
        }

        if( _clock.getTime64() - start > timeout )
            return co::ICommands();
    }
}

co::ICommand CommandQueue::tryPop()
{
    if( _messagePump )
        _messagePump->dispatchAll(); // non-blocking

    return co::CommandQueue::tryPop();
}

void CommandQueue::pump()
{
    if( _messagePump )
        _messagePump->dispatchAll(); // non-blocking
}

}
