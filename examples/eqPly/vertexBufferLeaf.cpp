/*  
    vertexBufferLeaf.cpp
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
    All rights reserved.  
    
    Implementation of the VertexBufferLeaf class.
*/


#include "vertexBufferLeaf.h"
#include "vertexBufferData.h"
#include "vertexBufferState.h"
#include "vertexData.h"
#include <map>

using namespace std;

namespace mesh
{

/*  Finish partial setup - sort, reindex and merge into global data.  */
void VertexBufferLeaf::setupTree( VertexData& data, const Index start,
                                  const Index length, const Axis axis,
                                  const size_t depth,
                                  VertexBufferData& globalData )
{
    #ifndef NDEBUG
    MESHINFO << "Entering VertexBufferLeaf::setupTree"
             << "( " << start << ", " << length << ", " << axis << ", " 
             << depth << " )." << endl;
    #endif
    
    data.sort( start, length, axis );
    _vertexStart = globalData.vertices.size();
    _vertexLength = 0;
    _indexStart = globalData.indices.size();
    _indexLength = 0;
    
    const bool hasColors = ( data.colors.size() > 0 ); 
    
    // stores the new indices (relative to _start)
    map< Index, ShortIndex > newIndex;
    
    for( Index t = 0; t < length; ++t )
    {
        for( Index v = 0; v < 3; ++v )
        {
            Index i = data.triangles[start + t][v];
            if( newIndex.find( i ) == newIndex.end() )
            {
                newIndex[i] = _vertexLength++;
                // assert number of vertices does not exceed SmallIndex range
                MESHASSERT( _vertexLength );
                globalData.vertices.push_back( data.vertices[i] );
                if( hasColors )
                    globalData.colors.push_back( data.colors[i] );
                globalData.normals.push_back( data.normals[i] );
            }
            globalData.indices.push_back( newIndex[i] );
            ++_indexLength; 
        }
    }
    
    #ifndef NDEBUG
    MESHINFO << "Exiting VertexBufferLeaf::setupTree"
             << "( " << _indexStart << ", " << _indexLength << "; " 
             << _vertexStart << ", " << _vertexLength << " )." << endl;
    #else
    MESHINFO << "Leaf " << this << " contains " << _vertexLength << " vertices"
             << " and " << _indexLength / 3 << " triangles." << endl;
    #endif
}


/*  Compute the bounding sphere of the leaf's indexed vertices.  */
const BoundingSphere& VertexBufferLeaf::updateBoundingSphere()
{
    // We determine a bounding sphere by:
    // 1) Using the inner sphere of the dominant axis of the bounding box as an
    //    estimate
    // 2) Test all points to be in that sphere
    // 3) Expand the sphere to contain all points outside.


    // 1a) initialize and compute a bounding box
    BoundingBox boundingBox;
    boundingBox[0] = 
        _globalData.vertices[ _vertexStart + _globalData.indices[_indexStart] ];
    boundingBox[1] = 
        _globalData.vertices[ _vertexStart + _globalData.indices[_indexStart] ];

    for( Index offset = 1; offset < _indexLength; ++offset )
    {
        const Vertex& vertex = 
            _globalData.vertices[ _vertexStart + 
                                  _globalData.indices[_indexStart + offset] ];

        boundingBox[0][0] = min( boundingBox[0][0], vertex[0] );
        boundingBox[1][0] = max( boundingBox[1][0], vertex[0] );
        boundingBox[0][1] = min( boundingBox[0][1], vertex[1] );
        boundingBox[1][1] = max( boundingBox[1][1], vertex[1] );
        boundingBox[0][2] = min( boundingBox[0][2], vertex[2] );
        boundingBox[1][2] = max( boundingBox[1][2], vertex[2] );
    }
    
    // 1b) get inner sphere of bounding box as an initial estimate
    _boundingSphere.center.x = ( boundingBox[0].x + boundingBox[1].x ) * 0.5f;
    _boundingSphere.center.y = ( boundingBox[0].y + boundingBox[1].y ) * 0.5f;
    _boundingSphere.center.z = ( boundingBox[0].z + boundingBox[1].z ) * 0.5f;

    _boundingSphere.radius  = EQ_MAX( boundingBox[1].x - boundingBox[0].x,
                                      boundingBox[1].y - boundingBox[0].y );
    _boundingSphere.radius  = EQ_MAX( boundingBox[1].z - boundingBox[0].z,
                                      _boundingSphere.radius );
    _boundingSphere.radius *= .5f;

    float  radius        = _boundingSphere.radius;
    float  radiusSquared =  radius * radius;
    Vertex center( _boundingSphere.xyzw );

    // 2) test all points to be in the estimated bounding sphere
    for( Index offset = 0; offset < _indexLength; ++offset )
    {
        const Vertex& vertex = 
            _globalData.vertices[ _vertexStart + 
                                  _globalData.indices[_indexStart + offset] ];
        
        const Vertex centerToPoint   = vertex - center;
        const float  distanceSquared = centerToPoint.lengthSquared();
        if( distanceSquared <= radiusSquared ) // point is inside existing BS
            continue;

        // 3) expand sphere to contain 'outside' points
        const float distance = sqrtf( distanceSquared );
        const float delta    = distance - radius;

        radius        = ( radius + distance ) * .5f;
        radiusSquared = radius * radius;
        center       += centerToPoint.getNormalized() * ( 0.5f * delta );

        EQASSERTINFO( Vertex( vertex-center ).lengthSquared() <= 
                      ( radiusSquared + 2.f* numeric_limits<float>::epsilon( )),
                      vertex << " c " << center << " r " << radius << " (" 
                             << Vertex( vertex-center ).length() << ")" );
    }

    // store optimal bounding sphere 
    _boundingSphere        = center;
    _boundingSphere.radius = radius;

#ifndef NDEBUG
    MESHINFO << "Exiting VertexBufferLeaf::updateBoundingSphere" 
             << "( " << _boundingSphere << " )." << endl;
#endif
    
    return _boundingSphere;
}


/*  Compute the range of this child.  */
void VertexBufferLeaf::updateRange()
{
    _range[0] = 1.0f * _indexStart / _globalData.indices.size();
    _range[1] = _range[0] + 1.0f * _indexLength / _globalData.indices.size();
    
    #ifndef NDEBUG
    MESHINFO << "Exiting VertexBufferLeaf::updateRange" 
             << "( " << _range[0] << ", " << _range[1] << " )." << endl;
    #endif
}

#define glewGetContext state.glewGetContext

/*  Set up rendering of the leaf nodes.  */
void VertexBufferLeaf::setupRendering( VertexBufferState& state,
                                       GLuint* data ) const
{
    switch( state.getRenderMode() )
    {
    case RENDER_MODE_IMMEDIATE:
        break;

    case RENDER_MODE_BUFFER_OBJECT:
    {
        MESHINFO << "Setting up VBO rendering for leaf " << this << "." << endl;
        const char* charThis = reinterpret_cast< const char* >( this );
        
        if( data[VERTEX_OBJECT] == state.FAILED )
            data[VERTEX_OBJECT] = state.newBufferObject( charThis + 0 );
        glBindBuffer( GL_ARRAY_BUFFER, data[VERTEX_OBJECT] );
        glBufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Vertex ),
                        &_globalData.vertices[_vertexStart], GL_STATIC_DRAW );
        
        if( data[NORMAL_OBJECT] == state.FAILED )
            data[NORMAL_OBJECT] = state.newBufferObject( charThis + 1 );
        glBindBuffer( GL_ARRAY_BUFFER, data[NORMAL_OBJECT] );
        glBufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Normal ),
                        &_globalData.normals[_vertexStart], GL_STATIC_DRAW );
        
        if( data[COLOR_OBJECT] == state.FAILED )
            data[COLOR_OBJECT] = state.newBufferObject( charThis + 2 );
        if( state.useColors() )
        {
            glBindBuffer( GL_ARRAY_BUFFER, data[COLOR_OBJECT] );
            glBufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Color ),
                            &_globalData.colors[_vertexStart], GL_STATIC_DRAW );
        }
        
        if( data[INDEX_OBJECT] == state.FAILED )
            data[INDEX_OBJECT] = state.newBufferObject( charThis + 3 );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data[INDEX_OBJECT] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, 
                        _indexLength * sizeof( ShortIndex ),
                        &_globalData.indices[_indexStart], GL_STATIC_DRAW );
        
        break;
    }        
    case RENDER_MODE_DISPLAY_LIST:
    default:
    {
        MESHINFO << "Setting up display list rendering for leaf " << this
                 << "." << endl;
        if( data[0] == state.FAILED )
            data[0] = state.newDisplayList( this );
        glNewList( data[0], GL_COMPILE );
        renderImmediate( state );
        glEndList();
        break;
    }
    }

    MESHINFO << "Leaf " << this << " contains " << _vertexLength << " vertices"
             << " and " << _indexLength / 3 << " triangles." << endl;
}


/*  Render the leaf.  */
void VertexBufferLeaf::render( VertexBufferState& state ) const
{
    switch( state.getRenderMode() )
    {
    case RENDER_MODE_IMMEDIATE:
        renderImmediate( state );
        return;
    case RENDER_MODE_BUFFER_OBJECT:
        renderBufferObject( state );
        return;
    case RENDER_MODE_DISPLAY_LIST:
    default:
        renderDisplayList( state );
        return;
    }
}


/*  Render the leaf with buffer objects.  */
void VertexBufferLeaf::renderBufferObject( VertexBufferState& state ) const
{
    GLuint buffers[4];
    for( int i = 0; i < 4; ++i )
        buffers[i] = 
            state.getBufferObject( reinterpret_cast< const char* >(this) + i );
    if( buffers[VERTEX_OBJECT] == state.FAILED || 
        buffers[NORMAL_OBJECT] == state.FAILED || 
        buffers[COLOR_OBJECT] == state.FAILED || 
        buffers[INDEX_OBJECT] == state.FAILED )

        setupRendering( state, buffers );
    
    if( state.useColors() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, buffers[COLOR_OBJECT] );
        glColorPointer( 4, GL_UNSIGNED_BYTE, 0, 0 );
    }
    glBindBuffer( GL_ARRAY_BUFFER, buffers[NORMAL_OBJECT] );
    glNormalPointer( GL_FLOAT, 0, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, buffers[VERTEX_OBJECT] );
    glVertexPointer( 3, GL_FLOAT, 0, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_OBJECT] );
    glDrawElements( GL_TRIANGLES, _indexLength, GL_UNSIGNED_SHORT, 0 );
}


/*  Render the leaf with a display list.  */
inline
void VertexBufferLeaf::renderDisplayList( VertexBufferState& state ) const
{
    GLuint displayList = state.getDisplayList( this );
    if( displayList == state.FAILED )
        setupRendering( state, &displayList );
    
    glCallList( displayList );
}


/*  Render the leaf with immediate mode primitives or vertex arrays.  */
inline
void VertexBufferLeaf::renderImmediate( VertexBufferState& state ) const
{
    glBegin( GL_TRIANGLES );  
    for( Index offset = 0; offset < _indexLength; ++offset )
    {
        const Index i =_vertexStart + _globalData.indices[_indexStart + offset];
        if( state.useColors() )
            glColor4ubv( &_globalData.colors[i][0] );
        glNormal3fv( &_globalData.normals[i][0] );
        glVertex3fv( &_globalData.vertices[i][0] );
    }
    glEnd();
    
//    if( state.useColors() )
//        glColorPointer( 4, GL_UNSIGNED_BYTE, 0, 
//                        &_globalData.colors[_vertexStart] );
//    glNormalPointer( GL_FLOAT, 0, &_globalData.normals[_vertexStart] );
//    glVertexPointer( 3, GL_FLOAT, 0, &_globalData.vertices[_vertexStart] );
//    glDrawElements( GL_TRIANGLES, _indexLength, GL_UNSIGNED_SHORT, 
//                    &_globalData.indices[_indexStart] );
}


/*  Read leaf node from memory.  */
void VertexBufferLeaf::fromMemory( char** addr, VertexBufferData& globalData )
{
    size_t nodeType;
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    if( nodeType != LEAF_TYPE )
        throw MeshException( "Error reading binary file. Expected a leaf "
                             "node, but found something else instead." );
    VertexBufferBase::fromMemory( addr, globalData );
    memRead( reinterpret_cast< char* >( &_vertexStart ), addr, 
             sizeof( Index ) );
    memRead( reinterpret_cast< char* >( &_vertexLength ), addr, 
             sizeof( ShortIndex ) );
    memRead( reinterpret_cast< char* >( &_indexStart ), addr, 
             sizeof( Index ) );
    memRead( reinterpret_cast< char* >( &_indexLength ), addr, 
             sizeof( Index ) );
}


/*  Write leaf node to output stream.  */
void VertexBufferLeaf::toStream( std::ostream& os )
{
    size_t nodeType = LEAF_TYPE;
    os.write( reinterpret_cast< char* >( &nodeType ), sizeof( size_t ) );
    VertexBufferBase::toStream( os );
    os.write( reinterpret_cast< char* >( &_vertexStart ), sizeof( Index ) );
    os.write( reinterpret_cast< char* >( &_vertexLength ), sizeof( ShortIndex ) );
    os.write( reinterpret_cast< char* >( &_indexStart ), sizeof( Index ) );
    os.write( reinterpret_cast< char* >( &_indexLength ), sizeof( Index ) );
}

}
