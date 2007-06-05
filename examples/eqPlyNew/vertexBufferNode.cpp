/*  
    vertexBufferNode.cpp
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Implementation of the VertexBufferNode class.
*/


#include "vertexBufferNode.h"
#include "vertexBufferLeaf.h"
#include "vertexBufferState.h"
#include "vertexData.h"


#undef NDEBUG
#include <cassert>


using namespace std;
using namespace mesh;


/*  Continue kd-tree setup, create intermediary or leaf nodes as required.  */
void VertexBufferNode::setupTree( VertexData& data, const Index start,
                                  const Index length, const Axis axis,
                                  VertexBufferData& globalData )
{
    #ifndef NDEBUG
    cout << "entering VertexBufferNode::setupTree"
         << "( " << start << ", " << length << ", " << axis << " )" << endl;
    #endif
    
    data.sort( start, length, axis );
    Index median = start + ( length / 2 );
    
    // left child will include elements smaller than the median
    Index leftLength = length / 2;
    if( leftLength > LEAF_SIZE )
        _left = new VertexBufferNode;
    else
        _left = new VertexBufferLeaf( globalData );
    
    // right child will include elements equal to or greater than the median
    Index rightLength = ( length + 1 ) / 2;
    if( rightLength > LEAF_SIZE )
        _right = new VertexBufferNode;
    else
        _right = new VertexBufferLeaf( globalData );
    
    // move to next axis and continue contruction in the child nodes
    Axis newAxis = static_cast< Axis >( ( axis + 1 ) % 3 );
    static_cast< VertexBufferNode* >
        ( _left )->setupTree( data, start, leftLength, newAxis, globalData );
    static_cast< VertexBufferNode* >
        ( _right )->setupTree( data, median, rightLength, newAxis, globalData );
}


/*  Compute the bounding sphere from the children's bounding spheres.  */
BoundingBox VertexBufferNode::updateBoundingSphere()
{
    // take the bounding boxes returned from the children
    BoundingBox leftBoundingBox = 
        static_cast< VertexBufferNode* >( _left )->updateBoundingSphere();
    BoundingBox rightBoundingBox = 
        static_cast< VertexBufferNode* >( _right )->updateBoundingSphere();
    
    // merge into new bounding box
    BoundingBox boundingBox;
    for( int i = 0; i < 3; ++i )
    {
        boundingBox[0][i] = min( leftBoundingBox[0][i], 
                                 rightBoundingBox[0][i] );
        boundingBox[1][i] = max( leftBoundingBox[1][i], 
                                 rightBoundingBox[1][i] );
    }
    
    // calculate sphere from the merged bounding box
    calculateBoundingSphere( boundingBox );
    
    #ifndef NDEBUG
    cout << "exiting VertexBufferNode::updateBoundingSphere" 
         << "( " << boundingBox[0] << ", " << boundingBox[1] << " )" << endl;
    #endif
    
    return boundingBox;
}


/*  Compute the range from the children's ranges.  */
void VertexBufferNode::updateRange()
{
    // update the children's ranges
    static_cast< VertexBufferNode* >( _left )->updateRange();
    static_cast< VertexBufferNode* >( _right )->updateRange();
    
    // set node range to min/max of the children's ranges
    _range[0] = min( _left->getRange()[0], _right->getRange()[0] );
    _range[1] = max( _left->getRange()[1], _right->getRange()[1] );
    
    #ifndef NDEBUG
    cout << "exiting VertexBufferNode::updateRange" 
         << "( " << _range[0] << ", " << _range[1] << " )" << endl;
    #endif
}


/*  Render the node by rendering the children.  */
void VertexBufferNode::render( VertexBufferState& state, 
                               bool performCulling ) const
{
    if( performCulling )
    {
        // out of range check (_range is node range, range is render range)
        const Range range = state.getRange();
        if( _range[0] >= range[1] || _range[1] < range[0] )
            return;
        
        // bounding sphere culling (if culler present)
        Culler* culler = state.getCuller();
        if( culler )
        {
            switch( culler->testSphere( _boundingSphere ) )
            {
            case vmml::VISIBILITY_FULL:
                // if fully visible and fully in range, bypass further culling
                if( _range[0] >= range[0] && _range[1] < range[1] )
                {
                    _left->render( state, false );
                    _right->render( state, false );
                    return;
                }
                // partial range, fall through to partial visibility
            case vmml::VISIBILITY_PARTIAL:
                break;
            case vmml::VISIBILITY_NONE:
                // no further rendering to be done
                return;
            }
        }
    }
    
    _left->render( state );
    _right->render( state );
}


/*  Read node from memory and continue with remaining nodes.  */
void VertexBufferNode::fromMemory( char** addr, VertexBufferData& globalData )
{
    // read node itself   
    size_t nodeType;
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    assert( nodeType == NODE_TYPE );
    VertexBufferBase::fromMemory( addr, globalData );
    
    // read left child (peek ahead)
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    assert( nodeType == NODE_TYPE || nodeType == LEAF_TYPE );
    *addr -= sizeof( size_t );
    if( nodeType == NODE_TYPE )
        _left = new VertexBufferNode;
    else
        _left = new VertexBufferLeaf( globalData );
    static_cast< VertexBufferNode* >( _left )->fromMemory( addr, globalData );
    
    // read right child (peek ahead)
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    assert( nodeType == NODE_TYPE || nodeType == LEAF_TYPE );
    *addr -= sizeof( size_t );
    if( nodeType == NODE_TYPE )
        _right = new VertexBufferNode;
    else
        _right = new VertexBufferLeaf( globalData );
    static_cast< VertexBufferNode* >( _right )->fromMemory( addr, globalData );
}


/*  Write node to output stream and continue with remaining nodes.  */
void VertexBufferNode::toStream( ostream& os )
{
    size_t nodeType = NODE_TYPE;
    os.write( reinterpret_cast< char* >( &nodeType ), sizeof( size_t ) );
    VertexBufferBase::toStream( os );
    static_cast< VertexBufferNode* >( _left )->toStream( os );
    static_cast< VertexBufferNode* >( _right )->toStream( os );
}


/*  Destructor, clears up children as well.  */
VertexBufferNode::~VertexBufferNode()
{
    delete static_cast< VertexBufferNode* >( _left );
    delete static_cast< VertexBufferNode* >( _right );
}
