
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

#include "channel.h"
#include "config.h"
#include "window.h"

using namespace std;
using namespace eq::base;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig( eq::ServerPtr parent )
        { return new eqPixelBench::Config( parent ); }
    virtual eq::Window* createWindow( eq::Pipe* parent )
        { return new eqPixelBench::Window( parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new eqPixelBench::Channel( parent ); }
};

int main( int argc, char** argv )
{
    // 1. initialization of local node
    NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << endl;
        return EXIT_FAILURE;
    }

    RefPtr<eq::Client> client = new eq::Client;
    if( !client->initLocal( argc, argv ))
    {
        EQERROR << "Can't init client" << endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // 2. connect to server
    RefPtr<eq::Server> server = new eq::Server;
    if( !client->connectServer( server ))
    {
        EQERROR << "Can't open server" << endl;
        client->exitLocal();
        eq::exit();
        return EXIT_FAILURE;
    }

    // 3. choose config
    eq::ConfigParams configParams;
    eqPixelBench::Config* config = static_cast<eqPixelBench::Config*>(
        server->chooseConfig( configParams ));

    if( !config )
    {
        EQERROR << "No matching config on server" << endl;
        client->disconnectServer( server );
        client->exitLocal();
        eq::exit();
        return EXIT_FAILURE;
    }

    // 4. init config
    eq::base::Clock clock;

    if( !config->init( 0 ))
    {
        EQERROR << "Config initialization failed: " 
                << config->getErrorMessage() << endl;
        server->releaseConfig( config );
        client->disconnectServer( server );
        client->exitLocal();
        eq::exit();
        return EXIT_FAILURE;
    }

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
    if( !client->disconnectServer( server ))
        EQERROR << "Client::disconnectServer failed" << endl;
    server = 0;

    client->exitLocal();
    client = 0;

    eq::exit();
    return EXIT_SUCCESS;
}
