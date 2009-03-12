
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eqPly.h"

#include "config.h"
#include "localInitData.h"

#include <stdlib.h>

using namespace std;

namespace eqPly
{

namespace
{
static const std::string _help(
    string( "eqPly - Equalizer polygonal rendering example\n" ) +
    string( "\tRun-time commands:\n" ) +
    string( "\t\tLeft Mouse Button:         Rotate model\n" ) +
    string( "\t\tMiddle Mouse Button:       Move model in X, Y\n" ) +
    string( "\t\tRight Mouse Button:        Move model in Z\n" ) +
    string( "\t\t<Cursor Keys>:             Move head in X,Y plane\n" )+
    string( "\t\t<Page Up,Down>:            Move head in Z\n" )+
    string( "\t\t<Esc>, All Mouse Buttons:  Exit program\n" ) +
    string( "\t\t<Space>:                   Reset camera\n" ) +
    string( "\t\to:                         Toggle perspective/orthographic\n" ) +
    string( "\t\ts:                         Toggle statistics overlay\n" ) +
    string( "\t\tF1, h:                     Toggle help overlay\n" ) +
    string( "\t\tw:                         Toggle wireframe mode\n" ) +
    string( "\t\tr:                         Switch rendering mode\n" ) +
    string( "\t\tv:                         Switch active view\n" ) +
    string( "\t\tm:                         Switch model for active view\n" ));
}

const std::string& EqPly::getHelp()
{
    return _help;
}

EqPly::EqPly( const LocalInitData& initData )
        : _initData( initData )
{}

int EqPly::run()
{
    // 1. connect to server
    eq::ServerPtr server = new eq::Server;
    if( !connectServer( server ))
    {
        EQERROR << "Can't open server" << endl;
        return EXIT_FAILURE;
    }

    // 2. choose config
    eq::ConfigParams configParams;
    Config* config = static_cast<Config*>(server->chooseConfig( configParams ));

    if( !config )
    {
        EQERROR << "No matching config on server" << endl;
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    // 3. init config
    eq::base::Clock clock;

    config->setInitData( _initData );
    if( !config->init( ))
    {
        EQERROR << "Config initialization failed: " 
                << config->getErrorMessage() << endl;
        server->releaseConfig( config );
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    EQLOG( LOG_STATS ) << "Config init took " << clock.getTimef() << " ms"
                       << endl;

    // 4. run main loop
    uint32_t maxFrames = _initData.getMaxFrames();
    
    clock.reset();
    while( config->isRunning( ) && maxFrames-- )
    {
        config->startFrame();
        // config->renderData(...);
        config->finishFrame();

        if( !config->needsRedraw( ))
            config->finishAllFrames(); // flush, TODO flush one task at a time
            
        while( !config->needsRedraw( )) // wait for an event requiring redraw
        {
            const eq::ConfigEvent* event = config->nextEvent();
            if( !config->handleEvent( event ))
                EQVERB << "Unhandled " << event << endl;
        }
        config->handleEvents(); // process all pending events
    }
    const uint32_t frame = config->finishAllFrames();
    const float    time  = clock.getTimef();
    EQLOG( LOG_STATS ) << "Rendering took " << time << " ms (" << frame
                       << " frames @ " << ( frame / time * 1000.f) << " FPS)"
                       << endl;

    // 5. exit config
    clock.reset();
    config->exit();
    EQLOG( LOG_STATS ) << "Exit took " << clock.getTimef() << " ms" <<endl;

    // 6. cleanup and exit
    server->releaseConfig( config );
    if( !disconnectServer( server ))
        EQERROR << "Client::disconnectServer failed" << endl;

    // TODO EQASSERTINFO( server->getRefCount() == 1, server->getRefCount( ));
    server = 0;

    return EXIT_SUCCESS;
}

bool EqPly::clientLoop()
{
    if( !_initData.isResident( )) // execute only one config run
        return eq::Client::clientLoop();

    // else execute client loops 'forever'
    while( true ) // TODO: implement SIGHUP handler to exit?
    {
        if( !eq::Client::clientLoop( ))
            return false;
        EQINFO << "One configuration run successfully executed" << endl;
    }
    return true;
}
}
