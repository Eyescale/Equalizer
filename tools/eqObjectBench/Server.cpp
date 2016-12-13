
/* Copyright (c) 2016-2017, Stefan.Eilemann@epfl.ch
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

#include "Server.h"

#include "MemModel.h"
#include "PlyModel.h"
#include <triply/vertexBufferRoot.h>

namespace plydist
{
namespace
{
uint32_t _chunkSize = 0;

class LocalNode : public co::LocalNode
{
    typedef co::CommandFunc< LocalNode > CmdFunc;

public:
    LocalNode( plydist::Server& parent )
        : _parent( parent )
    {
        registerCommand( CMD_NODE_DONE, CmdFunc( this, &LocalNode::_cmdDone ),
                         getCommandThreadQueue( ));
        if( _chunkSize == 0 )
            _chunkSize = co::Global::getObjectBufferSize();
    }

private:
    plydist::Server& _parent;

    bool _cmdDone( co::ICommand& ) { ++_parent.doneNodes; return true; }
};
}


Server::Server( const int argc, char** argv, const std::string& modelName,
                const Options options, co::CompressorInfo compressor )
{
    co::init( argc, argv );
    co::LocalNodePtr localNode = new LocalNode( *this );

    if( !(options & Options::instanceCache) )
        localNode->disableInstanceCache();
    if( options & Options::chunked )
        co::Global::setObjectBufferSize( _chunkSize );
    else
        co::Global::setObjectBufferSize( std::numeric_limits<uint32_t>::max( ));

    if( !localNode->initLocal( argc, argv ))
    {
        co::exit();
        LBTHROW( std::runtime_error( "Can't set up local node" ));
    }

    co::ConnectionDescriptionPtr main;
    if( localNode->getConnectionDescriptions().empty( ))
    {
        main = new co::ConnectionDescription;
        localNode->addListener( main );
    }
    else
        main = localNode->getConnectionDescriptions()[0];

    if( options & Options::sendOnRegister )
        localNode->enableSendOnRegister();

    if( options & Options::multicast )
    {
        co::ConnectionDescriptionPtr rsp = new co::ConnectionDescription;
        rsp->type = co::CONNECTIONTYPE_RSP;
        rsp->interfacename = main->hostname;
        _rsp = localNode->addListener( rsp );
    }

    const co::Object::ChangeType changeType = options & Options::buffered ?
                                                  co::Object::INSTANCE :
                                                  co::Object::UNBUFFERED;
    if( !(options & Options::compression) )
        compressor = co::CompressorInfo();

    lunchbox::Clock clock;
    if( modelName.length() > 3 &&
        modelName.compare( modelName.size() - 4, 4, ".ply" ) == 0 )
    {
        _model.reset( new PlyModel( changeType, compressor, modelName,
                                    localNode ));
    }
    else
        _model.reset( new MemModel( changeType, compressor, modelName,
                                    localNode ));
    _localNode = localNode;
}

Server::~Server()
{
    _model.reset();
    if( _localNode )
        _localNode->exitLocal();
    _localNode = nullptr;
    co::exit();
}

}
