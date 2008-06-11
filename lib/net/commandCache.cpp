
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "commandCache.h"

#include "command.h"
#include "node.h"
#include "packets.h"

using namespace std;

namespace eqNet
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
