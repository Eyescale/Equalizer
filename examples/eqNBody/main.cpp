
/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
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

#include "client.h"
#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

#include <eq/eq.h>
#include <stdlib.h>

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config* createConfig( eq::ServerPtr parent )
        { return new eqNbody::Config( parent ); }
    virtual eq::Node* createNode( eq::Config* parent )
        { return new eqNbody::Node( parent ); }
    virtual eq::Pipe* createPipe( eq::Node* parent )
        { return new eqNbody::Pipe( parent ); }
    virtual eq::Window* createWindow( eq::Pipe* parent )
        { return new eqNbody::Window( parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new eqNbody::Channel( parent ); }
};

int main( const int argc, char** argv )
{
    NodeFactory nodeFactory;

    if( !eq::init( argc, argv, &nodeFactory ))
    {
        LBERROR << "Equalizer init failed" << std::endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    eqNbody::InitData id;
    lunchbox::RefPtr< eqNbody::Client > client = new eqNbody::Client( id );
    if( !client->initLocal( argc, argv ))
    {
        LBERROR << "Can't init client" << std::endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // Init
    if( client->init() != EXIT_SUCCESS )
    {
        LBERROR << "Can't init client" << std::endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // Run the simulation
    client->run();

    // Exit
    if( client->exit() != EXIT_SUCCESS )
    {
        LBERROR << "Can't exit client" << std::endl;
    }

    client->exitLocal();
    client = 0;

    eq::exit();
    return EXIT_SUCCESS;
}
