
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <pthread.h>
#include "commandQueue.h"

#include "command.h"
#include "node.h"
#include "packets.h"

using namespace std;

namespace eq
{
namespace net
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
    if( !empty( ))
        EQWARN << "Flushing non-empty command queue" << endl;

    Command* command( 0 );
    while( (command = _commands.tryPop( )) )
    {
        EQWARN << *command << endl;
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

Command* CommandQueue::pop()
{
    CHECK_THREAD( _thread );
    return _commands.pop();
}

Command* CommandQueue::tryPop()
{
    CHECK_THREAD( _thread );
    return _commands.tryPop();
}

Command* CommandQueue::back() const
{
    return _commands.back();
}
}
}

