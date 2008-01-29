/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "commandQueue.h"

#include "command.h"
#include "node.h"
#include "packets.h"

using namespace std;

namespace eqNet
{
CommandQueue::CommandQueue()
        : _lastCommand(0)
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
#ifndef NDEBUG
    while( !_commands.empty( ))
        EQINFO << _commands.pop() << endl;
#endif

    if( _lastCommand )
        _commandCache.release( _lastCommand );
    _lastCommand = 0;

    _commandCache.flush();
    _commandCacheLock.unset();
}

void CommandQueue::push( Command& inCommand )
{
    _commandCacheLock.set();
    Command* outCommand = _commandCache.alloc( inCommand );
    _commandCacheLock.unset();

    // Note: REQ must always follow CMD
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
}

