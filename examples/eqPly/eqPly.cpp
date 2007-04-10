
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eqPly.h"

#include "config.h"
#include "localInitData.h"

#include <stdlib.h>

using namespace std;
using namespace eqBase;

EqPly::EqPly( const LocalInitData& initData )
        : _initData( initData )
{}

bool EqPly::initLocal( int argc, char** argv )
{
    if( _initData.isApplication( ))
        return eqNet::Node::initLocal( argc, argv );

    // else set up fixed client listener
    EQASSERT( getState() == eqNet::Node::STATE_STOPPED );
    EQASSERT( nConnectionDescriptions() == 0 );

    RefPtr<eqNet::ConnectionDescription> desc =new eqNet::ConnectionDescription;

    desc->type       = eqNet::CONNECTIONTYPE_TCPIP;
    desc->TCPIP.port = _initData.getClientPort();
    addConnectionDescription( desc );

    if( !listen( ))
    {
        EQERROR << "Can't setup listener" << endl; 
        return false;
    }
    return true;
}

int EqPly::run()
{
    if( _initData.isApplication( ))
        return runApplication();
    // else
    return runClient();
}

int EqPly::runApplication()
{
    // 1. connect to server
    RefPtr<eq::Server> server = new eq::Server;
    if( !connectServer( server ))
    {
        EQERROR << "Can't open server" << endl;
        return EXIT_FAILURE;
    }

    // 2. choose config
    eq::ConfigParams configParams;
    Config*          config = (Config*)server->chooseConfig( configParams );

    if( !config )
    {
        EQERROR << "No matching config on server" << endl;
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    // 3. init config
    eqBase::Clock clock;

    config->setInitData( _initData );
    if( !config->init( ))
    {
        EQERROR << "Config initialisation failed: " 
                << config->getErrorMessage() << endl;
        server->releaseConfig( config );
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    EQLOG( eq::LOG_CUSTOM ) << "Config init took " << clock.getTimef() << " ms"
                            << endl;

    // 4. run main loop
    uint32_t maxFrames = _initData.getMaxFrames();
    
    clock.reset();
    while( config->isRunning( ) && maxFrames-- )
    {
        config->startFrame();
        // config->renderData(...);
        config->finishFrame();
    }
    const uint32_t frame = config->finishAllFrames();
    const float    time  = clock.getTimef();
    EQLOG( eq::LOG_CUSTOM ) << "Rendering took " << time << " ms (" << frame
                            << " frames @ " << ( frame / time * 1000.f)
                            << " FPS)" << endl;

    // 5. exit config
    clock.reset();
    config->exit();
    EQLOG( eq::LOG_CUSTOM ) << "Exit took " << clock.getTimef() << " ms" <<endl;

    // 6. cleanup and exit
    server->releaseConfig( config );
    if( !disconnectServer( server ))
        EQERROR << "Client::disconnectServer failed" << endl;
    server = 0;
    return EXIT_SUCCESS;
}

int EqPly::runClient()
{
    // run client loop
    while( true ) // TODO: implement SIGHUP handler to exit?
    {
        clientLoop();
    }

    return EXIT_SUCCESS;
}
