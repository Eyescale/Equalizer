
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectDeltaDataIStream.h"

#include "command.h"
#include "commands.h"

#include <eq/base/idPool.h>

using namespace std;

namespace eqNet
{
ObjectDataIStream::ObjectDataIStream()
        : _lastCommand( 0 )
        , _version( EQ_ID_INVALID )
{
}

ObjectDataIStream::~ObjectDataIStream()
{
    reset();
}

void ObjectDataIStream::reset()
{
    DataIStream::reset();

    delete _lastCommand;
    _lastCommand = 0;

    while( !_commands.empty( ))
    {
        delete _commands.front();
        _commands.pop_front();
    }
}

void ObjectDataIStream::addDataPacket( const Command& command )
{
    // Copy packet since it is sent to all instances
    // OPT: Do not make copy for one instance or use ref counting?

    Command* copy = new Command( command );
    _commands.push_back( copy );
}

const Command* ObjectDataIStream::getNextCommand()
{
    delete _lastCommand;
    _lastCommand = 0;
    if( _commands.empty( ))
        return 0;
    
    _lastCommand = _commands.front();
    _commands.pop_front();
    return _lastCommand; 
}
}
