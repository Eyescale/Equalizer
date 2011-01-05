
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
#include "serverPackets.h"
#include "types.h"

#include <co/command.h>
#include <co/connection.h>

namespace eq
{

typedef co::CommandFunc< Server > CmdFunc;
typedef fabric::Server< Client, Server, Config, NodeFactory, co::Node > Super;

Server::Server()
        : Super( Global::getNodeFactory( ))
        , _localServer( false )
{
}

Server::~Server()
{
}

void Server::setClient( ClientPtr client )
{
    Super::setClient( client );
    if( !client )
        return;

    co::CommandQueue* queue = client->getMainThreadQueue();
    registerCommand( fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY, 
                     CmdFunc( this, &Server::_cmdChooseConfigReply ), queue );
    registerCommand( fabric::CMD_SERVER_RELEASE_CONFIG_REPLY, 
                     CmdFunc( this, &Server::_cmdReleaseConfigReply ), queue );
    registerCommand( fabric::CMD_SERVER_SHUTDOWN_REPLY, 
                     CmdFunc( this, &Server::_cmdShutdownReply ), queue );
}

co::CommandQueue* Server::getMainThreadQueue()
{
    return getClient()->getMainThreadQueue();
}

co::CommandQueue* Server::getCommandThreadQueue() 
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
    ClientPtr client = getClient();
    packet.requestID =  client->registerRequest();

    const std::string& workDir = parameters.getWorkDir();
    std::string rendererInfo = workDir + '#' + renderClient;
#ifdef _WIN32 // replace dir delimiters since '\' is often used as escape char
    for( size_t i=0; i<rendererInfo.length(); ++i )
        if( rendererInfo[i] == '\\' )
            rendererInfo[i] = '/';
#endif

    send( packet, rendererInfo );

    while( !client->isRequestServed( packet.requestID ))
        getClient()->processCommand();

    void* ptr = 0;
    client->waitRequest( packet.requestID, ptr );
    return static_cast<Config*>( ptr );
}

void Server::releaseConfig( Config* config )
{
    EQASSERT( isConnected( ));
    ClientPtr client = getClient();
    ServerReleaseConfigPacket packet;
    packet.requestID = client->registerRequest();
    packet.configID  = config->getID();
    send( packet );

    while( !client->isRequestServed( packet.requestID ))
        client->processCommand();

    client->waitRequest( packet.requestID );
}

bool Server::shutdown()
{
    if( !isConnected( ))
        return false;

    ClientPtr client = getClient();
    ServerShutdownPacket packet;
    packet.requestID = client->registerRequest();
    send( packet );

    while( !client->isRequestServed( packet.requestID ))
        getClient()->processCommand();

    bool result = false;
    client->waitRequest( packet.requestID, result );

    if( result )
        static_cast< co::LocalNode* >( getClient().get( ))->disconnect( this );

    return result;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Server::_cmdChooseConfigReply( co::Command& command )
{
    const ServerChooseConfigReplyPacket* packet = 
        command.getPacket<ServerChooseConfigReplyPacket>();
    EQVERB << "Handle choose config reply " << packet << std::endl;

    co::LocalNodePtr  localNode = command.getLocalNode();
    if( packet->configID == co::base::UUID::ZERO )
    {
        localNode->serveRequest( packet->requestID, (void*)0 );
        return true;
    }

    const Configs& configs = getConfigs();
   
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        Config* config = *i;
        if( config->getID() ==  packet->configID )
        {
            config->setupServerConnections( packet->connectionData );
            localNode->serveRequest( packet->requestID, config );
            return true;
        }
    }

    EQUNIMPLEMENTED
    return true;
}

bool Server::_cmdReleaseConfigReply( co::Command& command )
{
    const ServerReleaseConfigReplyPacket* packet = 
        command.getPacket<ServerReleaseConfigReplyPacket>();

    co::LocalNodePtr localNode = command.getLocalNode();
    localNode->serveRequest( packet->requestID );
    return true;
}

bool Server::_cmdShutdownReply( co::Command& command )
{
    const ServerShutdownReplyPacket* packet = 
        command.getPacket<ServerShutdownReplyPacket>();
    EQINFO << "Handle shutdown reply " << packet << std::endl;

    co::LocalNodePtr  localNode = command.getLocalNode();
    localNode->serveRequest( packet->requestID, packet->result );
    return true;
}
}

#include "../fabric/server.ipp"
template class eq::fabric::Server< eq::Client, eq::Server, eq::Config,
                                   eq::NodeFactory, co::Node >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                             const eq::Super& );
/** @endcond */
