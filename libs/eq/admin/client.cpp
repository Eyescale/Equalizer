
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

#include "client.h"

#include "server.h"
#include <co/command.h>

namespace eq
{
namespace admin
{

/** @cond IGNORE */
typedef fabric::Client Super;
/** @endcond */

Client::Client()
        : Super()
{
}

Client::~Client()
{
    close();
}

bool Client::connectServer( ServerPtr server )
{
    if( !Super::connectServer( server.get( )))
        return false;

    server->setClient( this );
    server->map();
    return true;
}

bool Client::disconnectServer( ServerPtr server )
{
    server->unmap();
    server->setClient( 0 );
    return Super::disconnectServer( server.get( ));
}

co::NodePtr Client::createNode( const uint32_t type )
{ 
    switch( type )
    {
        case fabric::NODETYPE_EQ_SERVER:
        {
            Server* server = new Server;
            server->setClient( this );
            return server;
        }

        default:
            return co::Node::createNode( type );
    }
}

}
}

