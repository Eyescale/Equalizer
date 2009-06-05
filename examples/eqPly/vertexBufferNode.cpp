/*  
    vertexBufferNode.cpp
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
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
  
    
    Implementation of the VertexBufferNode class.
*/


#include "vertexBufferNode.h"
#include "vertexBufferLeaf.h"
#include "vertexBufferState.h"
#include "vertexData.h"
#include <set>


using namespace std;

namespace mesh
{

inline static bool _subdivide( const Index length, const size_t depth )
{
    return ( length / 2 > LEAF_SIZE ) || ( depth < 3 && length > 1 );
}

/*  Continue kd-tree setup, create intermediary or leaf nodes as required.  */
void VertexBufferNode::setupTree( VertexData& data, const Index start,
                                  const Index length, const Axis axis,
                                  const size_t depth,
                                  VertexBufferData& globalData )
{
    #ifndef NDEBUG
    MESHINFO << "Entering VertexBufferNode::setupTree"
             << "( " << start << ", " << length << ", " << axis << ", " 
             << depth << " )." << endl;
    #endif

    data.sort( start, length, axis );
    const Index median = start + ( length / 2 );

    // left child will include elements smaller than the median
    const Index leftLength    = length / 2;
    const bool  subdivideLeft = _subdivide( leftLength, depth );

    if( subdivideLeft )
        _left = new VertexBufferNode;
    else
        _left = new VertexBufferLeaf( globalData );
    
    // right child will include elements equal to or greater than the median
    const Index rightLength    = ( length + 1 ) / 2;
    const bool  subdivideRight = _subdivide( rightLength, depth );

    if( subdivideRight )
        _right = new VertexBufferNode;
    else
        _right = new VertexBufferLeaf( globalData );
    
    // move to next axis and continue contruction in the child nodes
    const Axis newAxisLeft  = subdivideLeft ? 
                        data.getLongestAxis( start , leftLength  ) : AXIS_X;

    const Axis newAxisRight = subdivideRight ? 
                        data.getLongestAxis( median, rightLength ) : AXIS_X;

    static_cast< VertexBufferNode* >
            ( _left )->setupTree( data, start, leftLength, newAxisLeft, depth+1, 
                                  globalData );
    static_cast< VertexBufferNode* >
        ( _right )->setupTree( data, median, rightLength, newAxisRight, depth+1,
                               globalData );

}


/*  Compute the bounding sphere from the children's bounding spheres.  */
const BoundingSphere& VertexBufferNode::updateBoundingSphere()
{
    // take the bounding spheres returned by the children
    const BoundingSphere& sphere1 = _left->updateBoundingSphere();
    const BoundingSphere& sphere2 = _right->updateBoundingSphere();
    
    // compute enclosing sphere
    const Vertex center1( sphere1.xyzw );
    const Vertex center2( sphere2.xyzw );
    const Vertex c1ToC2     = center2 - center1;
    const Vertex c1ToC2Norm = c1ToC2.getNormalized();
    
    const Vertex outer1 = center1 - c1ToC2Norm * sphere1.radius;
    const Vertex outer2 = center2 + c1ToC2Norm * sphere2.radius;

    _boundingSphere        = Vertex( outer1 + outer2 ) * 0.5f;
    _boundingSphere.radius = Vertex( outer1 - outer2 ).length() * 0.5f;
    
#ifndef NDEBUG
    MESHINFO << "Exiting VertexBufferNode::updateBoundingSphere" 
             << "( " << _boundingSphere << " )." 
             << endl;
#endif
    
    return _boundingSphere;
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
    MESHINFO << "Exiting VertexBufferNode::updateRange" 
             << "( " << _range[0] << ", " << _range[1] << " )." << endl;
    #endif
}


/*  Render the node by rendering the children.  */
void VertexBufferNode::render( VertexBufferState& state ) const
{
    _left->render( state );
    _right->render( state );
}


/*  Read node from memory and continue with remaining nodes.  */
void VertexBufferNode::fromMemory( char** addr, VertexBufferData& globalData )
{
    // read node itself   
    size_t nodeType;
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    if( nodeType != NODE_TYPE )
        throw MeshException( "Error reading binary file. Expected a regular "
                             "node, but found something else instead." );
    VertexBufferBase::fromMemory( addr, globalData );
    
    // read left child (peek ahead)
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    if( nodeType != NODE_TYPE && nodeType != LEAF_TYPE )
        throw MeshException( "Error reading binary file. Expected either a "
                             "regular or a leaf node, but found neither." );
    *addr -= sizeof( size_t );
    if( nodeType == NODE_TYPE )
        _left = new VertexBufferNode;
    else
        _left = new VertexBufferLeaf( globalData );
    static_cast< VertexBufferNode* >( _left )->fromMemory( addr, globalData );
    
    // read right child (peek ahead)
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    if( nodeType != NODE_TYPE && nodeType != LEAF_TYPE )
        throw MeshException( "Error reading binary file. Expected either a "
                             "regular or a leaf node, but found neither." );
    *addr -= sizeof( size_t );
    if( nodeType == NODE_TYPE )
        _right = new VertexBufferNode;
    else
        _right = new VertexBufferLeaf( globalData );
    static_cast< VertexBufferNode* >( _right )->fromMemory( addr, globalData );
}


/*  Write node to output stream and continue with remaining nodes.  */
void VertexBufferNode::toStream( std::ostream& os )
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
}
