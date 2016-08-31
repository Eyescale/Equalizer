
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include "global.h"
#include "nodeFactory.h"
#include "types.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/configParams.h>

#include <co/iCommand.h>
#include <co/connection.h>

#include <algorithm>

#pragma clang diagnostic ignored "-Wunused-private-field" // _impl is unused

namespace eq
{
typedef co::CommandFunc< Server > CmdFunc;
typedef fabric::Server< Client, Server, Config, NodeFactory, co::Node,
                        ServerVisitor > Super;

Server::Server()
    : Super( Global::getNodeFactory( ))
    , _impl( 0 )
{}

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

Config* Server::chooseConfig( const fabric::ConfigParams& p )
{
    if( !isConnected( ))
        return 0;

    ClientPtr client = getClient();
    fabric::ConfigParams params( p );

    if( params.getName().empty( ))
        params.setName( client->getName( ));
    if( params.getWorkDir().empty( ))
        params.setWorkDir( Global::getWorkDir( ));
    if( params.getRenderClient().empty( ))
        params.setRenderClient( Global::getProgramName( ));
    if( params.getRenderClientArgs().empty( ))
        params.setRenderClientArgs( client->getCommandLine( ));
    if( params.getGPUFilter().empty( ))
        params.setGPUFilter( client->getGPUFilter( ));

    if( params.getRenderClient().empty( ))
        LBWARN << "No render client in ConfigParams specified" << std::endl;

    lunchbox::Request< void* > request = client->registerRequest< void* >();
    send( fabric::CMD_SERVER_CHOOSE_CONFIG )
        << request << params << eq::Global::getConfig();

    while( !request.isReady( ))
        getClient()->processCommand();

    return static_cast< Config* >( request.wait( ));
}

void Server::releaseConfig( Config* config )
{
    LBASSERT( isConnected( ));
    ClientPtr client = getClient();

    lunchbox::Request< void > request = client->registerRequest< void >();
    send( fabric::CMD_SERVER_RELEASE_CONFIG ) << config->getID() << request;

    while( !request.isReady( ))
        client->processCommand();
}

bool Server::shutdown()
{
    if( !isConnected( ))
        return false;

    ClientPtr client = getClient();
    lunchbox::Request< bool > request = client->registerRequest< bool >();
    send( fabric::CMD_SERVER_SHUTDOWN ) << request;

    while( !request.isReady( ))
        getClient()->processCommand();

    if( !request.wait( ))
        return false;

    static_cast< co::LocalNode& >( *getClient( )).disconnect( this );
    return true;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Server::_cmdChooseConfigReply( co::ICommand& command )
{
    co::LocalNodePtr  localNode = command.getLocalNode();
    const uint128_t& configID = command.read< uint128_t >();
    const uint32_t requestID = command.read< uint32_t >();

    LBVERB << "Handle choose config reply " << command << " req " << requestID
           << " id " << configID << std::endl;

    if( configID == 0 )
    {
        localNode->serveRequest( requestID, (void*)0 );
        return true;
    }

    const std::string& connectionData = command.read< std::string >();
    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        Config* config = *i;
        if( config->getID() == configID )
        {
            config->setupServerConnections( connectionData );
            localNode->serveRequest( requestID, config );
            return true;
        }
    }

    LBUNREACHABLE;
    localNode->serveRequest( requestID, (void*)0 );
    return true;
}

bool Server::_cmdReleaseConfigReply( co::ICommand& command )
{
    co::LocalNodePtr localNode = command.getLocalNode();
    localNode->serveRequest( command.read< uint32_t >( ));
    return true;
}

bool Server::_cmdShutdownReply( co::ICommand& command )
{
    co::LocalNodePtr localNode = command.getLocalNode();
    const uint32_t requestID = command.read< uint32_t >();
    const bool result = command.read< bool >();
    localNode->serveRequest( requestID, result );
    return true;
}
}

#include <eq/fabric/server.ipp>
template class eq::fabric::Server< eq::Client, eq::Server, eq::Config,
                                 eq::NodeFactory, co::Node, eq::ServerVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                             const eq::Super& );
/** @endcond */
