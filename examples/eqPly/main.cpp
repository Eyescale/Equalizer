
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eqPly.h"

#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"

#include <stdlib.h>
#include "tracker.h"

using namespace std;
using namespace eqBase;

#define DIE(reason)    { EQERROR << (reason) << endl; abort(); }

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig()  { return new ::Config; }
    virtual eq::Node*    createNode()    { return new ::Node; }
    virtual eq::Pipe*    createPipe()    { return new ::Pipe; }
    virtual eq::Channel* createChannel() { return new ::Channel; }
};

eq::NodeFactory* eq::createNodeFactory()
{
    return new ::NodeFactory;
}

int main( int argc, char** argv )
{
    // 1. initialisation
    if( !eq::init( argc, argv ))
        DIE( "Equalizer init failed" );

    // 2. connect to server
    RefPtr<eq::Server> server = new eq::Server;
    eq::OpenParams     openParams;
    openParams.appName = argv[0];

    if( !server->open( openParams ))
        DIE("Can't open server.");

    // 3. choose config
    eq::ConfigParams configParams;
    Config*          config = (Config*)server->chooseConfig( configParams );

    if( !config )
        DIE("No matching config on server.");

    // 4. init config
    eqBase::Clock clock;

    RefPtr<AppInitData> initData = config->getInitData();
    initData->parseArguments( argc, argv );
    
    if( !config->init( ))
        DIE("Config initialisation failed.");
    EQLOG( eq::LOG_CUSTOM ) << "Config init took " << clock.getTimef() << " ms"
                            << endl;
    // 5. init tracker
    Tracker tracker;
    if( !initData->getTrackerPort().empty( ))
    {
        if( !tracker.init( initData->getTrackerPort() ))
            EQWARN << "Failed to initialise tracker" << endl;
        else
        {
            // Set up position of tracking system in world space
            // Note: this depends on the physical installation.
            vmml::Matrix4f m( vmml::Matrix4f::IDENTITY );
            m.scale( 1.f, 1.f, -1.f );
            //m.x = .5;
            tracker.setWorldToEmitter( m );

            m = vmml::Matrix4f::IDENTITY;
            m.rotateZ( -M_PI_2 );
            tracker.setSensorToObject( m );
            EQLOG( eq::LOG_CUSTOM ) << "Tracker initialised" << endl;
        }
    }
    initData = 0;

    // 6. run main loop
    uint32_t maxFrames = 0; // set to 0 for 'endless'
    
    clock.reset();
    while( config->isRunning( ) && --maxFrames )
    {
        if( tracker.isRunning() )
        {
            tracker.update();
            const vmml::Matrix4f& headMatrix = tracker.getMatrix();
            config->setHeadMatrix( headMatrix );
        }

        config->beginFrame();
        // config->renderData(...);
        config->endFrame();
    }
    const uint32_t frame = config->finishFrames();
    EQLOG( eq::LOG_CUSTOM ) << "Rendering took " << clock.getTimef() << " ms ("
                            << ( frame / clock.getTimef() * 1000.f) << " FPS)"
                            << endl;

    // 7. exit config
    clock.reset();
    config->exit();
    EQLOG( eq::LOG_CUSTOM ) << "Exit took " << clock.getTimef() << " ms" <<endl;

    // 8. cleanup and exit
    server->releaseConfig( config );
    server->close();
    server = 0;
    eq::exit();
    return EXIT_SUCCESS;
}
