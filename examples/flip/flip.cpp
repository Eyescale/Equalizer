
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "flip.h"

#include "channel.h"
#include "config.h"
#include "frameData.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"

#include <getopt.h>
#include <stdlib.h>

using namespace std;

#define DIE(reason)    { cout << (reason) << endl; abort(); }


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
    if( !eq::init( argc, argv ))
        DIE( "Equalizer init failed" );

    eq::OpenParams openParams;
    InitData       initData;

    int           result;
    int           index;
    struct option options[] = 
        {
            { "model",          required_argument, NULL, 'm' },
            { NULL,             0,                 NULL,  0 }
        };

    while( (result = getopt_long( argc, argv, "", options, &index )) != -1 )
    {
        switch( result )
        {
            case 'm':
                initData.setFilename( optarg );
                break;

            default:
                EQWARN << "unhandled option: " << options[index].name << endl;
                break;
        }
    }

    eq::Server     server;
    openParams.appName = "foo";

    if( !server.open( openParams ))
        DIE("Can't open server.");

    eq::ConfigParams configParams;
    eq::Config*      config = server.chooseConfig( configParams );

    if( !config )
        DIE("No matching config on server.");

    config->registerObject( &initData, config->getNode( ));
    
    FrameData frameData;
    config->registerObject( &frameData, config->getNode( ));

    initData.setFrameData( &frameData );

    eqBase::Clock clock;
    if( !config->init( initData.getID( )))
        DIE("Config initialisation failed.");
    cerr << "Config init took " << clock.getTimef() << " ms" << endl;

    int nFrames = 100;
    clock.reset();
    while( nFrames-- )
    {
        // update database
        frameData.spin += .5;
        const uint32_t version = frameData.commit();

        config->beginFrame( version );
//         config->renderData(...);
//         ...;
        config->endFrame();

        // process events
    }
    cerr << "Rendering took " << clock.getTimef() << " ms" << endl;

    //sleep( 10 );
    clock.reset();
    config->exit();
    server.releaseConfig( config );
    server.close();
    eq::exit();
    cerr << "Exit took " << clock.getTimef() << " ms" << endl;
    return EXIT_SUCCESS;
}
