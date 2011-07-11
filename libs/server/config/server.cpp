
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.h> 
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

#include "display.h"
#include "resources.h"
#include "../config.h"
#include "../server.h"

namespace eq
{
namespace server
{
namespace config
{

ServerPtr Server::configureLocal()
{
    ServerPtr server = new server::Server;
    Config* config = new Config( server );
    config->setName( "Local Auto-Config" );

    Resources::discoverLocal( config );
    Display::discoverLocal( config );
    return server;
}

}
}
}
