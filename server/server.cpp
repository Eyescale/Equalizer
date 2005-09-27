
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "config.h"
#include "node.h"

#include <eq/packets.h>
#include <eq/base/refPtr.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

Server::Server()
        : _configID(1)
{
    for( int i=eqNet::CMD_NODE_CUSTOM; i<eq::CMD_SERVER_ALL; i++ )
        _cmdHandler[i - eqNet::CMD_NODE_CUSTOM] =  &eqs::Server::_cmdUnknown;

    _cmdHandler[eq::CMD_SERVER_CHOOSE_CONFIG - eqNet::CMD_NODE_CUSTOM] =
        &eqs::Server::_cmdChooseConfig;
    _cmdHandler[eq::CMD_SERVER_RELEASE_CONFIG - eqNet::CMD_NODE_CUSTOM] =
        &eqs::Server::_cmdReleaseConfig;
}

bool Server::run( int argc, char **argv )
{
    eqNet::init( argc, argv );
    if( !_loadConfig( argc, argv ))
        return false;

    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create(eqNet::TYPE_TCPIP);
    eqNet::ConnectionDescription connDesc;

    connDesc.parameters.TCPIP.port = 4242;
    if( !connection->listen( connDesc ))
        return false;

    if( !listen( connection ))
        return false;
    join();
    return true;
}

eqNet::Node* Server::handleConnect( RefPtr<eqNet::Connection> connection )
{
    Node* node = new Node();
    if( connect( node, connection ))
        return node;

    delete node;
    return NULL;
}

void Server::handleDisconnect( Node* node )
{
    const bool disconnected = disconnect( node );
    ASSERT( disconnected );

    // TODO: free up resources requested by this node
}

bool Server::_loadConfig( int argc, char **argv )
{
    // TODO
    Config* config = new Config();
    addConfig( config );
    return true;
}

void Server::handlePacket( eqNet::Node* node, const eqNet::Packet* packet )
{
}

void Server::handleCommand( eqNet::Node* node, const eqNet::NodePacket* packet )
{
    VERB << "handle " << packet << endl;
    ASSERT( packet->command >= eqNet::CMD_NODE_CUSTOM );

    if( packet->command < eq::CMD_SERVER_ALL )
        (this->*_cmdHandler[packet->command - eqNet::CMD_NODE_CUSTOM])
            (node, packet);
    else
        ERROR << "Unknown command " << packet->command << endl;
}

void Server::_cmdChooseConfig( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ServerChooseConfigPacket* packet = (eq::ServerChooseConfigPacket*)pkg;
    ASSERT( packet->appNameLength );

    char* appName = (char*)alloca(packet->appNameLength);
    node->recv( appName, packet->appNameLength );
    INFO << "Handle choose config " << packet << " appName " << appName << endl;

    // TODO
    Config* config = nConfigs()>0 ? getConfig(0) : NULL;
    
    eq::ServerChooseConfigReplyPacket reply;
    reply.requestID = packet->requestID;

    if( config==NULL )
    {
        reply.configID = INVALID_ID;
        node->send( reply );
        return;
    }

    Config* appConfig = new Config( *config );

    reply.configID = _configID++;

    appConfig->setID( reply.configID );
    _appConfigs[reply.configID] = appConfig;
    node->send( reply );
}

void Server::_cmdReleaseConfig( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ServerReleaseConfigPacket* packet = (eq::ServerReleaseConfigPacket*)pkg;
    INFO << "Handle release config " << packet << endl;

    Config* appConfig = _appConfigs[packet->configID];
    if( !appConfig )
    {
        WARN << "Release request for unknown config" << endl;
        return;
    }

    _appConfigs.erase( packet->configID );
    delete appConfig;
}
