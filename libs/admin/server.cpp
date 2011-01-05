
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#include <eq/admin/packets.h>
#include <co/command.h>
#include <co/dispatcher.h>

namespace eq
{
namespace admin
{
namespace
{
    static NodeFactory _nf;
}

typedef co::CommandFunc< Server > CmdFunc;
typedef fabric::Server< Client, Server, Config, NodeFactory, co::Node > Super;

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
    ServerMapPacket packet;
    ClientPtr client = getClient();

    packet.requestID = client->registerRequest();
    send( packet );

    while( !client->isRequestServed( packet.requestID ))
        client->processCommand();
    client->waitRequest( packet.requestID );
}

void Server::unmap()
{
    ServerUnmapPacket packet;
    ClientPtr client = getClient();
    
    packet.requestID = client->registerRequest();
    send( packet );

    while( !client->isRequestServed( packet.requestID ))
        client->processCommand();
    client->waitRequest( packet.requestID );
}

co::CommandQueue* Server::getMainThreadQueue()
{
    return getClient()->getMainThreadQueue();
}

bool Server::_cmdMapReply( co::Command& command )
{
    const ServerMapReplyPacket* packet = 
        command.getPacket< ServerMapReplyPacket >();
    ClientPtr client = getClient();
    client->serveRequest( packet->requestID );
    return true;
}

bool Server::_cmdUnmapReply( co::Command& command )
{
    const ServerUnmapReplyPacket* packet = 
        command.getPacket< ServerUnmapReplyPacket >();

    ClientPtr client = getClient();
    client->serveRequest( packet->requestID );
    return true;
}

}
}
#include "../fabric/server.ipp"
template class eq::fabric::Server< eq::admin::Client, eq::admin::Server,
                                   eq::admin::Config, eq::admin::NodeFactory,
                                   co::Node >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                       const eq::admin::Super& );
/** @endcond */
