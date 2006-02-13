
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "base.h"

#include "node.h"
#include "packets.h"

#include <eq/base/log.h>

using namespace eqNet;
using namespace std;

Base::Base( const uint32_t nCommands )
        : _nCommands( nCommands )
{
    _commandFunctions     = new CommandFcn[nCommands];
    _commandFunctionsThis = new Base*[nCommands];

    for( uint32_t i=0; i<nCommands; i++ )
        registerCommand( i, this, &eqNet::Base::_cmdUnknown );
}

Base::~Base()
{
    delete [] _commandFunctions;
    delete [] _commandFunctionsThis;
}

//===========================================================================
// command handling
//===========================================================================
void Base::registerCommand( const uint32_t command, void* thisPointer, 
                            CommandFcn handler )
{
    EQASSERT( command < _nCommands );
    _commandFunctions[command]     = handler;
    _commandFunctionsThis[command] = (Base*)thisPointer;
}

CommandResult Base::handleCommand( Node* node, const Packet* packet )
{
    const uint32_t which = packet->command;
    EQASSERT( which < _nCommands );
    return (_commandFunctionsThis[which]->*_commandFunctions[which])( node,
                                                                      packet );
}

CommandResult Base::_cmdUnknown( Node* node, const Packet* packet )
{
    EQERROR << "Unknown command " << packet << " from " << node << " class "
          << typeid(*this).name() << endl;
    return COMMAND_ERROR;
}
