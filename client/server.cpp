
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "config.h"
#include "configParams.h"
#include "node.h"
#include "packets.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Server::Server()
        : _state( STATE_STOPPED )
{
    for( int i=eqNet::CMD_NODE_CUSTOM; i<CMD_SERVER_ALL; i++ )
        _cmdHandler[i - eqNet::CMD_NODE_CUSTOM] =  &eq::Server::_cmdUnknown;

    _cmdHandler[CMD_SERVER_CHOOSE_CONFIG_REPLY - eqNet::CMD_NODE_CUSTOM] =
        &eq::Server::_cmdChooseConfigReply;

    INFO << "New server at " << (void*)this << endl;
}

bool Server::open( const string& address )
{
    if( _state != STATE_STOPPED )
        return false;

    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create(eqNet::TYPE_TCPIP);

    RefPtr<eqNet::ConnectionDescription> connDesc = 
        new eqNet::ConnectionDescription;

    const size_t colonPos = address.rfind( ':' );

    if( colonPos == string::npos )
        connDesc->hostname = address;
    else
    {
        connDesc->hostname = address.substr( 0, colonPos );
        string port = address.substr( colonPos+1 );
        connDesc->parameters.TCPIP.port = atoi( port.c_str( ));
    }

    if( !connDesc->parameters.TCPIP.port )
        connDesc->parameters.TCPIP.port = 4242;

    if( !connection->connect( connDesc ))
        return false;

    eqNet::Node* localNode = Node::getLocalNode();
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
    if( _state != STATE_OPENED )
        return NULL;

    ServerChooseConfigPacket packet;
    packet.requestID          = _requestHandler.registerRequest();
    packet.appNameLength      = parameters->appName.size() + 1;
    packet.renderClientLength = parameters->renderClient.size() + 1;
    packet.compoundModes      = parameters->compoundModes;

    send( packet );
    send( parameters->appName.c_str(),      packet.appNameLength );
    send( parameters->renderClient.c_str(), packet.renderClientLength );

    const void* result = _requestHandler.waitRequest( packet.requestID );
    return (Config*)result;
}

void Server::releaseConfig( Config* config )
{
    if( _state != STATE_OPENED )
        return;

    ServerReleaseConfigPacket packet;
    packet.configID = config->getID();
    send( packet );
    delete config;
}

void Server::handlePacket( const eqNet::Packet* packet )
{
    VERB << "handlePacket " << packet << endl;
    const uint datatype = packet->datatype;

    switch( datatype )
    {
        case eq::DATATYPE_EQ_CONFIG:
        {
            const ConfigPacket* configPacket = (ConfigPacket*)packet;
            const uint          configID     = configPacket->configID;
            Config*             config       = _configs[configID];
            ASSERT( config );

            config->handleCommand( configPacket );
        }
        break;

        default:
            ERROR << "unimplemented" << endl;
            abort();
    }
}

void Server::handleCommand( const eqNet::NodePacket* packet )
{
    INFO << "handle " << packet << endl;
    ASSERT( packet->command >= eqNet::CMD_NODE_CUSTOM );
    ASSERT( packet->command <  CMD_SERVER_ALL );

    (this->*_cmdHandler[packet->command - eqNet::CMD_NODE_CUSTOM])(packet);
}

void Server::_cmdUnknown( const eqNet::Packet* pkg )
{
    ERROR << "Unknown packet " << pkg << endl;
}

void Server::_cmdChooseConfigReply( const eqNet::Packet* pkg )
{
    ServerChooseConfigReplyPacket* packet = (ServerChooseConfigReplyPacket*)pkg;
    INFO << "Handle choose config reply " << packet << endl;

    if( packet->configID == INVALID_ID )
    {
        _requestHandler.serveRequest( packet->requestID, NULL );
        return;
    }
    
    Config* config = new Config( packet->configID, this );

    _configs[packet->configID] = config;
    _requestHandler.serveRequest( packet->requestID, config );
}
