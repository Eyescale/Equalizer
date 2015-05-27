
/* Copyright (c) 2009-2015, Stefan.Eilemann@epfl.ch
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

#include "channel.h"
#include "pipe.h"
#include "window.h"
#include <eq/client.h>
#include <eq/config.h>
#include <eq/init.h>
#include <eq/nodeFactory.h>
#include <eq/server.h>
#include <eq/fabric/configParams.h>

class NodeFactory : public eq::NodeFactory
{
public:
    eq::Pipe* createPipe( eq::Node* parent ) final
        { return new eqCpu::Pipe( parent ); }
    eq::Window* createWindow( eq::Pipe* parent ) final
        { return new eqCpu::Window( parent ); }
    eq::Channel* createChannel( eq::Window* parent ) final
        { return new eqCpu::Channel( parent ); }
};

int main( int argc, char** argv )
{
    // 1. initialization of local node
    NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        LBERROR << "Equalizer init failed" << std::endl;
        return EXIT_FAILURE;
    }

    eq::ClientPtr client = new eq::Client;
    if( !client->initLocal( argc, argv ))
    {
        LBERROR << "Can't init client" << std::endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // 2. connect to server
    eq::ServerPtr server = new eq::Server;
    if( !client->connectServer( server ))
    {
        LBERROR << "Can't open server" << std::endl;
        client->exitLocal();
        eq::exit();
        return EXIT_FAILURE;
    }

    // 3. choose config
    eq::fabric::ConfigParams configParams;
    eq::Config* config = server->chooseConfig( configParams );

    if( !config )
    {
        LBERROR << "No matching config on server" << std::endl;
        client->disconnectServer( server );
        client->exitLocal();
        eq::exit();
        return EXIT_FAILURE;
    }

    // 4. init config
    if( !config->init( eq::uint128_t( )))
    {
        server->releaseConfig( config );
        client->disconnectServer( server );
        client->exitLocal();
        eq::exit();
        return EXIT_FAILURE;
    }

    // 5. run main loop
    eq::uint128_t spin;
    while( config->isRunning( ))
    {
        config->startFrame( spin );
        config->finishFrame();
        ++spin;
    }

    // 6. exit config
    config->exit();

    // 7. cleanup and exit
    server->releaseConfig( config );
    if( !client->disconnectServer( server ))
        LBERROR << "Client::disconnectServer failed" << std::endl;
    server = 0;

    client->exitLocal();
    client = 0;

    eq::exit();
    return EXIT_SUCCESS;
}
