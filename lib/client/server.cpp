
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "server.h"

#include "client.h"
#include "config.h"
#include "configParams.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "packets.h"
#include "types.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eq::base;
using namespace std;
using eq::net::CommandFunc;
using eq::net::NodePtr;

namespace eq
{
Server::Server()
        : _localServer( false )
{
    EQINFO << "New server at " << (void*)this << endl;
}

Server::~Server()
{
    EQINFO << "Delete server at " << (void*)this << endl;
    _client = 0;
}

void Server::setClient( ClientPtr client )
{
    _client = client;
    if( !client )
        return;

    net::CommandQueue* queue = client->getNodeThreadQueue();

    registerCommand( CMD_SERVER_CREATE_CONFIG, 
                     CommandFunc<Server>( this, &Server::_cmdCreateConfig ),
                     queue );
    registerCommand( CMD_SERVER_DESTROY_CONFIG, 
                     CommandFunc<Server>( this, &Server::_cmdDestroyConfig ),
                     queue );
    registerCommand( CMD_SERVER_CHOOSE_CONFIG_REPLY, 
                    CommandFunc<Server>( this, &Server::_cmdChooseConfigReply ),
                     queue );
    registerCommand( CMD_SERVER_RELEASE_CONFIG_REPLY, 
                   CommandFunc<Server>( this, &Server::_cmdReleaseConfigReply ),
                     queue );
    registerCommand( CMD_SERVER_SHUTDOWN_REPLY, 
                     CommandFunc<Server>( this, &Server::_cmdShutdownReply ),
                     queue );
}

net::CommandQueue* Server::getNodeThreadQueue() 
{
    return _client->getNodeThreadQueue();
}

net::CommandQueue* Server::getCommandThreadQueue() 
{
    return _client->getCommandThreadQueue();
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

    while( !_requestHandler.isServed( packet.requestID ))
        _client->processCommand();

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

    while( !_requestHandler.isServed( packet.requestID ))
        _client->processCommand();

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

    while( !_requestHandler.isServed( packet.requestID ))
        _client->processCommand();

    _requestHandler.waitRequest( packet.requestID );
}

bool Server::shutdown()
{
    if( !isConnected( ))
        return false;

    ServerShutdownPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );

    while( !_requestHandler.isServed( packet.requestID ))
        _client->processCommand();

    bool result = false;
    _requestHandler.waitRequest( packet.requestID, result );

    if( result )
        _client->disconnect( this );

    return result;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Server::_cmdCreateConfig( net::Command& command )
{
    const ServerCreateConfigPacket* packet = 
        command.getPacket<ServerCreateConfigPacket>();
    EQINFO << "Handle create config " << packet << ", name " << packet->name 
           << endl;
    
    NodePtr localNode = command.getLocalNode();
    Config* config    = Global::getNodeFactory()->createConfig( this );

    config->_name      = packet->name;
    config->_appNodeID = packet->appNodeID;
    config->_appNodeID.convertToHost();

    localNode->mapSession( command.getNode(), config, packet->configID );

    if( packet->objectID != EQ_ID_INVALID )
        config->_initAppNode( packet->objectID );

    if( packet->requestID != EQ_ID_INVALID )
    {
        ConfigCreateReplyPacket reply( packet );
        command.getNode()->send( reply );
    }

    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdDestroyConfig( net::Command& command )
{
    const ServerDestroyConfigPacket* packet = 
        command.getPacket<ServerDestroyConfigPacket>();
    EQINFO << "Handle destroy config " << packet << endl;
    
    NodePtr       localNode  = command.getLocalNode();
    net::Session* session    = localNode->getSession( packet->configID );

    EQASSERTINFO( dynamic_cast<Config*>( session ), typeid(*session).name( ));
    Config* config = static_cast<Config*>( session );

    EQCHECK( localNode->unmapSession( config ));
    Global::getNodeFactory()->releaseConfig( config );

    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdChooseConfigReply( net::Command& command )
{
    const ServerChooseConfigReplyPacket* packet = 
        command.getPacket<ServerChooseConfigReplyPacket>();
    EQINFO << "Handle choose config reply " << packet << endl;

    if( packet->configID == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( packet->requestID, (void*)0 );
        return net::COMMAND_HANDLED;
    }

    NodePtr       localNode = command.getLocalNode();
    net::Session* session   = localNode->getSession( packet->configID );
    Config*       config    = static_cast< Config* >( session );
    EQASSERTINFO( dynamic_cast< Config* >( session ), 
                  "Session id " << packet->configID << " @" << (void*)session );

    _requestHandler.serveRequest( packet->requestID, config );
    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdReleaseConfigReply( net::Command& command )
{
    const ServerReleaseConfigReplyPacket* packet = 
        command.getPacket<ServerReleaseConfigReplyPacket>();

    _requestHandler.serveRequest( packet->requestID );
    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdShutdownReply( net::Command& command )
{
    const ServerShutdownReplyPacket* packet = 
        command.getPacket<ServerShutdownReplyPacket>();
    EQINFO << "Handle shutdown reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, packet->result );
    return net::COMMAND_HANDLED;
}
}
