
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "client.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "server.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Client::Client()
{
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
    _used.waitGE( 1 );  // Wait to be used at least once (see Node::_reqInit)
    _used.waitEQ( 0 );  // Wait to become unused (see Node::_reqExit)

    EQINFO << "Stopping client node" << endl;
    eqNet::NodeStopPacket packet;
    send( packet );
}

eqNet::CommandResult Client::handleCommand( eqNet::Command& command )
{
    EQVERB << "handleCommand " << command << endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_SERVER:
        {
            RefPtr<eqNet::Node> node = command.getNode();

            EQASSERT( dynamic_cast<Server*>( node.get( )) );
            RefPtr<Server> server = 
                RefPtr_static_cast<eqNet::Node, Server>( node );

            return server->invokeCommand( command );
        }
        default:
            return eqNet::COMMAND_ERROR;
    }
}
