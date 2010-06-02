
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <test.h>
#include "channel.h"
#include "config.h"
#include "window.h"

using namespace std;
using namespace eq::base;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig( eq::ServerPtr parent )
        { return new eqTransfertDSO::Config( parent ); }
    virtual eq::Window* createWindow( eq::Pipe* parent )
        { return new eqTransfertDSO::Window( parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new eqTransfertDSO::Channel( parent ); }
};

int main( int argc, char** argv )
{
    // 1. initialization of local node
    NodeFactory nodeFactory;
    TESTINFO( eq::init( argc, argv, &nodeFactory ), "Equalizer init failed" );
    
    RefPtr<eq::Client> client = new eq::Client;
    TESTINFO( client->initLocal( argc, argv ), "Can't init client" );
    
   
    // 2. connect to server
    RefPtr<eq::Server> server = new eq::Server;
    TESTINFO( client->connectServer( server ), "Can't open server" );
    
    // 3. choose config
    eq::ConfigParams configParams;
    eqTransfertDSO::Config* config = static_cast<eqTransfertDSO::Config*>(
        server->chooseConfig( configParams ));

    TESTINFO( config, "No matching config on server" )
    
    // 4. init config
    eq::base::Clock clock;

    TESTINFO( config->init( 0 ), "Config initialization failed: " 
                                 << config->getErrorMessage() );
    

    EQLOG( eq::LOG_CUSTOM ) << "Config init took " << clock.getTimef() << " ms"
                            << endl;

    // 5. render three frames
    clock.reset();
    for( uint32_t i = 0; i < 3; ++i )
    {
        config->startFrame( 0 );
        config->finishAllFrames();
    }
    EQLOG( eq::LOG_CUSTOM ) << "Rendering took " << clock.getTimef() << " ms ("
                            << ( 1.0f / clock.getTimef() * 1000.f) << " FPS)"
                            << endl;

    // 6. exit config
    clock.reset();
    config->exit();
    EQLOG( eq::LOG_CUSTOM ) << "Exit took " << clock.getTimef() << " ms" <<endl;

    // 7. cleanup and exit
    server->releaseConfig( config );
    TESTINFO( client->disconnectServer( server ), 
              "Client::disconnectServer failed" );
    server = 0;

    client->exitLocal();
    client = 0;

    eq::exit();
    return EXIT_SUCCESS;
}
