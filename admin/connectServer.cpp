
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

#include "connectServer.h"

#include "server.h"
#include <eq/fabric/client.h>

namespace eq
{
namespace admin
{

bool connectServer( eq::fabric::ClientPtr client, ServerPtr server )
{
    if( !client->connectServer( server.get( )))
        return false;

    server->setClient( client );
    server->map();
    EQINFO << "Connected " << server << std::endl;
    return true;
}

bool disconnectServer( eq::fabric::ClientPtr client, ServerPtr server )
{
    server->unmap();
    server->setClient( 0 );
    return client->disconnectServer( server.get( ));
}

}
}

