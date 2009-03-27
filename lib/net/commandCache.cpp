
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "commandCache.h"

#include "command.h"
#include "node.h"
#include "packets.h"

using namespace std;

namespace eq
{
namespace net
{
CommandCache::CommandCache()
{}

CommandCache::~CommandCache()
{
    flush();
}

void CommandCache::flush()
{
    for( vector< Command* >::const_iterator i = _freeCommands.begin();
         i != _freeCommands.end(); ++i )
    {
        Command* command = *i;
        delete command;
    }
    _freeCommands.clear();
}

Command* CommandCache::alloc( Command& inCommand )
{
    Command* outCommand;

    if( _freeCommands.empty( ))
        outCommand = new Command;
    else
    {
        outCommand = _freeCommands.back();
        _freeCommands.pop_back();
    }

    outCommand->swap( inCommand );
    return outCommand;
}

void CommandCache::release( Command* command )
{
#ifndef NDEBUG
    // Unref nodes in command to keep node ref counts easier for debugging.
    // Release builds will unref the nodes when the queues are flushed at the
    // exit of the consumer threads.
    command->allocate( 0, 0, 1 );
#endif

    if( command->isValid() && (*command)->exceedsMinSize( ))
        // old packet was 'big', release
        command->release();
    
    _freeCommands.push_back( command );
}
}
}
