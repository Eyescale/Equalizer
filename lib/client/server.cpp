
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "config.h"
#include "configParams.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "openParams.h"
#include "packets.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Server::Server()
        : _state( STATE_STOPPED )
{
    registerCommand( CMD_SERVER_CREATE_CONFIG, 
               eqNet::PacketFunc<Server>( this, &Server::_cmdCreateConfig ));
    registerCommand( CMD_SERVER_CHOOSE_CONFIG_REPLY, 
          eqNet::PacketFunc<Server>( this, &Server::_cmdChooseConfigReply ));

    EQINFO << "New server at " << (void*)this << endl;
}

bool Server::open( const OpenParams& params )
{
    if( _state != STATE_STOPPED )
        return false;

    if( nConnectionDescriptions() == 0 )
    {
        RefPtr<eqNet::ConnectionDescription> connDesc = 
            new eqNet::ConnectionDescription;
    
        connDesc->type = eqNet::Connection::TYPE_TCPIP;
        
        const char*  envServer = getenv( "EQ_SERVER" );
        const string address   = params.address.size() > 0 ? params.address :
                             envServer ? envServer : "localhost";
        const size_t colonPos  = address.rfind( ':' );

        if( colonPos == string::npos )
            connDesc->hostname = address;
        else
        {
            connDesc->hostname   = address.substr( 0, colonPos );
            string port          = address.substr( colonPos+1 );
            connDesc->TCPIP.port = atoi( port.c_str( ));
        }

        if( !connDesc->TCPIP.port )
            connDesc->TCPIP.port = EQ_DEFAULT_PORT;

        addConnectionDescription( connDesc );
    }

    if( connect( ))
    {
        // TODO?: send open packet (appName)
        _state = STATE_OPENED;
        return true;
    }

    //if( params.address.size()>0 || getenv("EQ_SERVER"))
    return false;

    // TODO: Use app-local server if no server was requested explicitly
}

bool Server::close()
{
    if( _state != STATE_OPENED )
        return false;

    if( !disconnect( ))
        return false;

    _state = STATE_STOPPED;
    return true;
}

Config* Server::chooseConfig( const ConfigParams& parameters )
{
    if( _state != STATE_OPENED )
        return NULL;

    ServerChooseConfigPacket packet;

    packet.requestID          = _requestHandler.registerRequest();
    packet.compoundModes      = parameters.compoundModes;
    const std::string rendererInfo = parameters.workDir + ":" + 
        parameters.renderClient;

    send( packet, rendererInfo );

    Config* config = (Config*)_requestHandler.waitRequest( packet.requestID );
    if( !config )
        return NULL;

    return config;
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

void Server::_addConfig( Config* config )
{
    EQASSERT( config->getID() != EQ_ID_INVALID );
    _configs[config->getID()] = config;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Server::_cmdCreateConfig( eqNet::Command& command )
{
    const ServerCreateConfigPacket* packet = 
        command.getPacket<ServerCreateConfigPacket>();
    EQINFO << "Handle create config " << packet << ", name " << packet->name 
         << endl;
    
    RefPtr<Node>    localNode  = Node::getLocalNode();
    eqNet::Session* session    = localNode->getSession( packet->configID );
    if( session )
    {
        EQASSERT( dynamic_cast<Config*>( session ));

        Config* config = (Config*)session;
        config->_appNodeID = packet->appNodeID;
        return eqNet::COMMAND_HANDLED;
    }
 
    Config* config = Global::getNodeFactory()->createConfig();
    config->_appNodeID = packet->appNodeID;
    localNode->addSession( config, command.getNode(), packet->configID,
                           packet->name );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdChooseConfigReply( eqNet::Command& command )
{
    const ServerChooseConfigReplyPacket* packet = 
        command.getPacket<ServerChooseConfigReplyPacket>();
    EQINFO << "Handle choose config reply " << packet << endl;

    if( packet->configID == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( packet->requestID, NULL );
        return eqNet::COMMAND_HANDLED;
    }

    Config*      config    = Global::getNodeFactory()->createConfig();
    RefPtr<Node> localNode = Node::getLocalNode();

    localNode->addSession( config, command.getNode(), packet->configID, 
                           packet->sessionName);
    _addConfig( config );

    _requestHandler.serveRequest( packet->requestID, config );
    return eqNet::COMMAND_HANDLED;
}
