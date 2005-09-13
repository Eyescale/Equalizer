
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include <eq/packets.h>

using namespace eqs;
using namespace eqNet;
using namespace std;

Server::Server()
{
    for( int i=CMD_NODE_CUSTOM; i<CMD_SERVER_ALL; i++ )
        _cmdHandler[i - CMD_NODE_CUSTOM] =  &eqs::Server::_cmdUnknown;

    _cmdHandler[CMD_SERVER_CHOOSE_CONFIG - CMD_NODE_CUSTOM] =
        &eqs::Server::_cmdChooseConfig;
}

bool Server::run( int argc, char **argv )
{
    eqNet::init( argc, argv );

    Connection *connection = Connection::create(TYPE_TCPIP);
    ConnectionDescription connDesc;

    connDesc.parameters.TCPIP.port = 4242;
    if( !connection->listen( connDesc ))
        return false;

    if( !listen( connection ))
        return false;
    join();
    return true;
}

void Server::handlePacket( Node* node, const Packet* packet )
{
}

void Server::handleCommand( Node* node, const NodePacket* packet )
{
    INFO << "handle " << packet << " " << (packet->command - CMD_NODE_CUSTOM)<< endl;
    ASSERT( packet->command >= CMD_NODE_CUSTOM );

    if( packet->command < CMD_SERVER_ALL )
        (this->*_cmdHandler[packet->command - CMD_NODE_CUSTOM])(node, packet);
    else
        ERROR << "Unknown command " << packet->command << endl;
}

void Server::_cmdChooseConfig( Node* node, const Packet* pkg )
{
    ServerChooseConfigPacket* packet = (ServerChooseConfigPacket*)pkg;
    ASSERT( packet->appNameLength );

    char* appName = (char*)alloca(packet->appNameLength);
    node->recv( appName, packet->appNameLength );
    INFO << "Handle choose config " << packet << " appName " << appName << endl;

    // TODO
    Config* config = nConfigs()>0 ? getConfig(0) : NULL;
    
    if( config==NULL )
    {
        eq::ServerChooseConfigPacketReply reply;

    }
}
