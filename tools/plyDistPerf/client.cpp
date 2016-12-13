
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#include "server.h"

#include <triply/vertexBufferDist.h>
#include <triply/vertexBufferRoot.h>

namespace plydist
{
Client::Client( const int argc, char** argv )
{
    co::init( argc, argv );
    co::LocalNodePtr localNode = new co::LocalNode;
    LBCHECK( localNode->initLocal( argc, argv ));

    const co::Nodes& nodes = localNode->getNodes( false );
    if( nodes.size() != 1 )
        LBTHROW( std::runtime_error( "Client setup failure: " +
                                     std::to_string( nodes.size( )) +
                                     " nodes " ));

    co::NodePtr master = nodes.front();

    lunchbox::Clock clock;

    triply::VertexBufferDist modelDist;
    triply::VertexBufferRoot* model = modelDist.loadModel( master, localNode,
                                                           modelID );
    const float time = clock.getTimef();
    if( model )
    {
        modelDist.releaseTree();
        delete model;
        master->send( CMD_NODE_DONE ) << time;
    }

    localNode->exitLocal();
    co::exit();
}
}
