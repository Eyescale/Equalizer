
/* Copyright (c) 2007-2015, Tobias Wolf <twolf@access.unizh.ch>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
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


#include "vertexBufferLeaf.h"
#include "vertexBufferData.h"
#include "vertexBufferState.h"
#include "vertexData.h"
#include <map>

namespace triply
{

/*  Finish partial setup - sort, reindex and merge into global data.  */
void VertexBufferLeaf::setupTree( VertexData& data, const Index start,
                                  const Index length, const Axis axis,
                                  const size_t depth,
                                  VertexBufferData& globalData,
                                  boost::progress_display& progress )
{
    data.sort( start, length, axis );
    _vertexStart = globalData.vertices.size();
    _vertexLength = 0;
    _indexStart = globalData.indices.size();
    _indexLength = 0;

    const bool hasColors = !data.colors.empty();

    // stores the new indices (relative to _start)
    std::map< Index, ShortIndex > newIndex;

    for( Index t = 0; t < length; ++t )
    {
        for( Index v = 0; v < 3; ++v )
        {
            Index i = data.triangles[start + t][v];
            if( newIndex.find( i ) == newIndex.end() )
            {
                newIndex[i] = _vertexLength++;
                // assert number of vertices does not exceed SmallIndex range
                PLYLIBASSERT( _vertexLength );
                globalData.vertices.push_back( data.vertices[i] );
                if( hasColors )
                    globalData.colors.push_back( data.colors[i] );
                globalData.normals.push_back( data.normals[i] );
            }
            globalData.indices.push_back( newIndex[i] );
            ++_indexLength;
        }
    }
    if( depth == 3 )
        ++progress;
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
    _boundingBox[0] = _globalData.vertices[ _vertexStart +
                                            _globalData.indices[_indexStart] ];
    _boundingBox[1] = _globalData.vertices[ _vertexStart +
                                            _globalData.indices[_indexStart] ];

    for( Index i = 1 + _indexStart; i < _indexStart + _indexLength; ++i )
    {
        const Vertex& vertex = _globalData.vertices[ _vertexStart +
                                                     _globalData.indices[ i ] ];
        _boundingBox[0][0] = std::min( _boundingBox[0][0], vertex[0] );
        _boundingBox[1][0] = std::max( _boundingBox[1][0], vertex[0] );
        _boundingBox[0][1] = std::min( _boundingBox[0][1], vertex[1] );
        _boundingBox[1][1] = std::max( _boundingBox[1][1], vertex[1] );
        _boundingBox[0][2] = std::min( _boundingBox[0][2], vertex[2] );
        _boundingBox[1][2] = std::max( _boundingBox[1][2], vertex[2] );
    }

    // 1b) get inner sphere of bounding box as an initial estimate
    _boundingSphere.x() = ( _boundingBox[0].x() + _boundingBox[1].x() ) * 0.5f;
    _boundingSphere.y() = ( _boundingBox[0].y() + _boundingBox[1].y() ) * 0.5f;
    _boundingSphere.z() = ( _boundingBox[0].z() + _boundingBox[1].z() ) * 0.5f;

    _boundingSphere.w()  = std::max( _boundingBox[1].x() - _boundingBox[0].x(),
                                     _boundingBox[1].y() - _boundingBox[0].y());
    _boundingSphere.w()  = std::max( _boundingBox[1].z() - _boundingBox[0].z(),
                                     _boundingSphere.w( ));
    _boundingSphere.w() *= .5f;

    float  radius        = _boundingSphere.w();
    float  radiusSquared =  radius * radius;
    Vertex center( _boundingSphere.array );

    // 2) test all points to be in the estimated bounding sphere
    for( Index offset = 0; offset < _indexLength; ++offset )
    {
        const Vertex& vertex =
            _globalData.vertices[ _vertexStart +
                                  _globalData.indices[_indexStart + offset] ];

        const Vertex centerToPoint   = vertex - center;
        const float  distanceSquared = centerToPoint.squared_length();
        if( distanceSquared <= radiusSquared ) // point is inside existing BS
            continue;

        // 3) expand sphere to contain 'outside' points
        const float distance = sqrtf( distanceSquared );
        const float delta    = distance - radius;

        radius        = ( radius + distance ) * .5f;
        radiusSquared = radius * radius;
        const Vertex normdelta = normalize( centerToPoint ) * ( 0.5f * delta );

        center       += normdelta;

        LBASSERTINFO( Vertex( vertex-center ).squared_length() <=
                ( radiusSquared + 2.f * std::numeric_limits<float>::epsilon( )),
                      vertex << " c " << center << " r " << radius << " ("
                             << Vertex( vertex-center ).length() << ")" );
    }

#ifndef NDEBUG
    // 2a) re-test all points to be in the estimated bounding sphere
    for( Index offset = 0; offset < _indexLength; ++offset )
    {
        const Vertex& vertex =
            _globalData.vertices[ _vertexStart +
                                  _globalData.indices[_indexStart + offset] ];

        const Vertex centerToPoint   = vertex - center;
        const float  distanceSquared = centerToPoint.squared_length();
        LBASSERTINFO( distanceSquared <=
                ( radiusSquared + 2.f * std::numeric_limits<float>::epsilon( )),
                      vertex << " c " << center << " r " << radius << " ("
                             << Vertex( vertex-center ).length() << ")" );
    }
#endif

    // store optimal bounding sphere
    _boundingSphere.x() = center.x();
    _boundingSphere.y() = center.y();
    _boundingSphere.z() = center.z();
    _boundingSphere.w() = radius;
    return _boundingSphere;
}


/*  Compute the range of this child.  */
void VertexBufferLeaf::updateRange()
{
    _range[0] = 1.0f * _indexStart / _globalData.indices.size();
    _range[1] = _range[0] + 1.0f * _indexLength / _globalData.indices.size();
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
        const char* charThis = reinterpret_cast< const char* >( this );

        if( data[VERTEX_OBJECT] == state.INVALID )
            data[VERTEX_OBJECT] = state.newBufferObject( charThis + 0 );
        glBindBuffer( GL_ARRAY_BUFFER, data[VERTEX_OBJECT] );
        glBufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Vertex ),
                        &_globalData.vertices[_vertexStart], GL_STATIC_DRAW );

        if( data[NORMAL_OBJECT] == state.INVALID )
            data[NORMAL_OBJECT] = state.newBufferObject( charThis + 1 );
        glBindBuffer( GL_ARRAY_BUFFER, data[NORMAL_OBJECT] );
        glBufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Normal ),
                        &_globalData.normals[_vertexStart], GL_STATIC_DRAW );

        if( data[COLOR_OBJECT] == state.INVALID )
            data[COLOR_OBJECT] = state.newBufferObject( charThis + 2 );
        if( state.useColors() )
        {
            glBindBuffer( GL_ARRAY_BUFFER, data[COLOR_OBJECT] );
            glBufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Color ),
                            &_globalData.colors[_vertexStart], GL_STATIC_DRAW );
        }

        if( data[INDEX_OBJECT] == state.INVALID )
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
        if( data[0] == state.INVALID )
        {
            char* key = (char*)( this );
            if( state.useColors( ))
                ++key;
            data[0] = state.newDisplayList( key );
        }
        glNewList( data[0], GL_COMPILE );
        renderImmediate( state );
        glEndList();
        break;
    }
    }
}

/*  Draw the leaf.  */
void VertexBufferLeaf::draw( VertexBufferState& state ) const
{
    if( state.stopRendering( ))
        return;

    state.updateRegion( _boundingBox );
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
    if( buffers[VERTEX_OBJECT] == state.INVALID ||
        buffers[NORMAL_OBJECT] == state.INVALID ||
        buffers[COLOR_OBJECT] == state.INVALID ||
        buffers[INDEX_OBJECT] == state.INVALID )

        setupRendering( state, buffers );

    if( state.useColors() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, buffers[COLOR_OBJECT] );
        glColorPointer( 3, GL_UNSIGNED_BYTE, 0, 0 );
    }
    glBindBuffer( GL_ARRAY_BUFFER, buffers[NORMAL_OBJECT] );
    glNormalPointer( GL_FLOAT, 0, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, buffers[VERTEX_OBJECT] );
    glVertexPointer( 3, GL_FLOAT, 0, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_OBJECT] );
    glDrawElements( GL_TRIANGLES, GLsizei(_indexLength), GL_UNSIGNED_SHORT, 0 );
}


/*  Render the leaf with a display list.  */
inline
void VertexBufferLeaf::renderDisplayList( VertexBufferState& state ) const
{
    char* key = (char*)( this );
    if( state.useColors( ))
        ++key;

    GLuint displayList = state.getDisplayList( key );

    if( displayList == state.INVALID )
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
            glColor3ubv( &_globalData.colors[i][0] );
        glNormal3fv( &_globalData.normals[i][0] );
        glVertex3fv( &_globalData.vertices[i][0] );
    }
    glEnd();
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
    memRead( reinterpret_cast< char* >( &_boundingBox ), addr,
             sizeof( BoundingBox ) );
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
    os.write( reinterpret_cast< char* >( &nodeType ), sizeof( size_t ));
    VertexBufferBase::toStream( os );
    os.write( reinterpret_cast< char* >( &_boundingBox ), sizeof( BoundingBox));
    os.write( reinterpret_cast< char* >( &_vertexStart ), sizeof( Index ));
    os.write( reinterpret_cast< char* >( &_vertexLength ),sizeof( ShortIndex ));
    os.write( reinterpret_cast< char* >( &_indexStart ), sizeof( Index ));
    os.write( reinterpret_cast< char* >( &_indexLength ), sizeof( Index ));
}

}
