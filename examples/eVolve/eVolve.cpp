
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "eVolve.h"

#include "config.h"
#include "localInitData.h"

#include <stdlib.h>

namespace eVolve
{

static const std::string _help(
    std::string( "eVolve - Equalizer volume rendering example\n" ) +
    std::string( "\tRun-time commands:\n" ) +
    std::string( "\t\tLeft Mouse Button:         Rotate model\n" ) +
    std::string( "\t\tMiddle Mouse Button:       Move model in X, Y\n" ) +
    std::string( "\t\tRight Mouse Button:        Move model in Z\n" ) +
    std::string( "\t\t<Esc>, All Mouse Buttons:  Exit program\n" ) +
    std::string( "\t\t<Space>, r:                Reset camera\n" ) +
    std::string( "\t\td:                         Toggle demo color mode\n" ) +
    std::string( "\t\tb:                         Toggle background color\n" ) +
    std::string( "\t\tn:                         Toggle normals Quality mode (raw data only)\n" ) +
    std::string( "\t\to:                         Toggle " ) +
    std::string( "perspective/orthographic\n" ) +
    std::string( "\t\ts:                         Toggle statistics " ) +
    std::string( "overlay\n" ) +
    std::string( "\t\tl:                         Switch layout for active canvas\n")+
    std::string( "\t\tF1, h:                     Toggle help overlay\n" )
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

void EVolve::clientLoop()
{
    do
    {
         eq::Client::clientLoop();
         LBINFO << "Configuration run successfully executed" << std::endl;
    }
    while( _initData.isResident( )); // execute at lease one config run
}
}
