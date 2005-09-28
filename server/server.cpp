
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "appConfig.h"
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
    VERB << "handlePacket " << packet << endl;
    const uint datatype = packet->datatype;

    switch( datatype )
    {
        case eq::DATATYPE_EQ_CONFIG:
        {
            const eq::ConfigPacket* configPacket = (eq::ConfigPacket*)packet;
            const uint              configID     = configPacket->configID;
            AppConfig*              config       = _appConfigs[configID];
            ASSERT( config );

            config->handleCommand( node, configPacket );
        }
        break;

        default:
            ERROR << "unimplemented" << endl;
            abort();
    }
}

void Server::handleCommand( eqNet::Node* node, const eqNet::NodePacket* packet )
{
    VERB << "handleCommand " << packet << endl;
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
    ASSERT( packet->renderClientLength );

    char* appName = (char*)alloca(packet->appNameLength);
    node->recv( appName, packet->appNameLength );

    char* renderClient = (char*)alloca(packet->renderClientLength);
    node->recv( renderClient, packet->renderClientLength );

    INFO << "Handle choose config " << packet << " appName " << appName 
         << " renderClient " << renderClient << endl;

    // TODO
    Config* config = nConfigs()>0 ? getConfig(0) : NULL;
    
    eq::ServerChooseConfigReplyPacket reply( packet );

    if( config==NULL )
    {
        reply.configID = INVALID_ID;
        node->send( reply );
        return;
    }

    reply.configID = _configID++;

    AppConfig* appConfig = new AppConfig( *config );

    appConfig->setID( reply.configID );
    appConfig->setAppName( appName );
    appConfig->setRenderClient( renderClient );

    _appConfigs[reply.configID] = appConfig;
    node->send( reply );
}

void Server::_cmdReleaseConfig( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ServerReleaseConfigPacket* packet = (eq::ServerReleaseConfigPacket*)pkg;
    INFO << "Handle release config " << packet << endl;

    AppConfig* appConfig = _appConfigs[packet->configID];
    if( !appConfig )
    {
        WARN << "Release request for unknown config" << endl;
        return;
    }

    _appConfigs.erase( packet->configID );
    delete appConfig;
}
