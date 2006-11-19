
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "commandCache.h"

#include "node.h"
#include "packets.h"

using namespace eqNet;

CommandCache::CommandCache()
{}

CommandCache::~CommandCache()
{
    while( !_freeCommands.empty( ))
    {
        Command* holder = _freeCommands.front();
        _freeCommands.pop_front();
        delete holder;
    }
}

Command* CommandCache::alloc( Command& inHolder )
{
    Command* outHolder;

    if( _freeCommands.empty( ))
        outHolder = new Command;
    else
    {
        outHolder = _freeCommands.front();
        _freeCommands.pop_front();
    }

    *outHolder = inHolder;
    return outHolder;
}

void CommandCache::release( Command* holder )
{
    EQASSERT( holder->isValid( ));

    if( (*holder)->exceedsMinSize( )) // old packet was 'big', release
        holder->release();
    
    _freeCommands.push_back( holder );
}

