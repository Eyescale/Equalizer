
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"
#include "flip.h"
#include "frameData.h"

#include <stdlib.h>

using namespace std;

#define DIE(reason)    { cout << (reason) << endl; abort(); }


class Config : public eq::Config
{
protected:
    eqNet::Mobject* instanciateMobject( const uint32_t type, const void* data, 
                                        const uint64_t dataSize )
        {
            if( type == OBJECT_FRAMEDATA )
                return new FrameData( data, dataSize );

            return eqNet::Session::instanciateMobject( type, data, dataSize );
        }
};

class NodeFactory : public eq::NodeFactory
{
public:
    virtual Config*  createConfig()  { return new ::Config; }
    virtual Channel* createChannel() { return new ::Channel; }
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

    FrameData frameData;
    config->registerMobject( &frameData, config->getNode( ));

    eqBase::Clock clock;
    if( !config->init( frameData.getID( )))
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

    //sleep( 5 );
    clock.reset();
    config->exit();
    server.releaseConfig( config );
    server.close();
    eq::exit();
    cerr << "Exit took " << clock.getTimef() << " ms" << endl;
    return EXIT_SUCCESS;
}

