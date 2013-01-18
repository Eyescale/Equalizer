
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

namespace eq
{

typedef co::CommandFunc< Server > CmdFunc;
typedef fabric::Server< Client, Server, Config, NodeFactory, co::Node,
                        ServerVisitor > Super;

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

Config* Server::chooseConfig( const fabric::ConfigParams& parameters )
{
    if( !isConnected( ))
        return 0;

    if( parameters.getRenderClient().empty( ))
    {
        LBWARN << "No render client in ConfigParams specified" << std::endl;
        return 0;
    }

    ClientPtr client = getClient();
    const uint32_t requestID =  client->registerRequest();

    send( fabric::CMD_SERVER_CHOOSE_CONFIG )
        << requestID << parameters << eq::Global::getConfigFile();

    while( !client->isRequestServed( requestID ))
        getClient()->processCommand();

    void* ptr = 0;
    client->waitRequest( requestID, ptr );
    return static_cast<Config*>( ptr );
}

void Server::releaseConfig( Config* config )
{
    LBASSERT( isConnected( ));
    ClientPtr client = getClient();
    const uint32_t requestID = client->registerRequest();
    send( fabric::CMD_SERVER_RELEASE_CONFIG ) << config->getID() << requestID;

    while( !client->isRequestServed( requestID ))
        client->processCommand();

    client->waitRequest( requestID );
}

bool Server::shutdown()
{
    if( !isConnected( ))
        return false;

    ClientPtr client = getClient();
    const uint32_t requestID = client->registerRequest();
    send( fabric::CMD_SERVER_SHUTDOWN ) << requestID;

    while( !client->isRequestServed( requestID ))
        getClient()->processCommand();

    bool result = false;
    client->waitRequest( requestID, result );

    if( result )
        static_cast< co::LocalNode& >( *getClient( )).disconnect( this );

    return result;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Server::_cmdChooseConfigReply( co::ICommand& command )
{
    co::LocalNodePtr  localNode = command.getLocalNode();
    const UUID configID = command.get< UUID >();
    const uint32_t requestID = command.get< uint32_t >();

    LBVERB << "Handle choose config reply " << command << " req " << requestID
           << " id " << configID << std::endl;

    if( configID == 0 )
    {
        localNode->serveRequest( requestID, (void*)0 );
        return true;
    }

    const std::string connectionData = command.get< std::string >();
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

    LBUNIMPLEMENTED
    return true;
}

bool Server::_cmdReleaseConfigReply( co::ICommand& command )
{
    co::LocalNodePtr localNode = command.getLocalNode();
    localNode->serveRequest( command.get< uint32_t >( ));
    return true;
}

bool Server::_cmdShutdownReply( co::ICommand& command )
{
    co::LocalNodePtr localNode = command.getLocalNode();
    const uint32_t requestID = command.get< uint32_t >();
    const bool result = command.get< bool >();
    localNode->serveRequest( requestID, result );
    return true;
}
}

#include "../fabric/server.ipp"
template class eq::fabric::Server< eq::Client, eq::Server, eq::Config,
                                 eq::NodeFactory, co::Node, eq::ServerVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                             const eq::Super& );
/** @endcond */
