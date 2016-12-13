
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

#include "server.h"

#include "vertexBufferDist.h"
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
    LocalNode( Result& result )
        : _result( result )
    {
        registerCommand( CMD_NODE_DONE, CmdFunc( this, &LocalNode::_cmdDone ),
                         getCommandThreadQueue( ));
        if( _chunkSize == 0 )
            _chunkSize = co::Global::getObjectBufferSize();
    }

private:
    Result& _result;

    bool _cmdDone( co::ICommand& cmd )
    {
        _result.times.push_back( cmd.read< float >( ));
        ++_result.doneNodes;
        return true;
    }
};
}


Server::Server( const int argc, char** argv, const std::string& modelName,
                const Options options, const uint32_t compressor )
{
    co::init( argc, argv );

    // Load and register model
    if( !_model.readFromFile( modelName ))
    {
        co::exit();
        LBTHROW( std::runtime_error( "Can't load model: " + modelName ));
    }

    static std::string lastModel;
    if( modelName != lastModel )
    {
        lastModel = modelName;
        size_t nObjects = 0;
        std::vector< triply::VertexBufferBase* > objects( 1, &_model );
        while( !objects.empty( ))
        {
            triply::VertexBufferBase* object = objects.back();
            objects.pop_back();
            ++nObjects;

            if( object->getLeft( ))
                objects.push_back( object->getLeft( ));
            if( object->getRight( ))
                objects.push_back( object->getRight( ));
        }
        std::cout << nObjects << " objects in model" << std::endl;
    }

    co::LocalNodePtr localNode = new LocalNode( result );
    localNode->addConnectionDescription( new co::ConnectionDescription );
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

    if( options & Options::sendOnRegister )
        localNode->enableSendOnRegister();

    if( options & Options::multicast )
    {
        co::ConnectionDescriptionPtr rsp = new co::ConnectionDescription;
        rsp->type = co::CONNECTIONTYPE_RSP;
        localNode->addListener( rsp );
    }

    if( options & Options::compression )
        if( options & Options::buffered )
            _modelDist = new VertexBufferDist< co::Object::INSTANCE >( _model,
                                                                   compressor );
        else
            _modelDist = new VertexBufferDist< co::Object::STATIC >( _model,
                                                                   compressor );
    else
        if( options & Options::buffered )
            _modelDist = new VertexBufferDist< co::Object::INSTANCE >( _model,
                                                           EQ_COMPRESSOR_NONE );
        else
            _modelDist = new VertexBufferDist< co::Object::STATIC >( _model,
                                                           EQ_COMPRESSOR_NONE );

    _modelDist->setID( modelID );

    lunchbox::Clock clock;
    _modelDist->registerTree( localNode );
    std::cout << "Registration took " << clock.getTimef() << " ms" << std::endl;

    _localNode = localNode;
}

Server::~Server()
{
    if( _modelDist )
    {
        _modelDist->releaseTree();
        delete _modelDist;
    }

    _localNode->exitLocal();
    _localNode = nullptr;
    co::exit();
}

}
