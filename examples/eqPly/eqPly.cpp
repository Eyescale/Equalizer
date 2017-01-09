
/* Copyright (c) 2005-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "eqPly.h"

#include "config.h"
#include "localInitData.h"

#include <stdlib.h>

namespace eqPly
{

namespace
{
static const std::string _help(
    std::string( "eqPly - Equalizer polygonal rendering example\n" ) +
    std::string( "\tRun-time commands:\n" ) +
    std::string( "\t\tLeft Mouse Button:         Rotate model\n" ) +
    std::string( "\t\tMiddle Mouse Button:       Move model in X, Y\n" ) +
    std::string( "\t\tRight Mouse Button:        Move model in Z\n" ) +
    std::string( "\t\t<Cursor Keys>:             Move head in X,Y plane\n" )+
    std::string( "\t\t<Page Up,Down>:            Move head in Z\n" )+
    std::string( "\t\t<Esc>, All Mouse Buttons:  Exit program\n" ) +
    std::string( "\t\t<Space>:                   Reset camera (twice for Immersive Setup)\n" ) +
    std::string( "\t\tF1, h:                     Toggle help overlay\n" ) +
    std::string( "\t\to:                         Toggle perspective/orthographic\n"
                                                                            ) +
    std::string( "\t\ts:                         Toggle statistics overlay\n" ) +
    std::string( "\t\tw:                         Toggle wireframe mode\n" ) +
    std::string( "\t\td:                         Toggle color demo mode\n" ) +
    std::string( "\t\ti:                         Toggle usage of idle anti-aliasing\n" ) +
    std::string( "\t\tq, Q:                      Adjust non-idle image quality\n" ) +
    std::string( "\t\tn:                         Toggle navigation mode (trackball, walk)\n" ) +
    std::string( "\t\tr:                         Switch rendering mode (display list, VBO, immediate)\n" ) +
    std::string( "\t\tu:                         Toggle image compression\n" ) +
    std::string( "\t\tc:                         Switch active canvas\n" ) +
    std::string( "\t\tv:                         Switch active view\n" ) +
    std::string( "\t\tm:                         Switch model for active view\n" ) +
    std::string( "\t\tl:                         Switch layout for active canvas\n" ) +
    std::string( "\t\ta:                         Add active stereo window\n" ) +
    std::string( "\t\tp:                         Add passive stereo window\n" ) +
    std::string( "\t\tx:                         Remove window\n" ) +
    std::string( "\t\ty, Y:                      Adjust model unit\n" ) +
    std::string( "\t\tz, Z:                      Adjust eye base\n" ) +
    std::string( "\t\tF11:                       Screenshot to $PWD/eqPly_NUM.png\n" ));
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
        LBERROR << "Can't open server" << std::endl;
        return EXIT_FAILURE;
    }

    // 2. choose config
    eq::fabric::ConfigParams configParams;
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
    LBLOG( LOG_STATS ) << "Config init took " << clock.getTimef() << " ms"
                       << std::endl;

    // 4. run main loop
    uint32_t maxFrames = _initData.getMaxFrames();
    int lastFrame = 0;

    clock.reset();
    while( config->isRunning( ) && maxFrames-- )
    {
        config->startFrame();
        config->finishFrame();

        if( config->getAnimationFrame() == 1 )
        {
            const float time = clock.resetTimef();
            const size_t nFrames = config->getFinishedFrame() - lastFrame;
            lastFrame = config->getFinishedFrame();

            LBLOG( LOG_STATS ) << time << " ms for " << nFrames << " frames @ "
                               << ( nFrames / time * 1000.f) << " FPS)"
                               << std::endl;
        }

        while( !config->needRedraw( )) // wait for an event requiring redraw
        {
            if( hasCommands( )) // execute non-critical pending commands
            {
                processCommand();
                config->handleEvents(); // non-blocking
            }
            else  // no pending commands, block on user event
            {
                const eq::EventICommand& event = config->getNextEvent();
                if( !config->handleEvent( event ))
                    LBVERB << "Unhandled " << event << std::endl;
            }
        }
        config->handleEvents(); // process all pending events
    }
    const uint32_t frame = config->finishAllFrames();
    const float time = clock.resetTimef();
    const size_t nFrames = frame - lastFrame;
    LBLOG( LOG_STATS ) << time << " ms for " << nFrames << " frames @ "
                       << ( nFrames / time * 1000.f) << " FPS)" << std::endl;

    // 5. exit config
    clock.reset();
    config->exit();
    LBLOG( LOG_STATS ) << "Exit took " << clock.getTimef() << " ms" <<std::endl;

    // 6. cleanup and exit
    server->releaseConfig( config );
    if( !disconnectServer( server ))
        LBERROR << "Client::disconnectServer failed" << std::endl;

    return EXIT_SUCCESS;
}

void EqPly::clientLoop()
{
    do
    {
         eq::Client::clientLoop();
         LBINFO << "Configuration run successfully executed" << std::endl;
    }
    while( _initData.isResident( )); // execute at least one config run
}
}
