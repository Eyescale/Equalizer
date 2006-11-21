
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "commandQueue.h"

#include "command.h"
#include "node.h"
#include "packets.h"

using namespace eqNet;
using namespace std;

CommandQueue::CommandQueue()
        : _lastCommand(NULL)
{
}

CommandQueue::~CommandQueue()
{
    if( _lastCommand )
        _commandCache.release( _lastCommand );
}

void CommandQueue::push( Command& inCommand )
{
    _commandCacheLock.set();
    Command* outCommand = _commandCache.alloc( inCommand );
    _commandCacheLock.unset();

    // Note 1: REQ must always follow CMD
    // Note 2: Use of const_cast here is less ugly than passing around non-cast
    //         packets just for this one place were the packet is
    //         modified. Could also use mutable modifier for Packet::command.
    ++(*outCommand)->command;
    _commands.push( outCommand );
}

void CommandQueue::pushFront( Command& inCommand )
{
    _commandCacheLock.set();
    Command* outCommand = _commandCache.alloc( inCommand );
    _commandCacheLock.unset();

    ++(*outCommand)->command; // REQ must always follow CMD
    _commands.pushFront( outCommand );
}

Command* CommandQueue::pop()
{
    CHECK_THREAD( _thread );

    if( _lastCommand )
    {
        _commandCacheLock.set();
        _commandCache.release( _lastCommand );
        _commandCacheLock.unset();
    }
    
    _lastCommand = _commands.pop();
    return _lastCommand;
}

Command* CommandQueue::tryPop()
{
    CHECK_THREAD( _thread );

    Command* command = _commands.tryPop();
    if( !command )
        return 0;

    if( _lastCommand )
    {
        _commandCacheLock.set();
        _commandCache.release( _lastCommand );
        _commandCacheLock.unset();
    }
    
    _lastCommand = command;
    return command;
}

Command* CommandQueue::back() const
{
    return _commands.back();
}
