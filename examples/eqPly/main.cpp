
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eqPly.h"

#include "channel.h"
#include "config.h"
#include "frameData.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"

#include <getopt.h>
#include <stdlib.h>

using namespace std;
using namespace eqBase;

#define DIE(reason)    { cout << (reason) << endl; abort(); }

static void _parseArguments( int argc, char** argv,  InitData* initData );

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

    InitData* initData = new InitData;
    _parseArguments( argc, argv, initData );

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
    FrameData *frameData = new FrameData;

    config->registerObject( initData, config->getLocalNode( ));
    config->registerObject( frameData, config->getLocalNode( ));
    initData->setFrameData( frameData );
    config->setFrameData( frameData );

    // 5. init config
    eqBase::Clock clock;
    if( !config->init( initData->getID( )))
        DIE("Config initialisation failed.");
    cerr << "Config init took " << clock.getTimef() << " ms" << endl;

    // 6. run main loop
    clock.reset();
    while( config->isRunning( ))
    {
        config->beginFrame();
        // config->renderData(...);
        config->endFrame();
    }
    cerr << "Rendering took " << clock.getTimef() << " ms" << endl;

    // 7. exit config
    clock.reset();
    config->exit();
    cerr << "Exit took " << clock.getTimef() << " ms" << endl;

    // 8. cleanup and exit
    EQASSERT( frameData->getRefCount() == 1 );
    EQASSERT( initData->getRefCount() == 1 );
    config->deregisterObject( frameData );
    config->deregisterObject( initData );
    server->releaseConfig( config );
    server->close();
    server = NULL;
    eq::exit();
    return EXIT_SUCCESS;
}

void _parseArguments( int argc, char** argv, InitData* initData )
{
    int      result;
    int      index;
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
                initData->setFilename( optarg );
                break;

            default:
                EQWARN << "unhandled option: " << options[index].name << endl;
                break;
        }
    }
}
