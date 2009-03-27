
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
    _commandCacheLock.set();

    if( !empty( ))
        EQWARN << "Flushing non-empty command queue" << endl;

    while( !_commands.empty( ))
    {
        Command* command = _commands.pop();
        EQWARN << *command << endl;
        if( command )
            _commandCache.release( command );
    }

    _commandCache.flush();
    _commandCacheLock.unset();
}

void CommandQueue::push( Command& inCommand )
{
    EQASSERT( inCommand.isValid( ));

    _commandCacheLock.set();
    Command* outCommand = _commandCache.alloc( inCommand );
    _commandCacheLock.unset();

    EQASSERT( outCommand->isValid( ));
    outCommand->_dispatched = true;
    _commands.push( outCommand );
}

void CommandQueue::pushFront( Command& inCommand )
{
    EQASSERT( inCommand.isValid( ));

    _commandCacheLock.set();
    Command* outCommand = _commandCache.alloc( inCommand );
    _commandCacheLock.unset();

    EQASSERT( outCommand->isValid( ));
    outCommand->_dispatched = true;
    _commands.pushFront( outCommand );
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

void CommandQueue::release( Command* command )
{
    if( !command )
        return;

    _commandCacheLock.set();
    _commandCache.release( command );
    _commandCacheLock.unset();
}

Command* CommandQueue::back() const
{
    return _commands.back();
}
}
}

