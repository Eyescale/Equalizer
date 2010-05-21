
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

typedef net::CommandFunc< Server > CmdFunc;
typedef fabric::Server< Client, Server, Config, NodeFactory > Super;

Server::Server()
        : Super( Global::getNodeFactory( ))
        , _localServer( false )
{
    EQINFO << "New server at " << (void*)this << std::endl;
}

Server::~Server()
{
    EQINFO << "Delete server at " << (void*)this << std::endl;
}

void Server::setClient( ClientPtr client )
{
    Super::setClient( client );
    if( !client )
        return;

    net::CommandQueue* queue = client->getMainThreadQueue();
    registerCommand( fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY, 
                     CmdFunc( this, &Server::_cmdChooseConfigReply ), queue );
    registerCommand( fabric::CMD_SERVER_RELEASE_CONFIG_REPLY, 
                     CmdFunc( this, &Server::_cmdReleaseConfigReply ), queue );
    registerCommand( fabric::CMD_SERVER_SHUTDOWN_REPLY, 
                     CmdFunc( this, &Server::_cmdShutdownReply ), queue );
}

net::CommandQueue* Server::getMainThreadQueue()
{
    return getClient()->getMainThreadQueue();
}

net::CommandQueue* Server::getCommandThreadQueue() 
{
    return getClient()->getCommandThreadQueue();
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
        getClient()->processCommand();

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
        getClient()->processCommand();

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
        getClient()->processCommand();

    bool result = false;
    waitRequest( packet.requestID, result );

    if( result )
        static_cast< net::Node* >( getClient().get( ))->close( this );

    return result;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
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

#include "../fabric/server.ipp"
template class eq::fabric::Server< eq::Client, eq::Server, eq::Config,
                                   eq::NodeFactory >;

