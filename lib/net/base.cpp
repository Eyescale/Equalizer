
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

Base::Base( const bool threadSafe )
        : _requestHandler( threadSafe )
{
}

Base::~Base()
{
}

//===========================================================================
// command handling
//===========================================================================
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

CommandResult Base::_cmdUnknown( Command& packet )
{
    EQERROR << "Unknown command " << packet << " class " << typeid(*this).name()
            << endl;
    return COMMAND_ERROR;
}
