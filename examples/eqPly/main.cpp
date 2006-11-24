
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eqPly.h"

#include "channel.h"
#include "config.h"
#include "frameData.h"
#include "appInitData.h"
#include "node.h"
#include "pipe.h"

#include <getopt.h>
#include <stdlib.h>
#include "tracker.h"

using namespace std;
using namespace eqBase;

#define DIE(reason)    { cout << (reason) << endl; abort(); }

static void _parseArguments( int argc, char** argv,
                             RefPtr<AppInitData> appInitData );

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

    RefPtr<AppInitData> appInitData = new AppInitData;
    _parseArguments( argc, argv, appInitData );

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

    // 4. register application data
    RefPtr<FrameData> frameData = new FrameData;

    config->registerObject( appInitData.get(), config->getLocalNode( ));
    config->registerObject( frameData.get(), config->getLocalNode( ));
    appInitData->setFrameData( frameData );
    config->setFrameData( frameData );

    // 5a. init config
    eqBase::Clock clock;
    if( !config->init( appInitData->getID( )))
        DIE("Config initialisation failed.");
    EQINFO << "Config init took " << clock.getTimef() << " ms" << endl;

    // 5b. init tracker
    Tracker tracker;
    if( !appInitData->getTrackerPort().empty( ))
    {
        if( !tracker.init( appInitData->getTrackerPort() ))
            EQWARN << "Failed to initialise tracker" << endl;
        else
        {
            // Set up position of tracking system in world space
            // Note: this depends on the installation used.
            vmml::Matrix4f m( vmml::Matrix4f::IDENTITY );
            m.scale( 1.f, 1.f, -1.f );
            //m.x = .5;
            tracker.setWorldToEmitter( m );

            m = vmml::Matrix4f::IDENTITY;
            m.rotateZ( -M_PI_2 );
            tracker.setSensorToObject( m );
            EQINFO << "Tracker initialised" << endl;
        }
    }

    // 6. run main loop
    uint32_t frame = 0;
    clock.reset();
    while( config->isRunning( ))
    {
        if( tracker.isRunning() )
        {
            tracker.update();
            const vmml::Matrix4f& headMatrix = tracker.getMatrix();
            config->setHeadMatrix( headMatrix );
        }

        config->beginFrame();
        // config->renderData(...);
        frame = config->endFrame();
    }
    EQINFO << "Rendering took " << clock.getTimef() << " ms (" << 
        (frame / clock.getTimef() * 1000.f) << " FPS)" << endl;

    // 7. exit config
    clock.reset();
    config->exit();
    EQINFO << "Exit took " << clock.getTimef() << " ms" << endl;

    // 8. cleanup and exit
    appInitData->setFrameData( NULL );
    config->setFrameData( NULL );
    config->deregisterObject( frameData.get( ));
    config->deregisterObject( appInitData.get( ));
    EQASSERT( frameData->getRefCount() == 1 );
    EQASSERT( appInitData->getRefCount() == 1 );

    server->releaseConfig( config );
    server->close();
    server = NULL;
    eq::exit();
    return EXIT_SUCCESS;
}

void _parseArguments( int argc, char** argv, RefPtr<AppInitData> appInitData )
{
    int      result;
    int      index;
    struct option options[] = 
        {
            { "model",          required_argument, NULL, 'm' },
            { "port",           required_argument, NULL, 'p' },
            { NULL,             0,                 NULL,  0 }
        };

    while( (result = getopt_long( argc, argv, "", options, &index )) != -1 )
    {
        switch( result )
        {
            case 'm':
                appInitData->setFilename( optarg );
                break;

            case 'p':
                appInitData->setTrackerPort( optarg );

            default:
                EQWARN << "unhandled option: " << options[index].name << endl;
                break;
        }
    }
}
