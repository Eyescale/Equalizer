
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
    if( which >= _nCommands )
    {
        EQERROR << "Command higher than number of registered command handlers "
                << "for object of type " << typeid(*this).name() << endl;
        return COMMAND_ERROR;
    }
    return (_commandFunctionsThis[which]->*_commandFunctions[which])( node,
                                                                      packet );
}

CommandResult Base::_cmdUnknown( Node* node, const Packet* packet )
{
    switch( packet->datatype )
    {
        case DATATYPE_EQNET_NODE:
            EQERROR << "Unknown command " << (NodePacket*)packet << " from "
                    << node << " class " << typeid(*this).name() << endl;
            break;
        case DATATYPE_EQNET_SESSION:
            EQERROR << "Unknown command " << (SessionPacket*)packet << " from "
                    << node << " class " << typeid(*this).name() << endl;
            break;
        case DATATYPE_EQNET_OBJECT:
            EQERROR << "Unknown command " << (ObjectPacket*)packet << " from "
                    << node << " class " << typeid(*this).name() << endl;
            break;

        default:
            EQERROR << "Unknown command " << packet << " from " << node 
                    << " class " << typeid(*this).name() << endl;
            break;
    }

    return COMMAND_ERROR;
}
