
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "EQ/volVis.h"

#include "EQ/config.h"
#include "EQ/node.h"
#include "EQ/pipe.h"
#include "EQ/window.h"
#include "EQ/channel.h"
#include "EQ/error.h"

//#include <msv/util/statLogger.h>

#include <stdlib.h>


class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig(  eq::ServerPtr parent )  { return new massVolVis::Config(  parent ); }
    virtual eq::Node*    createNode(    eq::Config* parent   )  { return new massVolVis::Node(    parent ); }
    virtual eq::Pipe*    createPipe(    eq::Node* parent     )  { return new massVolVis::Pipe(    parent ); }
    virtual eq::Window*  createWindow(  eq::Pipe* parent     )  { return new massVolVis::Window(  parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent   )  { return new massVolVis::Channel( parent ); }
};

using namespace std;
int main( const int argc, char** argv )
{
    // 1. Equalizer initialization
    NodeFactory nodeFactory;
    massVolVis::initErrors();

    if( !eq::init( argc, argv, &nodeFactory ))
    {
        LBERROR << "Equalizer init failed" << std::endl;
        massVolVis::exitErrors();
        return EXIT_FAILURE;
    }

    // 2. parse arguments
    massVolVis::LocalInitData initData;
    initData.parseArguments( argc, argv );

    // 3. initialization of local client node
    lunchbox::RefPtr< massVolVis::VolVis > client = new massVolVis::VolVis( initData );
    if( !client->initLocal( argc, argv ))
    {
        LBERROR << "Can't init client" << std::endl;
        eq::exit();
        massVolVis::exitErrors();
        return EXIT_FAILURE;
    }

/*    util::EventLogger& logger = *util::StatLogger::instance().createLogger( "main" );
    logger << "test " << (int)12;
    logger << "test1 " << (int)13 << std::endl;
    sleep( 1 );
    logger << "test2 " << (int)14 << std::endl;
    return 0;
*/
    // 4. run client
    const int ret = client->run();

    // 5. cleanup and exit
    client->exitLocal();

    EQASSERTINFO( client->getRefCount() == 1, "Client still referenced by " <<
                  client->getRefCount() - 1 );
    client = 0;

    eq::exit();
    massVolVis::exitErrors();
    return ret;
}

/*    // 2. get a configuration
    bool        error  = false;
    eq::Config* config = eq::getConfig( argc, argv );
    if( config )
    {
        // 3. init config
        if( config->init( 0 ))
        {
            if( config->getError( ))
                LBWARN << "Error during initialization: " << config->getError()
                       << std::endl;

            // 4. run main loop
            eq::uint128_t spin = 0;
            while( config->isRunning( ))
            {
                config->startFrame( ++spin );
                config->finishFrame();
            }

            // 5. exit config
            config->exit();
        }
        else
            error = true;

        // 6. release config
        eq::releaseConfig( config );
    }
    else
    {
        LBERROR << "Cannot get config" << std::endl;
        error = true;
    }

    // 7. exit
    eq::exit();
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}*/







