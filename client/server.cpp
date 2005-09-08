
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "node.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Server::Server()
        : _state( STATE_STOPPED )
{}

bool Server::open( const string& address )
{
    if( _state != STATE_STOPPED )
        return false;

    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create(eqNet::TYPE_TCPIP);

    eqNet::ConnectionDescription connDesc;
    const size_t colonPos = address.rfind( ':' );

    if( colonPos == string::npos )
        connDesc.hostname = address;
    else
    {
        connDesc.hostname = address.substr( 0, colonPos-1 );
        string port = address.substr( colonPos );
        connDesc.parameters.TCPIP.port = atoi( port.c_str( ));
    }

    if( !connDesc.parameters.TCPIP.port )
        connDesc.parameters.TCPIP.port = 4242;

    if( !connection->connect( connDesc ))
        return false;

    Node* localNode = eq::Node::getLocalNode();
    if( !localNode->connect( this, connection ))
        return false;

    _state = STATE_OPENED;
    return true;
}

bool Server::close()
{
    if( _state != STATE_OPENED )
        return false;

    Node* localNode = eq::Node::getLocalNode();
    if( !localNode->disconnect( this ))
        return false;

    _state = STATE_STOPPED;
    return true;
}

Config* Server::chooseConfig( const ConfigParams* parameters )
{
}

void Server::releaseConfig( Config* config )
{
    // TODO
}
