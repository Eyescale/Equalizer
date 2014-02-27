
/* Copyright (c) 2010-2014, Stefan Eilemann <eile@eyescale.ch>
 *               2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include "nodeFactory.h"

#include <eq/fabric/commands.h>
#include <co/dispatcher.h>
#include <co/iCommand.h>

#include <boost/foreach.hpp>

namespace eq
{
namespace admin
{
namespace
{
    static NodeFactory _nf;
}

typedef co::CommandFunc< Server > CmdFunc;
typedef fabric::Server< Client, Server, Config, NodeFactory, co::Node,
                        ServerVisitor > Super;

Server::Server()
        : Super( &_nf )
{}

void Server::setClient( ClientPtr client )
{
    Super::setClient( client );
    if( !client )
        return;

    co::CommandQueue* queue = client->getMainThreadQueue();
    registerCommand( fabric::CMD_SERVER_MAP_REPLY,
                     CmdFunc( this, &Server::_cmdMapReply ), queue );
    registerCommand( fabric::CMD_SERVER_UNMAP_REPLY,
                     CmdFunc( this, &Server::_cmdUnmapReply ), queue );
}

void Server::map()
{
    ClientPtr client = getClient();

    const uint32_t requestID = client->registerRequest();
    send( fabric::CMD_SERVER_MAP ) << requestID;

    while( !client->isRequestServed( requestID ))
        client->processCommand();
    client->waitRequest( requestID );
}

void Server::unmap()
{
    ClientPtr client = getClient();

    const uint32_t requestID = client->registerRequest();
    send( fabric::CMD_SERVER_UNMAP ) << requestID;

    while( !client->isRequestServed( requestID ))
        client->processCommand();
    client->waitRequest( requestID );
}

void Server::syncConfig( const co::uint128_t& configID,
                         const co::uint128_t& version  )
{
    const Configs& configs = getConfigs();
    BOOST_FOREACH( Config* config, configs )
    {
        if( config->getID() == configID )
            config->sync( version );
    }
}

co::CommandQueue* Server::getMainThreadQueue()
{
    return getClient()->getMainThreadQueue();
}

bool Server::_cmdMapReply( co::ICommand& command )
{
    ClientPtr client = getClient();
    client->serveRequest( command.get< uint32_t >( ));
    return true;
}

bool Server::_cmdUnmapReply( co::ICommand& command )
{
    ClientPtr client = getClient();
    client->serveRequest( command.get< uint32_t >( ));
    return true;
}

}
}
#include "../fabric/server.ipp"
template class eq::fabric::Server< eq::admin::Client, eq::admin::Server,
                                   eq::admin::Config, eq::admin::NodeFactory,
                                   co::Node, eq::admin::ServerVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                       const eq::admin::Super& );
/** @endcond */
