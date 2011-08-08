
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

namespace co
{
CommandQueue::CommandQueue()
{
}

CommandQueue::~CommandQueue()
{
    flush();
}

void CommandQueue::flush()
{
    if( !isEmpty( ))
        EQWARN << "Flushing non-empty command queue" << std::endl;

    Command* command( 0 );
    while( _commands.tryPop( command ))
    {
        EQWARN << *command << std::endl;
        EQASSERT( command );
        command->release();
    }
}

void CommandQueue::push( Command& command )
{
    EQASSERT( command.isValid( ));
    command.retain();
    _commands.push( &command );
}

void CommandQueue::pushFront( Command& command )
{
    EQASSERT( command.isValid( ));
    command.retain();
    _commands.pushFront( &command );
}

Command* CommandQueue::pop( const uint32_t timeout )
{
    EQ_TS_THREAD( _thread );

    Command* command;
    if( !_commands.timedPop( timeout, command ))
        throw Exception( Exception::TIMEOUT_COMMANDQUEUE );

    return command;
}

Command* CommandQueue::tryPop()
{
    EQ_TS_THREAD( _thread );
    Command* command = 0;
    _commands.tryPop( command );
    return command;
}

}

