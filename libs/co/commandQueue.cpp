
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

#include <co/base/mtQueue.h>

namespace co
{
namespace detail
{
class CommandQueue
{
public:
    /** Thread-safe command queue. */
    base::MTQueue< Command* > commands;
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
        EQWARN << "Flushing non-empty command queue" << std::endl;

    Command* command( 0 );
    while( _impl->commands.tryPop( command ))
    {
        EQWARN << *command << std::endl;
        EQASSERT( command );
        command->release();
    }
}

bool CommandQueue::isEmpty() const
{
    return _impl->commands.isEmpty();
}

size_t CommandQueue::getSize() const
{
    return _impl->commands.getSize();
}

void CommandQueue::push( Command& command )
{
    EQASSERT( command.isValid( ));
    command.retain();
    _impl->commands.push( &command );
}

void CommandQueue::pushFront( Command& command )
{
    EQASSERT( command.isValid( ));
    command.retain();
    _impl->commands.pushFront( &command );
}

void CommandQueue::wakeup()
{
    _impl->commands.push( static_cast< Command* >( 0 ));
}

Command* CommandQueue::pop( const uint32_t timeout )
{
    EQ_TS_THREAD( _thread );

    Command* command;
    if( !_impl->commands.timedPop( timeout, command ))
        throw Exception( Exception::TIMEOUT_COMMANDQUEUE );

    return command;
}

Command* CommandQueue::tryPop()
{
    EQ_TS_THREAD( _thread );
    Command* command = 0;
    _impl->commands.tryPop( command );
    return command;
}

}

