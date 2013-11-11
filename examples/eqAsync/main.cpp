
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010-2011, Maxim Makhinya  <maxmah@gmail.com>
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
 *
 */

#include "eqAsync.h"

#include <stdlib.h>

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Pipe*    createPipe( eq::Node* parent )
        { return new eqAsync::Pipe( parent ); }

    virtual eq::Window*  createWindow( eq::Pipe* parent )
        { return new eqAsync::Window( parent ); }

    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new eqAsync::Channel( parent ); }
};


int main( const int argc, char** argv )
{
    // 1. Equalizer initialization
    NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        LBERROR << "Equalizer init failed" << std::endl;
        return EXIT_FAILURE;
    }

    // 2. get a configuration
    bool        error  = false;
    eq::Config* config = eq::getConfig( argc, argv );
    if( config )
    {
        // 3. init config
        if( config->init( eq::uint128_t( 0 )))
        {
            if( config->getError( ))
                LBWARN << "Error during initialization: " << config->getError()
                       << std::endl;

            // 4. run main loop
            eq::uint128_t spin( 0 );
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
}
