
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "base.h"

#include "command.h"
#include "node.h"
#include "packets.h"

#include <eq/base/log.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

Base::Base()
{
}

Base::Base( const Base& from )
{
}

Base::~Base()
{
}

//===========================================================================
// command handling
//===========================================================================
void Base::_registerCommand( const uint32_t command,
                             const CommandFunc<Base>& func)
{
    if( _vTable.size() <= command )
    {
        while( _vTable.size() < command )
            _vTable.push_back( CommandFunc<Base>( this, &Base::_cmdUnknown ));

        _vTable.push_back( func );
        EQASSERT( _vTable.size() == command + 1 );
    }
    else
        _vTable[command] = func;
}


CommandResult Base::invokeCommand( Command& command )
{
    const uint32_t which = command->command;
    if( which >= _vTable.size( ))
    {
        EQERROR << "Command " << which
                << " higher than number of registered command handlers ("
                << _vTable.size() << ") for object of type "
                << typeid(*this).name() << endl;
        return COMMAND_ERROR;
    }
    return _vTable[which]( command );
}

CommandResult Base::_cmdUnknown( Command& command )
{
    EQERROR << "Unknown " << command << ", type " << typeid(*this).name()
            << endl;
    return COMMAND_ERROR;
}
