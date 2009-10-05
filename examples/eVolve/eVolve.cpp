
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "eVolve.h"

#include "config.h"
#include "localInitData.h"

#include <stdlib.h>

using namespace std;

namespace eVolve
{

static const std::string _help(
    string( "eVolve - Equalizer volume rendering example\n" ) +
    string( "\tRun-time commands:\n" ) +
    string( "\t\tLeft Mouse Button:         Rotate model\n" ) +
    string( "\t\tMiddle Mouse Button:       Move model in X, Y\n" ) +
    string( "\t\tRight Mouse Button:        Move model in Z\n" ) +
    string( "\t\t<Esc>, All Mouse Buttons:  Exit program\n" ) +
    string( "\t\t<Space>, r:                Reset camera\n" ) +
    string( "\t\to:                         Toggle " ) +
    string( "perspective/orthographic\n" ) +
    string( "\t\ts:                         Toggle statistics " ) +
    string( "overlay\n" ) +
    string( "\t\tl:                         Switch layout for active canvas\n")+
    string( "\t\tF1, h:                     Toggle help overlay\n" )
 );

const std::string& EVolve::getHelp()
{
    return _help;
}

EVolve::EVolve( const LocalInitData& initData )
        : _initData( initData )
{}

int EVolve::run()
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
        config->finishFrame();
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
    server = 0;

    return EXIT_SUCCESS;
}

bool EVolve::clientLoop()
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
