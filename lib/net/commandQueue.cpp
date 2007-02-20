
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "commandQueue.h"

#include "command.h"
#include "node.h"
#include "packets.h"

using namespace eqNet;
using namespace std;

CommandQueue::CommandQueue()
        : _lastCommand(0)
#ifdef WIN32
        , _win32ThreadID(0)
#endif
{
}

CommandQueue::~CommandQueue()
{
    flush();
}

void CommandQueue::flush()
{
    EQASSERT( empty( ));

    _commandCacheLock.set();
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

    // Note 1: REQ must always follow CMD
    // Note 2: Use of const_cast here is less ugly than passing around non-cast
    //         packets just for this one place were the packet is
    //         modified. Could also use mutable modifier for Packet::command.
    ++(*outCommand)->command;
    _commands.push( outCommand );

#ifdef WIN32
    if( _win32ThreadID )
        PostThreadMessage( _win32ThreadID, WM_APP, 0, 0 ); // Wake up pop()
#endif
}

void CommandQueue::pushFront( Command& inCommand )
{
    _commandCacheLock.set();
    Command* outCommand = _commandCache.alloc( inCommand );
    _commandCacheLock.unset();

    ++(*outCommand)->command; // REQ must always follow CMD
    _commands.pushFront( outCommand );

#ifdef WIN32
    if( _win32ThreadID )
        PostThreadMessage( _win32ThreadID, WM_APP, 0, 0 ); // Wake up pop()
#endif
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
#ifdef WIN32
    else
    {
        // First call - force creation of thread message queue
        MSG msg;
        PeekMessage( &msg, 0, WM_USER, WM_USER, PM_NOREMOVE );
        _win32ThreadID = GetCurrentThreadId();
    }

    while( true )
    {
        // Nonblocking windows message pump
        MSG msg;
        while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        // Poll for a command
        _lastCommand = _commands.tryPop();
        if( _lastCommand )
            return _lastCommand;

        // Blocking windows message pump - push will send 'fake' message
        if( GetMessage( &msg, 0, 0, 0 ))
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
#else
    _lastCommand = _commands.pop();
    return _lastCommand;
#endif
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
