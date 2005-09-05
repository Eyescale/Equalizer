
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

using namespace eqs;
using namespace eqNet;
using namespace std;

Server::Server()
{}

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
}
