
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
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
 */

#include "vertexBufferDist.h"

#include "vertexBufferLeaf.h"
#include "vertexBufferRoot.h"

namespace triply
{

VertexBufferDist::VertexBufferDist()
    : _root( 0 )
    , _node( 0 )
    , _left( 0 )
    , _right( 0 )
    , _isRoot( false )
{}

VertexBufferDist::VertexBufferDist( VertexBufferRoot* root )
    : _root( root )
    , _node( root )
    , _left( 0 )
    , _right( 0 )
    , _isRoot( true )
{
    if( root->getLeft( ))
        _left = new VertexBufferDist( root, root->getLeft( ));

    if( root->getRight( ))
        _right = new VertexBufferDist( root, root->getRight( ));
}

VertexBufferDist::VertexBufferDist( VertexBufferRoot* root,
                                    VertexBufferBase* node )
        : _root( root )
        , _node( node )
        , _left( 0 )
        , _right( 0 )
        , _isRoot( false )
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
    LBASSERT( !isAttached() );
    LBCHECK( node->registerObject( this ));

    if( _left )
        _left->registerTree( node );

    if( _right )
        _right->registerTree( node );
}

void VertexBufferDist::deregisterTree()
{
    LBASSERT( isAttached() );
    LBASSERT( isMaster( ));

    getLocalNode()->deregisterObject( this );

    if( _left )
        _left->deregisterTree();
    if( _right )
        _right->deregisterTree();
}

VertexBufferRoot* VertexBufferDist::loadModel( co::NodePtr master,
                                                     co::LocalNodePtr localNode,
                                                     const eq::uint128_t& modelID )
{
    LBASSERT( !_root && !_node );

    if( !localNode->syncObject( this, modelID, master ))
    {
        LBWARN << "Mapping of model failed" << std::endl;
        return 0;
    }
    return _root;
}

void VertexBufferDist::getInstanceData( co::DataOStream& os )
{
    LBASSERT( _node );
    os << _isRoot;

    if( _left && _right )
    {
        os << _left->getID() << _right->getID();

        if( _isRoot )
        {
            LBASSERT( _root );
            const VertexBufferData& data = _root->_data;

            os << data.vertices << data.colors << data.normals << data.indices
               << _root->_name;
        }
    }
    else
    {
        os << eq::uint128_t() << eq::uint128_t();

        LBASSERT( dynamic_cast< const VertexBufferLeaf* >( _node ));
        const VertexBufferLeaf* leaf =
            static_cast< const VertexBufferLeaf* >( _node );

        os << leaf->_boundingBox[0] << leaf->_boundingBox[1]
           << uint64_t( leaf->_vertexStart ) << uint64_t( leaf->_indexStart )
           << uint64_t( leaf->_indexLength ) << leaf->_vertexLength;
    }

    os << _node->_boundingSphere << _node->_range;
}

void VertexBufferDist::applyInstanceData( co::DataIStream& is )
{
    LBASSERT( !_node );

    VertexBufferNode* node = 0;
    VertexBufferBase* base = 0;

    eq::uint128_t leftID, rightID;
    is >> _isRoot >> leftID >> rightID;

    if( leftID != 0 && rightID != 0 )
    {
        if( _isRoot )
        {
            VertexBufferRoot* root = new VertexBufferRoot;
            VertexBufferData& data = root->_data;

            is >> data.vertices >> data.colors >> data.normals >> data.indices
               >> root->_name;

            node  = root;
            _root = root;
        }
        else
        {
            LBASSERT( _root );
            node = new VertexBufferNode;
        }

        base   = node;
        _left  = new VertexBufferDist( _root, 0 );
        _right = new VertexBufferDist( _root, 0 );
        co::NodePtr from = is.getRemoteNode();
        co::LocalNodePtr to = is.getLocalNode();
        co::f_bool_t leftSync = to->syncObject( _left, leftID, from );
        co::f_bool_t rightSync = to->syncObject( _right, rightID, from );

        LBCHECK( leftSync.wait() && rightSync.wait( ));

        node->_left  = _left->_node;
        node->_right = _right->_node;
    }
    else
    {
        LBASSERT( !_isRoot );
        VertexBufferData& data = _root->_data;
        VertexBufferLeaf* leaf = new VertexBufferLeaf( data );

        uint64_t i1, i2, i3;
        is >> leaf->_boundingBox[0] >> leaf->_boundingBox[1]
           >> i1 >> i2 >> i3 >> leaf->_vertexLength;
        leaf->_vertexStart = size_t( i1 );
        leaf->_indexStart = size_t( i2 );
        leaf->_indexLength = size_t( i3 );

        base = leaf;
    }

    LBASSERT( base );
    is >> base->_boundingSphere >> base->_range;

    _node = base;
}

}
