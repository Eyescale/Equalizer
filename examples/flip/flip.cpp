
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"
#include "config.h"
#include "flip.h"
#include "frameData.h"
#include "initData.h"
#include "pipe.h"

#include <stdlib.h>

using namespace std;

#define DIE(reason)    { cout << (reason) << endl; abort(); }


class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig()  { return new ::Config; }
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
        abort();

    eq::Server     server;
    eq::OpenParams openParams;
    openParams.address = "localhost:4242";
    openParams.appName = "foo";

    if( !server.open( openParams ))
        DIE("Can't open server.");

    eq::ConfigParams configParams;
    eq::Config*      config = server.chooseConfig( configParams );

    if( !config )
        DIE("No matching config on server.");

    InitData initData;
    config->registerMobject( &initData, config->getNode( ));
    
    FrameData frameData;
    config->registerMobject( &frameData, config->getNode( ));

    initData.setFrameData( &frameData );
    initData.setFilename( "foo" );

    eqBase::Clock clock;
    if( !config->init( initData.getID( )))
        DIE("Config initialisation failed.");
    cerr << "Config init took " << clock.getTimef() << " ms" << endl;

    int nFrames = 100;
    clock.reset();
    while( nFrames-- )
    {
        // update database
        frameData.spin += .1;
        const uint32_t version = frameData.commit();

        config->frameBegin( version );
//         config->renderData(...);
//         ...;
        config->frameEnd();

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

