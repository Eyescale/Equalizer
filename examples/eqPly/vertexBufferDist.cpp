
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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
 * co::Object to distribute a model. Has a VertexBufferBase node.
 */

#include "vertexBufferDist.h"

#include "vertexBufferLeaf.h"

using namespace std;

namespace eqPly 
{

VertexBufferDist::VertexBufferDist()
        : _root( 0 )
        , _node( 0 )
        , _isRoot( false )
        , _left( 0 )
        , _right( 0 )
{}

VertexBufferDist::VertexBufferDist( const mesh::VertexBufferRoot* root )
        : _root( root )
        , _node( root )
        , _isRoot( true )
        , _left( 0 )
        , _right( 0 )
{
    if( root->getLeft( ))
        _left = new VertexBufferDist( root, root->getLeft( ));

    if( root->getRight( ))
        _right = new VertexBufferDist( root, root->getRight( ));
}

VertexBufferDist::VertexBufferDist( const mesh::VertexBufferRoot* root, 
                                    const mesh::VertexBufferBase* node )
        : _root( root )
        , _node( node )
        , _isRoot( false )
        , _left( 0 )
        , _right( 0 )
{
    if( !node )
        return;

    if( node->getLeft( ))
        _left = new VertexBufferDist( root, node->getLeft( ));

    if( node->getRight( ))
        _right = new VertexBufferDist( root, node->getRight( ));
}

VertexBufferDist::~VertexBufferDist()
{
    delete _left;
    _left = 0;
    delete _right;
    _right = 0;
}

void VertexBufferDist::registerTree( co::LocalNodePtr node )
{
    EQASSERT( !isAttached() );
    EQCHECK( node->registerObject( this ));

    if( _left )
        _left->registerTree( node );
    
    if( _right )
        _right->registerTree( node );
}

void VertexBufferDist::deregisterTree()
{
    EQASSERT( isAttached() );
    EQASSERT( isMaster( ));

    getLocalNode()->deregisterObject( this );

    if( _left )
        _left->deregisterTree();
    if( _right )
        _right->deregisterTree();
}

mesh::VertexBufferRoot* VertexBufferDist::mapModel( 
                                      co::LocalNodePtr node,
                                      const eq::uint128_t& modelID )
{
    EQASSERT( !_root && !_node );

    if( !node->mapObject( this, modelID ))
    {
        EQWARN << "Mapping of model failed" << endl;
        return 0;
    }

    return const_cast< mesh::VertexBufferRoot* >( _root );
}

void VertexBufferDist::unmapTree()
{
    EQASSERT( isAttached() );
    EQASSERT( !isMaster( ));

    getLocalNode()->unmapObject( this );

    if( _left )
        _left->unmapTree();
    if( _right )
        _right->unmapTree();
}

void VertexBufferDist::getInstanceData( co::DataOStream& os )
{
    EQASSERT( _node );
    os << _isRoot;

    if( _left && _right )
    {
        os << _left->getID() << _right->getID();

        if( _isRoot )
        {
            EQASSERT( _root );
            const mesh::VertexBufferData& data = _root->_data;
            
            os << data.vertices << data.colors << data.normals << data.indices 
               << _root->_name;
        }
    }
    else
    {
        os << co::base::UUID::ZERO << co::base::UUID::ZERO;

        EQASSERT( dynamic_cast< const mesh::VertexBufferLeaf* >( _node ));
        const mesh::VertexBufferLeaf* leaf = 
            static_cast< const mesh::VertexBufferLeaf* >( _node );

        os << uint64_t( leaf->_vertexStart ) << uint64_t( leaf->_indexStart )
           << uint64_t( leaf->_indexLength ) << leaf->_vertexLength;
    }

    os << _node->_boundingSphere << _node->_range;
}

void VertexBufferDist::applyInstanceData( co::DataIStream& is )
{
    EQASSERT( !_node );

    mesh::VertexBufferNode* node = 0;
    mesh::VertexBufferBase* base = 0;

    co::base::UUID leftID, rightID;
    is >> _isRoot >> leftID >> rightID;

    if( leftID != co::base::UUID::ZERO && rightID != co::base::UUID::ZERO )
    {
        if( _isRoot )
        {
            mesh::VertexBufferRoot* root = new mesh::VertexBufferRoot;
            mesh::VertexBufferData& data = root->_data;

            is >> data.vertices >> data.colors >> data.normals >> data.indices
               >> root->_name;

            node  = root;
            _root = root;
        }
        else
        {
            EQASSERT( _root );
            node = new mesh::VertexBufferNode;
        }

        base   = node;
        _left  = new VertexBufferDist( _root, 0 );
        _right = new VertexBufferDist( _root, 0 );
        co::LocalNodePtr localNode = getLocalNode();
        const uint32_t sync1 = localNode->mapObjectNB( _left, leftID );
        const uint32_t sync2 = localNode->mapObjectNB( _right, rightID );

        EQCHECK( localNode->mapObjectSync( sync1 ));
        EQCHECK( localNode->mapObjectSync( sync2 ));

        node->_left  = const_cast< mesh::VertexBufferBase* >( _left->_node );
        node->_right = const_cast< mesh::VertexBufferBase* >( _right->_node );
    }
    else
    {
        EQASSERT( !_isRoot );
        mesh::VertexBufferData& data = 
            const_cast< mesh::VertexBufferData& >( _root->_data );
        mesh::VertexBufferLeaf* leaf = new mesh::VertexBufferLeaf( data );

        uint64_t i1, i2, i3;
        is >> i1 >> i2 >> i3 >> leaf->_vertexLength;
        leaf->_vertexStart = size_t( i1 );
        leaf->_indexStart = size_t( i2 );
        leaf->_indexLength = size_t( i3 );

        base = leaf;
    }

    EQASSERT( base );
    is >> base->_boundingSphere >> base->_range;

    _node = base;
}

}
