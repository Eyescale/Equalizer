/*  
 *   Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  
 *
 * eq::net::Object to distribute a model. Has a VertexBufferBase node.
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

void VertexBufferDist::registerTree( eq::net::Session* session )
{
    EQASSERT( getID() == EQ_ID_INVALID );
    session->registerObject( this );

    if( _left )
        _left->registerTree( session );
    if( _right )
        _right->registerTree( session );
}

void VertexBufferDist::deregisterTree()
{
    EQASSERT( getID() != EQ_ID_INVALID );
    EQASSERT( isMaster( ));

    getSession()->deregisterObject( this );

    if( _left )
        _left->deregisterTree();
    if( _right )
        _right->deregisterTree();
}

mesh::VertexBufferRoot* VertexBufferDist::mapModel( eq::net::Session* session,
                                                    const uint32_t modelID )
{
    EQASSERT( !_root && !_node );

    if( !session->mapObject( this, modelID ))
    {
        EQWARN << "Mapping of model failed" << endl;
        return 0;
    }

    return const_cast< mesh::VertexBufferRoot* >( _root );
}

void VertexBufferDist::unmapTree()
{
    EQASSERT( getID() != EQ_ID_INVALID );
    EQASSERT( !isMaster( ));

    getSession()->unmapObject( this );

    if( _left )
        _left->unmapTree();
    if( _right )
        _right->unmapTree();
}

void VertexBufferDist::getInstanceData( eq::net::DataOStream& os )
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
        os << EQ_ID_INVALID << EQ_ID_INVALID;

        EQASSERT( dynamic_cast< const mesh::VertexBufferLeaf* >( _node ));
        const mesh::VertexBufferLeaf* leaf = 
            static_cast< const mesh::VertexBufferLeaf* >( _node );

        os << leaf->_vertexStart << leaf->_vertexLength << leaf->_indexStart
           << leaf->_indexLength;
    }

    os << _node->_boundingSphere << _node->_range;
}

void VertexBufferDist::applyInstanceData( eq::net::DataIStream& is )
{
    EQASSERT( !_node );

    mesh::VertexBufferNode* node = 0;
    mesh::VertexBufferBase* base = 0;

    uint32_t leftID, rightID;
    is >> _isRoot >> leftID >> rightID;

    if( leftID != EQ_ID_INVALID && rightID != EQ_ID_INVALID )
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

        eq::net::Session* session = getSession();
        const uint32_t sync1 = session->mapObjectNB( _left, leftID );
        const uint32_t sync2 = session->mapObjectNB( _right, rightID );
        EQASSERT( sync1 != EQ_ID_INVALID );
        EQASSERT( sync2 != EQ_ID_INVALID );

        EQCHECK( session->mapObjectSync( sync1 ));
        EQCHECK( session->mapObjectSync( sync2 ));

        node->_left  = const_cast< mesh::VertexBufferBase* >( _left->_node );
        node->_right = const_cast< mesh::VertexBufferBase* >( _right->_node );
    }
    else
    {
        EQASSERT( !_isRoot );
        mesh::VertexBufferData& data = 
            const_cast< mesh::VertexBufferData& >( _root->_data );
        mesh::VertexBufferLeaf* leaf = new mesh::VertexBufferLeaf( data );

        is >> leaf->_vertexStart >> leaf->_vertexLength >> leaf->_indexStart
           >> leaf->_indexLength;

        base = leaf;
    }

    EQASSERT( base );
    is >> base->_boundingSphere >> base->_range;

    _node = base;
}

}
