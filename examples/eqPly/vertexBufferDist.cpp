/*  
 *   Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
 *   All rights reserved.  
 *
 * eqNet::Object to distribute a model. Has a VertexBufferBase node.
 */

#include "vertexBufferDist.h"

#include "vertexBufferLeaf.h"

using namespace std;

namespace eqPly 
{

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

void VertexBufferDist::registerTree( eqNet::Session* session )
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

mesh::VertexBufferRoot* VertexBufferDist::mapModel(eqNet::Session* session,
                                                   const uint32_t modelID )
{
    VertexBufferDist modelDist( 0, 0 );

    if( !session->mapObject( &modelDist, modelID ))
    {
        EQWARN << "Mapping of model failed" << endl;
        return 0;
    }

    modelDist._unmapTree(); // No longer needed    

    return const_cast< mesh::VertexBufferRoot* >( modelDist._root );
}

void VertexBufferDist::_unmapTree()
{
    EQASSERT( getID() != EQ_ID_INVALID );
    EQASSERT( !isMaster( ));

    getSession()->unmapObject( this );

    if( _left )
        _left->_unmapTree();
    if( _right )
        _right->_unmapTree();
}

void VertexBufferDist::getInstanceData( eqNet::DataOStream& os )
{
    EQASSERT( _node );
    os << _isRoot;

    if( _left && _right )
    {
        os << _left->getID() << _right->getID();

        if( _isRoot )
        {
            const mesh::VertexBufferRoot* root = 
                static_cast< const mesh::VertexBufferRoot* >( _node );
            const mesh::VertexBufferData& data = root->_data;
            
            os << data.vertices << data.colors << data.normals << data.indices
               << root->_invertFaces;
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

void VertexBufferDist::applyInstanceData( eqNet::DataIStream& is )
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
               >> root->_invertFaces;

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

        eqNet::Session* session = getSession();
        const uint32_t sync1 = session->mapObjectNB( _left, leftID );
        const uint32_t sync2 = session->mapObjectNB( _right, rightID );
        EQASSERT( sync1 != EQ_ID_INVALID );
        EQASSERT( sync2 != EQ_ID_INVALID );

        const bool mapped1 = session->mapObjectSync( sync1 );
        const bool mapped2 = session->mapObjectSync( sync2 );
        EQASSERT( mapped1 );
        EQASSERT( mapped2 );

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
