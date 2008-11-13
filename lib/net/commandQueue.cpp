/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
        release( command );
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

