
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "base.h"

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
CommandResult Base::handleCommand( Node* node, const Packet* packet )
{
    const uint32_t which = packet->command;
    if( which >= _vTable.size( ))
    {
        EQERROR << "Command " << which
                << " higher than number of registered command handlers ("
                << _vTable.size() << ") for object of type "
                << typeid(*this).name() << endl;
        return COMMAND_ERROR;
    }
    return _vTable[which]( node, packet );
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
