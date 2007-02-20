
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eqPly.h"

#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"

#include <stdlib.h>

using namespace std;
using namespace eqBase;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig()  { return new ::Config; }
    virtual eq::Node*    createNode()    { return new ::Node; }
    virtual eq::Pipe*    createPipe()    { return new ::Pipe; }
    virtual eq::Channel* createChannel() { return new ::Channel; }
};

int main( int argc, char** argv )
{
    // 1. initialisation of local node
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
    Config*          config = (Config*)server->chooseConfig( configParams );

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

    config->parseArguments( argc, argv );
    
    if( !config->init( ))
    {
        EQERROR << "Config initialisation failed: " 
                << config->getErrorMessage() << endl;
        server->releaseConfig( config );
        client->disconnectServer( server );
        client->exitLocal();
        eq::exit();
        return EXIT_FAILURE;
    }

    EQLOG( eq::LOG_CUSTOM ) << "Config init took " << clock.getTimef() << " ms"
                            << endl;

    // 5. run main loop
    uint32_t maxFrames = 0; // set to 0 for 'endless'
    
    clock.reset();
    while( config->isRunning( ) && --maxFrames )
    {
        config->startFrame();
        // config->renderData(...);
        config->endFrame();
    }
    const uint32_t frame = config->finishFrames();
    EQLOG( eq::LOG_CUSTOM ) << "Rendering took " << clock.getTimef() << " ms ("
                            << ( frame / clock.getTimef() * 1000.f) << " FPS)"
                            << endl;

    // 6. exit config
    clock.reset();
    config->exit();
    EQLOG( eq::LOG_CUSTOM ) << "Exit took " << clock.getTimef() << " ms" <<endl;

    // 7. cleanup and exit
    server->releaseConfig( config );
    client->disconnectServer( server );
    server = 0;

    client->exitLocal();
    client = 0;

    eq::exit();
    return EXIT_SUCCESS;
}
