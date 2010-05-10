
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

#include <eq/admin/admin.h>

void _runMainLoop( eq::admin::ServerPtr server );

int main( const int argc, char** argv )
{
    // 1. Equalizer admin initialization
    if( !eq::admin::init( argc, argv ))
    {
        EQERROR << "Initialization of Equalizer administrative library failed"
                << std::endl;
        return EXIT_FAILURE;
    }
    
    // 2. initialization of local client node
    eq::admin::ClientPtr client = new eq::admin::Client;
    if( !client->initLocal( argc, argv ))
    {
        EQERROR << "Can't init client" << std::endl;
        eq::admin::exit();
        return EXIT_FAILURE;
    }

    // 3. connect to server
    eq::admin::ServerPtr server = new eq::admin::Server;
    if( !client->connectServer( server ))
    {
        EQERROR << "Can't open server" << std::endl;
        eq::admin::exit();
        return EXIT_FAILURE;
    }

    _runMainLoop( server );

    // 4. cleanup and exit
    if( !client->disconnectServer( server ))
        EQERROR << "Client::disconnectServer failed" << std::endl;

    client->exitLocal();

    // TODO EQASSERTINFO( client->getRefCount() == 1, client->getRefCount( ));
    client = 0;
    eq::admin::exit();
    return EXIT_SUCCESS;
}

void _runMainLoop( eq::admin::ServerPtr server )
{
    // Find first pipe...
    const eq::admin::ConfigVector& configs = server->getConfigs();
    if( configs.empty( ))
    {
        std::cout << "No configs on server, exiting" << std::endl;
        return;
    }

    const eq::admin::Config* config = configs.front();
    const eq::admin::NodeVector& nodes = config->getNodes();
    if( nodes.empty( ))
    {
        std::cout << "No nodes in config, exiting" << std::endl;
        return;
    }
 
    const eq::admin::Node* node = nodes.front();
    const eq::admin::PipeVector& pipes = node->getPipes();
    if( pipes.empty( ))
    {
        std::cout << "No pipes in node, exiting" << std::endl;
        return;
    }

    eq::admin::Pipe* pipe = pipes.front();
    EQASSERT( pipe );
    //std::cout << "Using " << *pipe << std::endl;
}
