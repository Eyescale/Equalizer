
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

using namespace eqBase;
using namespace std;

namespace eq
{
Server::Server()
{
    registerCommand( CMD_SERVER_CREATE_CONFIG, 
                     eqNet::CommandFunc<Server>( this, &Server::_cmdPush ));
    registerCommand( REQ_SERVER_CREATE_CONFIG, 
                 eqNet::CommandFunc<Server>( this, &Server::_reqCreateConfig ));
    registerCommand( CMD_SERVER_DESTROY_CONFIG, 
                     eqNet::CommandFunc<Server>( this, &Server::_cmdPush ));
    registerCommand( REQ_SERVER_DESTROY_CONFIG, 
                eqNet::CommandFunc<Server>( this, &Server::_reqDestroyConfig ));
    registerCommand( CMD_SERVER_CHOOSE_CONFIG_REPLY, 
                     eqNet::CommandFunc<Server>( this, &Server::_cmdPush ));
    registerCommand( REQ_SERVER_CHOOSE_CONFIG_REPLY, 
            eqNet::CommandFunc<Server>( this, &Server::_reqChooseConfigReply ));
    registerCommand( CMD_SERVER_RELEASE_CONFIG_REPLY, 
                     eqNet::CommandFunc<Server>( this, &Server::_cmdPush ));
    registerCommand( REQ_SERVER_RELEASE_CONFIG_REPLY, 
           eqNet::CommandFunc<Server>( this, &Server::_reqReleaseConfigReply ));
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

    const string& renderClient = parameters.getRenderClient();
    if( renderClient.empty( ))
    {
        EQWARN << "No render client in ConfigParams specified" << endl;
        return 0;
    }

    ServerChooseConfigPacket packet;
    const string& workDir = parameters.getWorkDir();

    packet.requestID      = _requestHandler.registerRequest();
    string rendererInfo   = workDir + '#' + renderClient;
#ifdef WIN32 // replace dir delimiters since '\' is often used as escape char
    for( size_t i=0; i<rendererInfo.length(); ++i )
        if( rendererInfo[i] == '\\' )
            rendererInfo[i] = '/';
#endif
    send( packet, rendererInfo );

    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();

    void* ptr = 0;
    _requestHandler.waitRequest( packet.requestID, ptr );
    return static_cast<Config*>( ptr );
}

Config* Server::useConfig( const ConfigParams& parameters, 
                           const std::string& config )
{
    if( !isConnected( ))
        return 0;

    const string& renderClient = parameters.getRenderClient();
    if( renderClient.empty( ))
    {
        EQWARN << "No render client in ConfigParams specified" << endl;
        return 0;
    }

    ServerUseConfigPacket packet;
    const string& workDir = parameters.getWorkDir();

    packet.requestID  = _requestHandler.registerRequest();
    string configInfo = workDir + '#' + renderClient;
#ifdef WIN32 // replace dir delimiters since '\' is often used as escape char
    for( size_t i=0; i<configInfo.length(); ++i )
        if( configInfo[i] == '\\' )
            configInfo[i] = '/';
#endif

    configInfo += '#' + config;
    send( packet, configInfo );

    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();

    void* ptr = 0;
    _requestHandler.waitRequest( packet.requestID, ptr );
    return static_cast<Config*>( ptr );
}

void Server::releaseConfig( Config* config )
{
    EQASSERT( isConnected( ));

    ServerReleaseConfigPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.configID  = config->getID();
    send( packet );

    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();

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
eqNet::CommandResult Server::_reqCreateConfig( eqNet::Command& command )
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

    if( packet->requestID != EQ_ID_INVALID )
    {
        ConfigCreateReplyPacket reply( packet );
        command.getNode()->send( reply );
    }

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_reqDestroyConfig( eqNet::Command& command )
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

eqNet::CommandResult Server::_reqChooseConfigReply( eqNet::Command& command )
{
    const ServerChooseConfigReplyPacket* packet = 
        command.getPacket<ServerChooseConfigReplyPacket>();
    EQINFO << "Handle choose config reply " << packet << endl;

    if( packet->configID == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( packet->requestID, (void*)0 );
        return eqNet::COMMAND_HANDLED;
    }

    RefPtr<Node>    localNode = command.getLocalNode();
    eqNet::Session* session   = localNode->getSession( packet->configID );
    Config*         config    = static_cast< Config* >( session );
    EQASSERT( dynamic_cast< Config* >( session ));

    _requestHandler.serveRequest( packet->requestID, config );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_reqReleaseConfigReply( eqNet::Command& command )
{
    const ServerReleaseConfigReplyPacket* packet = 
        command.getPacket<ServerReleaseConfigReplyPacket>();

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
}
