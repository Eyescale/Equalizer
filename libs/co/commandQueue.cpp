
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "command.h"
#include "exception.h"
#include "node.h"

#include <lunchbox/mtQueue.h>

namespace co
{
namespace detail
{
class CommandQueue
{
public:
    /** Thread-safe command queue. */
    lunchbox::MTQueue< CommandPtr > commands;
};
}

CommandQueue::CommandQueue()
        : _impl( new detail::CommandQueue )
{
}

CommandQueue::~CommandQueue()
{
    flush();
    delete _impl;
}

void CommandQueue::flush()
{
    if( !isEmpty( ))
        LBWARN << "Flushing non-empty command queue" << std::endl;

    _impl->commands.clear();
}

bool CommandQueue::isEmpty() const
{
    return _impl->commands.isEmpty();
}

size_t CommandQueue::getSize() const
{
    return _impl->commands.getSize();
}

void CommandQueue::push( CommandPtr command )
{
    LBASSERT( command->isValid( ));
    _impl->commands.push( command );
}

void CommandQueue::pushFront( CommandPtr command )
{
    LBASSERT( command->isValid( ));
    _impl->commands.pushFront( command );
}

void CommandQueue::wakeup()
{
    _impl->commands.push( CommandPtr( ));
}

CommandPtr CommandQueue::pop( const uint32_t timeout )
{
    LB_TS_THREAD( _thread );

    CommandPtr command;
    if( !_impl->commands.timedPop( timeout, command ))
        throw Exception( Exception::TIMEOUT_COMMANDQUEUE );

    return command;
}

CommandPtr CommandQueue::tryPop()
{
    LB_TS_THREAD( _thread );
    CommandPtr command;
    _impl->commands.tryPop( command );
    return command;
}

}

