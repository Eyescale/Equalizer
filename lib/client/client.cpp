
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "client.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

Client::Client()
        : Node( CMD_CLIENT_ALL ),
          _clientLoopRunning(false)
{
    registerCommand( eqNet::CMD_NODE_STOP, this, reinterpret_cast<CommandFcn>( 
                         &eq::Client::_cmdStop ));
    registerCommand( eqNet::REQ_NODE_STOP, this, reinterpret_cast<CommandFcn>( 
                         &eq::Client::_reqStop ));
}

Client::~Client()
{
}

eqBase::RefPtr<eqNet::Node> Client::createNode( const CreateReason reason )
{ 
    switch( reason )
    {
        case REASON_CLIENT_CONNECT:
            return new Server;

        default:
            return new eqNet::Node;
    }
}

eqNet::Session* Client::createSession()
{
    return Global::getNodeFactory()->createConfig(); 
}

void Client::clientLoop()
{
    _clientLoopRunning = true;

    eqNet::Node*   node;
    eqNet::Packet* packet;

    while( _clientLoopRunning )
    {
        _requestQueue.pop( &node, &packet );

        switch( dispatchPacket( node, packet ))
        {
            case eqNet::COMMAND_PROPAGATE:
                EQWARN << "COMMAND_PROPAGATE returned, but nowhere to propagate"
                       << endl;
                break;

            case eqNet::COMMAND_HANDLED:
                break;

            case eqNet::COMMAND_ERROR:
                EQERROR << "Error handling command packet" << endl;
                abort();

            case eqNet::COMMAND_RESCHEDULE:
                EQUNIMPLEMENTED;
        }
    }
}

eqNet::CommandResult Client::handlePacket( eqNet::Node* node,
                                           const eqNet::Packet* packet )
{
    EQVERB << "handlePacket " << packet << endl;
    const uint32_t datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQ_SERVER:
            EQASSERT( dynamic_cast<Server*>(node) );

            Server* server = static_cast<Server*>(node);
            return server->handleCommand( node, packet );

        default:
            return eqNet::COMMAND_ERROR;
    }
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Client::_cmdStop( eqNet::Node* node, 
                                       const eqNet::Packet* packet)
{
    if( _clientLoopRunning )
    {
        pushRequest( node, packet );
        return eqNet::COMMAND_HANDLED;
    }

    return eqNet::Node::_cmdStop( node, packet );
}

eqNet::CommandResult Client::_reqStop( eqNet::Node* node, 
                                       const eqNet::Packet* packet )
{
    EQINFO << "handle node stop " << packet << endl;
    _clientLoopRunning = false;

    eqNet::NodeStopPacket stopPacket;
    send( stopPacket );

    return eqNet::COMMAND_HANDLED;
}
