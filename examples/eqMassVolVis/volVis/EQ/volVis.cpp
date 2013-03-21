
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "volVis.h"

#include "guiDock.h"

#include "config.h"
#include "localInitData.h"

#include <msv/util/statLogger.h>

#include <stdlib.h>

namespace massVolVis
{


const std::string& VolVis::getHelp()
{
    static const std::string help(
        std::string( "volVis - Large Volumes Rendering programm\n" ) +
        std::string( "\tRun-time commands:\n" ) +
        std::string( "\t\tLeft Mouse Button:         Rotate model\n" ) +
        std::string( "\t\tMiddle Mouse Button:       Move model in X, Y\n" ) +
        std::string( "\t\tRight Mouse Button:        Move model in Z\n" ) +
        std::string( "\t\t<Esc>, All Mouse Buttons:  Exit program\n" ) +
        std::string( "\t\t<Space>:                   Reset camera\n" ) +
        std::string( "\t\td:                         Toggle demo color mode\n" ) +
        std::string( "\t\tb:                         Toggle background color\n" ) +
        std::string( "\t\tn:                         Toggle normals Quality mode (raw data only)\n" ) +
        std::string( "\t\tm:                         Toggle navigation mode (trackball, walk)\n" ) +
        std::string( "\t\tp:                         Make a screenshot\n" ) +
        std::string( "\t\tr:                         Start/Stop recording\n" ) +
        std::string( "\t\ts:                         Toggle statistics\n" ) +
        std::string( "\t\tw:                         Toggle bounding boxes\n" ) +
        std::string( "\t\tup/down:                   Change rendering budget\n" ) +
        std::string( "overlay\n" ) +
        std::string( "\t\tl:                         Switch layout for active canvas\n")+
        std::string( "\t\tF1, h:                     Toggle help overlay\n" )
    );

    return help;
}


VolVis::VolVis( const LocalInitData& initData )
        : _initData( initData )
{
}


VolVis::~VolVis()
{
}


int VolVis::run()
{
    // 1. connect to server
    eq::ServerPtr server = new eq::Server;
    if( !connectServer( server ))
    {
        LBERROR << "Can't open server" << std::endl;
        return EXIT_FAILURE;
    }

    // 2. choose config
    eq::ConfigParams configParams;
    Config* config = static_cast<Config*>(server->chooseConfig( configParams ));

    if( !config )
    {
        LBERROR << "No matching config on server" << std::endl;
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    GUIDock guiDock;
    if( !guiDock.init( config ))
    {
        LBERROR << "Can't intialize GUI handler" << std::endl;

        config->exit();
        server->releaseConfig( config );
        if( !disconnectServer( server ))
            LBERROR << "Client::disconnectServer failed" << std::endl;
        server = 0;

        return EXIT_FAILURE;
    }

    // 3. init config
    lunchbox::Clock clock;

    config->setInitData( _initData );
    if( !config->init( ))
    {
        server->releaseConfig( config );
        disconnectServer( server );
        return EXIT_FAILURE;
    }
    else if( config->getError( ))
        LBWARN << "Error during initialization: " << config->getError()
               << std::endl;

    EQLOG( LOG_STATS ) << "Config init took " << clock.getTimef() << " ms"
                       << std::endl;

    // 4. run main loop
    uint32_t maxFrames = _initData.getMaxFrames();

    clock.reset();
    while( config->isRunning( ) && maxFrames-- )
    {
        config->startFrame();
        if( config->getError( ))
            LBWARN << "Error during frame start: " << config->getError()
                   << std::endl;
        config->finishFrame();
    }
    const uint32_t frame = config->finishAllFrames();
    const float    time  = clock.getTimef();
    EQLOG( LOG_STATS ) << "Rendering took " << time << " ms (" << frame
                       << " frames @ " << ( frame / time * 1000.f) << " FPS)"
                       << std::endl;


    if( _initData.dumpStatistics() )
        util::StatLogger::instance().printStats();

    // 5. exit config
    clock.reset();
    config->exit();
    EQLOG( LOG_STATS ) << "Exit took " << clock.getTimef() << " ms" <<std::endl;

    // 6. cleanup and exit
    server->releaseConfig( config );
    if( !disconnectServer( server ))
        LBERROR << "Client::disconnectServer failed" << std::endl;
    server = 0;

    return EXIT_SUCCESS;
}

void VolVis::clientLoop()
{
    do
    {
         eq::Client::clientLoop();
         LBINFO << "Configuration run successfully executed" << std::endl;
    }
    while( _initData.isResident( )); // execute at lease one config run
}
}
