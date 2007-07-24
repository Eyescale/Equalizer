
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "client.h"
#include "config.h"
#include "configParams.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "packets.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Server::Server()
{
    registerCommand( CMD_SERVER_CREATE_CONFIG, 
                 eqNet::CommandFunc<Server>( this, &Server::_cmdCreateConfig ));
    registerCommand( CMD_SERVER_DESTROY_CONFIG, 
                eqNet::CommandFunc<Server>( this, &Server::_cmdDestroyConfig ));
    registerCommand( CMD_SERVER_CHOOSE_CONFIG_REPLY, 
            eqNet::CommandFunc<Server>( this, &Server::_cmdChooseConfigReply ));
    registerCommand( CMD_SERVER_RELEASE_CONFIG_REPLY, 
           eqNet::CommandFunc<Server>( this, &Server::_cmdReleaseConfigReply ));
    registerCommand( CMD_SERVER_SHUTDOWN_REPLY, 
           eqNet::CommandFunc<Server>( this, &Server::_cmdShutdownReply ));

    EQINFO << "New server at " << (void*)this << endl;
}

Server::~Server()
{
    EQINFO << "Delete server at " << (void*)this << endl;
}


Config* Server::chooseConfig( const ConfigParams& parameters )
{
    if( !isConnected( ))
        return 0;

    if( parameters.renderClient.empty( ))
    {
        EQWARN << "No render client in ConfigParams specified" << endl;
        return 0;
    }

    ServerChooseConfigPacket packet;

    packet.requestID      = _requestHandler.registerRequest();
    string rendererInfo   = parameters.workDir + '#' + parameters.renderClient;
#ifdef WIN32 // replace dir delimeters since '\' is often used as escape char
    for( size_t i=0; i<rendererInfo.length(); ++i )
        if( rendererInfo[i] == '\\' )
            rendererInfo[i] = '/';
#endif
    send( packet, rendererInfo );

    void* ptr = 0;
    _requestHandler.waitRequest( packet.requestID, ptr );
    return static_cast<Config*>( ptr );
}

Config* Server::useConfig( const ConfigParams& parameters, 
                           const std::string& config )
{
    if( !isConnected( ))
        return 0;

    if( parameters.renderClient.empty( ))
    {
        EQWARN << "No render client in ConfigParams specified" << endl;
        return 0;
    }

    ServerUseConfigPacket packet;

    packet.requestID  = _requestHandler.registerRequest();
    string configInfo = parameters.workDir + '#' + parameters.renderClient;
#ifdef WIN32 // replace dir delimeters since '\' is often used as escape char
    for( size_t i=0; i<configInfo.length(); ++i )
        if( configInfo[i] == '\\' )
            configInfo[i] = '/';
#endif

    configInfo += '#' + config;
    send( packet, configInfo );

    void* ptr = 0;
    _requestHandler.waitRequest( packet.requestID, ptr );
    return static_cast<Config*>( ptr );
}

void Server::releaseConfig( Config* config )
{
    EQASSERT( isConnected( ));

    ServerReleaseConfigPacket packet;
    packet.requestID = _requestHandler.registerRequest( config );
    packet.configID  = config->getID();
    send( packet );

    _requestHandler.waitRequest( packet.requestID );
}

bool Server::shutdown()
{
    if( !isConnected( ))
        return false;

    ServerShutdownPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );

    bool result = false;
    _requestHandler.waitRequest( packet.requestID, result );
    return result;
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
    
    RefPtr<Node> localNode = command.getLocalNode();
    Config*      config    = Global::getNodeFactory()->createConfig();

    EQASSERT( localNode->getSession( packet->configID ) == 0 );
    config->_appNodeID = packet->appNodeID;
    localNode->addSession( config, command.getNode(), packet->configID,
                           packet->name );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdDestroyConfig( eqNet::Command& command )
{
    const ServerDestroyConfigPacket* packet = 
        command.getPacket<ServerDestroyConfigPacket>();
    EQINFO << "Handle destroy config " << packet << endl;
    
    RefPtr<Node>    localNode  = command.getLocalNode();
    eqNet::Session* session    = localNode->getSession( packet->configID );

    EQASSERTINFO( dynamic_cast<Config*>( session ), typeid(*session).name( ));
    Config* config = static_cast<Config*>( session );

    localNode->removeSession( config );
    delete config;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdChooseConfigReply( eqNet::Command& command )
{
    const ServerChooseConfigReplyPacket* packet = 
        command.getPacket<ServerChooseConfigReplyPacket>();
    EQINFO << "Handle choose config reply " << packet << endl;

    if( packet->configID == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( packet->requestID, (void*)0 );
        return eqNet::COMMAND_HANDLED;
    }

    Config*      config    = Global::getNodeFactory()->createConfig();
    RefPtr<Node> localNode = command.getLocalNode();
 
    EQASSERT( localNode->getSession( packet->configID ) == 0 );
    config->_appNodeID = localNode->getNodeID();
    localNode->addSession( config, command.getNode(), packet->configID, 
                           packet->sessionName);

    _requestHandler.serveRequest( packet->requestID, config );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdReleaseConfigReply( eqNet::Command& command )
{
    const ServerReleaseConfigReplyPacket* packet = 
        command.getPacket<ServerReleaseConfigReplyPacket>();

    RefPtr<Node> localNode = command.getLocalNode();
    Config*      config    = static_cast<Config*>
        ( _requestHandler.getRequestData( packet->requestID ));
    EQASSERT( config );
    
    localNode->removeSession( config );
    delete config;
    _requestHandler.serveRequest( packet->requestID );
    
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdShutdownReply( eqNet::Command& command )
{
    const ServerShutdownReplyPacket* packet = 
        command.getPacket<ServerShutdownReplyPacket>();

    _requestHandler.serveRequest( packet->requestID, packet->result );
    return eqNet::COMMAND_HANDLED;
}
