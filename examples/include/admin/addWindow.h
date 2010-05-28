
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided
 * with the distribution. Neither the name of Eyescale Software GmbH nor the
 * names of its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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


#include <eq/admin.h>

namespace eqAdmin
{

inline bool addWindow( eq::admin::ServerPtr server )
{
    // Find first pipe...
    const eq::admin::ConfigVector& configs = server->getConfigs();
    if( configs.empty( ))
    {
        std::cout << "No configs on server, exiting" << std::endl;
        return false;
    }

    eq::admin::Config* config = configs.front();
    const eq::admin::NodeVector& nodes = config->getNodes();
    if( nodes.empty( ))
    {
        std::cout << "No nodes in config, exiting" << std::endl;
        return false;
    }
 
    const eq::admin::Node* node = nodes.front();
    const eq::admin::PipeVector& pipes = node->getPipes();
    if( pipes.empty( ))
    {
        std::cout << "No pipes in node, exiting" << std::endl;
        return false;
    }

    eq::admin::Pipe* pipe = pipes.front();
    EQASSERT( pipe );

    // Add window (->channel->segment->canvas->layout->view)
    eq::admin::Window* window = new eq::admin::Window( pipe );
    eq::admin::Channel* channel = new eq::admin::Channel( window );
    eq::admin::Canvas* canvas = new eq::admin::Canvas( config );
    eq::admin::Segment* segment = new eq::admin::Segment( canvas );
    eq::admin::Layout* layout = new eq::admin::Layout( config );
    new eq::admin::View( layout );

    window->setPixelViewport( eq::fabric::PixelViewport( 100, 100, 400, 300 ));
    window->setName( "Runtime-created window" );
    
    segment->setChannel( channel );
    canvas->addLayout( layout );
    canvas->setWall( eq::fabric::Wall( eq::fabric::Vector3f(-.4f,-.3f,-1.f ),
                                       eq::fabric::Vector3f( .4f,-.3f,-1.f ),
                                       eq::fabric::Vector3f(-.4f, .3f,-1.f )));
    config->commit();
    return true;
}

}
