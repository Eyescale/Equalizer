/*  
    vertexBufferLeaf.cpp
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Implementation of the VertexBufferLeaf class.
*/


#include "vertexBufferLeaf.h"
#include "vertexBufferData.h"
#include "vertexBufferState.h"
#include "vertexData.h"
#include <map>

using namespace std;
using namespace mesh;


/*  Finish partial setup - sort, reindex and merge into global data.  */
void VertexBufferLeaf::setupTree( VertexData& data, const Index start,
                                  const Index length, const Axis axis,
                                  VertexBufferData& globalData )
{
    #ifndef NDEBUG
    MESHINFO << "Entering VertexBufferLeaf::setupTree"
             << "( " << start << ", " << length << ", " << axis << " )." 
             << endl;
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
BoundingBox VertexBufferLeaf::updateBoundingSphere()
{
    // first initialize and compute a bounding box
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
        for( Index i = 0; i < 3; ++i )
        {
            boundingBox[0][i] = min( boundingBox[0][i], vertex[i] );
            boundingBox[1][i] = max( boundingBox[1][i], vertex[i] );
        } 
    }
    
    // now compute the bounding sphere around the bounding box
    calculateBoundingSphere( boundingBox );
    
    #ifndef NDEBUG
    MESHINFO << "Exiting VertexBufferLeaf::updateBoundingSphere" 
             << "( " << boundingBox[0] << ", " << boundingBox[1] << " )." 
             << endl;
    #endif
    
    return boundingBox;
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


/*  Set up rendering of the leaf nodes.  */
void VertexBufferLeaf::setupRendering( VertexBufferState& state ) const
{
    if( _isSetup )
        return;
    
    const GLFunctions* gl = state.getGLFunctions();
    
    switch( state.getRenderMode() )
    {
    case IMMEDIATE_MODE:
        break;
        
    case BUFFER_OBJECT_MODE:
    {
        MESHINFO << "Setting up VBO rendering for leaf " << this << "." << endl;
        GLuint buffers[4];
        
        buffers[VERTEX_OBJECT] = 
            state.newBufferObject( reinterpret_cast< const char* >( this ) + 0 );
        gl->bindBuffer( GL_ARRAY_BUFFER, buffers[VERTEX_OBJECT] );
        gl->bufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Vertex ),
                       &_globalData.vertices[_vertexStart], GL_STATIC_DRAW );
        
        buffers[NORMAL_OBJECT] = 
            state.newBufferObject( reinterpret_cast< const char* >( this ) + 1 );
        gl->bindBuffer( GL_ARRAY_BUFFER, buffers[NORMAL_OBJECT] );
        gl->bufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Normal ),
                       &_globalData.normals[_vertexStart], GL_STATIC_DRAW );
        
        buffers[COLOR_OBJECT] = 
            state.newBufferObject( reinterpret_cast< const char* >( this ) + 2 );
        if( state.useColors() )
        {
            gl->bindBuffer( GL_ARRAY_BUFFER, buffers[COLOR_OBJECT] );
            gl->bufferData( GL_ARRAY_BUFFER, _vertexLength * sizeof( Color ),
                           &_globalData.colors[_vertexStart], GL_STATIC_DRAW );
        }
        
        buffers[INDEX_OBJECT] = 
            state.newBufferObject( reinterpret_cast< const char* >( this ) + 3 );
        gl->bindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_OBJECT] );
        gl->bufferData( GL_ELEMENT_ARRAY_BUFFER, 
                       _indexLength * sizeof( ShortIndex ),
                       &_globalData.indices[_indexStart], GL_STATIC_DRAW );
        
        break;
    }        
    case DISPLAY_LIST_MODE:
    default:
    {
        MESHINFO << "Setting up display list rendering for leaf " << this
                 << "." << endl;
        GLuint displayList = state.newDisplayList( this );
        glNewList( displayList, GL_COMPILE );
        renderImmediate( state );
        glEndList();
        break;
    }
    }

    MESHINFO << "Leaf " << this << " contains " << _vertexLength << " vertices"
             << " and " << _indexLength / 3 << " triangles." << endl;
    
    _isSetup = true;
}


/*  Render the leaf.  */
void VertexBufferLeaf::render( VertexBufferState& state ) const
{
    if( !_isSetup )
        setupRendering( state );
    
    switch( state.getRenderMode() )
    {
    case IMMEDIATE_MODE:
        renderImmediate( state );
        return;
    case BUFFER_OBJECT_MODE:
        renderBufferObject( state );
        return;
    case DISPLAY_LIST_MODE:
    default:
        renderDisplayList( state );
        return;
    }
}


/*  Render the leaf with buffer objects.  */
inline
void VertexBufferLeaf::renderBufferObject( VertexBufferState& state ) const
{
    const GLFunctions* gl = state.getGLFunctions();
    GLuint buffers[4];
    for( int i = 0; i < 4; ++i )
        buffers[i] = 
            state.getBufferObject( reinterpret_cast< const char* >( this ) + i );
    
    if( state.useColors() )
    {
        gl->bindBuffer( GL_ARRAY_BUFFER, buffers[COLOR_OBJECT] );
        glColorPointer( 4, GL_UNSIGNED_BYTE, 0, 0 );
    }
    gl->bindBuffer( GL_ARRAY_BUFFER, buffers[NORMAL_OBJECT] );
    glNormalPointer( GL_FLOAT, 0, 0 );
    gl->bindBuffer( GL_ARRAY_BUFFER, buffers[VERTEX_OBJECT] );
    glVertexPointer( 3, GL_FLOAT, 0, 0 );
    gl->bindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_OBJECT] );
    glDrawElements( GL_TRIANGLES, _indexLength, GL_UNSIGNED_SHORT, 0 );
}


/*  Render the leaf with a display list.  */
inline
void VertexBufferLeaf::renderDisplayList( VertexBufferState& state ) const
{
    GLuint displayList = state.getDisplayList( this );
    glCallList( displayList );
}


/*  Render the leaf with immediate mode primitives or vertex arrays.  */
inline
void VertexBufferLeaf::renderImmediate( VertexBufferState& state ) const
{
    glBegin( GL_TRIANGLES );  
    for( Index offset = 0; offset < _indexLength; ++offset )
    {
        Index i = _vertexStart + _globalData.indices[_indexStart + offset];
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
void VertexBufferLeaf::toStream( ostream& os )
{
    size_t nodeType = LEAF_TYPE;
    os.write( reinterpret_cast< char* >( &nodeType ), sizeof( size_t ) );
    VertexBufferBase::toStream( os );
    os.write( reinterpret_cast< char* >( &_vertexStart ), sizeof( Index ) );
    os.write( reinterpret_cast< char* >( &_vertexLength ), sizeof( ShortIndex ) );
    os.write( reinterpret_cast< char* >( &_indexStart ), sizeof( Index ) );
    os.write( reinterpret_cast< char* >( &_indexLength ), sizeof( Index ) );
}
