
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

#include "Client.h"

#include "MemModel.h"
#include "PlyModel.h"
#include "Server.h"

namespace plydist
{

Client::Client( const int argc, char** argv )
{
    // Setup network
    co::init( argc, argv );
    co::LocalNodePtr localNode = new co::LocalNode;
    LBCHECK( localNode->initLocal( argc, argv ));

    typedef co::CommandFunc< Client > CmdFunc;
    const auto queue = localNode->getCommandThreadQueue();
    localNode->registerCommand( CMD_CLIENT_MAP,
                                CmdFunc( this, &Client::_cmdMap ), queue );
    localNode->registerCommand( CMD_CLIENT_SYNC,
                                CmdFunc( this, &Client::_cmdSync ), queue );

    const co::Nodes& nodes = localNode->getNodes( false );
    if( nodes.size() != 1 )
        LBTHROW( std::runtime_error( "Client setup failure: " +
                                     std::to_string( nodes.size( )) +
                                     " nodes " ));

    co::NodePtr master = nodes.front();
    std::string modelName;
    co::uint128_t modelID;
    for( int i = 1; i < argc; ++i )
    {
        if(( std::string( "--model" ) == argv[i] ||
             std::string( "-m" ) == argv[i] ) && i+1 < argc )
        {
            modelName = argv[i+1];
        }
        if( std::string( "--model-id" ) == argv[i] && i+1 < argc )
            modelID = std::string( argv[i+1] );
    }

    master->send( CMD_NODE_DONE );

    // map model
    _map.waitEQ( true );
    std::unique_ptr< Model > model;
    if( modelName.empty() ||
        ( modelName.length() > 3 &&
          modelName.compare( modelName.size() - 4, 4, ".ply" ) == 0 ))
    {
        model.reset( new PlyModel( master, localNode, modelID ));
    }
    else
        model.reset( new MemModel( master, localNode, modelID ));
    master->send( CMD_NODE_DONE );

    // sync model
    const uint32_t nCommits = _sync.waitNE( 0 );
    for( size_t i = 0; i < nCommits; ++i )
        model->sync( eq::uint128_t( co::VERSION_NEXT ));
    master->send( CMD_NODE_DONE );

    model.reset();
    localNode->exitLocal();
    co::exit();
}
}
