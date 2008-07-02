
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"
#include "config.h"

using namespace std;
using namespace eqBase;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig( eqBase::RefPtr< eq::Server > parent )
        { return new eqPixelBench::Config( parent ); }
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
    eqBase::Clock clock;

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
    config->startFrame( 0 );
    config->finishAllFrames();
    config->startFrame( 0 );
    config->finishAllFrames();
    config->startFrame( 0 );
    config->finishAllFrames();
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
