
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

namespace eq
{

typedef net::CommandFunc<Server> CmdFunc;

Server::Server()
        : _localServer( false )
{
    EQINFO << "New server at " << (void*)this << std::endl;
}

Server::~Server()
{
    EQINFO << "Delete server at " << (void*)this << std::endl;
    _client = 0;
}

void Server::setClient( ClientPtr client )
{
    _client = client;
    if( !client )
        return;

    net::CommandQueue* queue = client->getNodeThreadQueue();

    registerCommand( fabric::CMD_SERVER_CREATE_CONFIG, 
                     CmdFunc( this, &Server::_cmdCreateConfig ), queue );
    registerCommand( fabric::CMD_SERVER_DESTROY_CONFIG, 
                     CmdFunc( this, &Server::_cmdDestroyConfig ), queue );
    registerCommand( fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY, 
                     CmdFunc( this, &Server::_cmdChooseConfigReply ), queue );
    registerCommand( fabric::CMD_SERVER_RELEASE_CONFIG_REPLY, 
                     CmdFunc( this, &Server::_cmdReleaseConfigReply ), queue );
    registerCommand( fabric::CMD_SERVER_SHUTDOWN_REPLY, 
                     CmdFunc( this, &Server::_cmdShutdownReply ), queue );
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

    const std::string& renderClient = parameters.getRenderClient();
    if( renderClient.empty( ))
    {
        EQWARN << "No render client in ConfigParams specified" << std::endl;
        return 0;
    }

    ServerChooseConfigPacket packet;
    const std::string& workDir = parameters.getWorkDir();

    packet.requestID         = registerRequest();
    std::string rendererInfo = workDir + '#' + renderClient;
#ifdef WIN32 // replace dir delimiters since '\' is often used as escape char
    for( size_t i=0; i<rendererInfo.length(); ++i )
        if( rendererInfo[i] == '\\' )
            rendererInfo[i] = '/';
#endif
    send( packet, rendererInfo );

    while( !isRequestServed( packet.requestID ))
        _client->processCommand();

    void* ptr = 0;
    waitRequest( packet.requestID, ptr );
    return static_cast<Config*>( ptr );
}

void Server::releaseConfig( Config* config )
{
    EQASSERT( isConnected( ));

    ServerReleaseConfigPacket packet;
    packet.requestID = registerRequest();
    packet.configID  = config->getID();
    send( packet );

    while( !isRequestServed( packet.requestID ))
        _client->processCommand();

    waitRequest( packet.requestID );
}

bool Server::shutdown()
{
    if( !isConnected( ))
        return false;

    ServerShutdownPacket packet;
    packet.requestID = registerRequest();
    send( packet );

    while( !isRequestServed( packet.requestID ))
        _client->processCommand();

    bool result = false;
    waitRequest( packet.requestID, result );

    if( result )
        static_cast< net::Node* >( _client.get( ))->close( this );

    return result;
}

void Server::_addConfig( Config* config )
{ 
    EQASSERT( config->getServer() == this );
    _configs.push_back( config );
}

bool Server::_removeConfig( Config* config )
{
    ConfigVector::iterator i = find( _configs.begin(), _configs.end(),
                                      config );
    if( i == _configs.end( ))
        return false;

    _configs.erase( i );
    return true;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Server::_cmdCreateConfig( net::Command& command )
{
    const ServerCreateConfigPacket* packet = 
        command.getPacket<ServerCreateConfigPacket>();
    EQVERB << "Handle create config " << packet << std::endl;
    EQASSERT( packet->proxyID != EQ_ID_INVALID );
    
    net::NodePtr localNode = command.getLocalNode();
    Config* config = Global::getNodeFactory()->createConfig( this );

    config->_appNodeID = packet->appNodeID;
    localNode->mapSession( command.getNode(), config, packet->configID );
    config->map( packet->proxyID );

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
    EQVERB << "Handle destroy config " << packet << std::endl;
    
    net::NodePtr  localNode  = command.getLocalNode();
    net::Session* session    = localNode->getSession( packet->configID );

    EQASSERTINFO( dynamic_cast<Config*>( session ), typeid(*session).name( ));
    Config* config = static_cast<Config*>( session );

    config->unmap();
    config->_exitMessagePump();
    EQCHECK( localNode->unmapSession( config ));
    Global::getNodeFactory()->releaseConfig( config );

    if( packet->requestID != EQ_ID_INVALID )
    {
        ServerDestroyConfigReplyPacket reply( packet );
        command.getNode()->send( reply );
    }
    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdChooseConfigReply( net::Command& command )
{
    const ServerChooseConfigReplyPacket* packet = 
        command.getPacket<ServerChooseConfigReplyPacket>();
    EQVERB << "Handle choose config reply " << packet << std::endl;

    if( packet->configID == net::SessionID::ZERO )
    {
        serveRequest( packet->requestID, (void*)0 );
        return net::COMMAND_HANDLED;
    }

    net::NodePtr  localNode = command.getLocalNode();
    net::Session* session   = localNode->getSession( packet->configID );
    Config*       config    = static_cast< Config* >( session );
    EQASSERTINFO( dynamic_cast< Config* >( session ), 
                  "Session id " << packet->configID << " @" << (void*)session );

    serveRequest( packet->requestID, config );
    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdReleaseConfigReply( net::Command& command )
{
    const ServerReleaseConfigReplyPacket* packet = 
        command.getPacket<ServerReleaseConfigReplyPacket>();

    serveRequest( packet->requestID );
    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdShutdownReply( net::Command& command )
{
    const ServerShutdownReplyPacket* packet = 
        command.getPacket<ServerShutdownReplyPacket>();
    EQINFO << "Handle shutdown reply " << packet << std::endl;

    serveRequest( packet->requestID, packet->result );
    return net::COMMAND_HANDLED;
}
}
