
/* Copyright (c) 2007-2016, Tobias Wolf <twolf@access.unizh.ch>
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


#include "vertexBufferRoot.h"
#include "vertexBufferState.h"
#include "vertexData.h"
#include <string>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#   include <sys/mman.h>
#endif

namespace triply
{

using vmml::FrustumCullerf;

/*  Determine number of bits used by the current architecture.  */
size_t getArchitectureBits();
/*  Determine whether the current architecture is little endian or not.  */
bool isArchitectureLittleEndian();
/*  Construct architecture dependent file name.  */
std::string getArchitectureFilename( const std::string& filename );

/*  Begin kd-tree setup, go through full range starting with x axis.  */
void VertexBufferRoot::setupTree( VertexData& data,
                                  boost::progress_display& progress )
{
    // data is VertexData, _data is VertexBufferData
    _data.clear();

    const Axis axis = data.getLongestAxis( 0, data.triangles.size() );

    VertexBufferNode::setupTree( data, 0, data.triangles.size(),
                                 axis, 0, _data, progress );
    VertexBufferNode::updateBoundingSphere();
    VertexBufferNode::updateRange();
}

// #define LOGCULL
void VertexBufferRoot::cullDraw( VertexBufferState& state ) const
{
    _beginRendering( state );

#ifdef LOGCULL
    size_t verticesRendered = 0;
    size_t verticesOverlap  = 0;
#endif

    const Range& range = state.getRange();
    const FrustumCullerf culler( state.getProjectionModelViewMatrix( ));

    // start with root node
    std::vector< const triply::VertexBufferBase* > candidates;
    candidates.push_back( this );

    while( !candidates.empty() )
    {
        if( state.stopRendering( ))
            return;

        const triply::VertexBufferBase* treeNode = candidates.back();
        candidates.pop_back();

        // completely out of range check
        if( treeNode->getRange()[0] >= range[1] ||
            treeNode->getRange()[1] < range[0] )
        {
            continue;
        }

        // bounding sphere view frustum culling
        const vmml::Visibility visibility = state.useFrustumCulling() ?
                            culler.test( treeNode->getBoundingSphere( )) :
                            vmml::VISIBILITY_FULL;
        switch( visibility )
        {
            case vmml::VISIBILITY_FULL:
                // if fully visible and fully in range, render it
                if( treeNode->getRange()[0] >= range[0] &&
                    treeNode->getRange()[1] <  range[1] )
                {
                    treeNode->draw( state );
                    //treeNode->drawBoundingSphere( state );
#ifdef LOGCULL
                    verticesRendered += treeNode->getNumberOfVertices();
#endif
                    break;
                }
                // partial range, fall through to partial visibility

            case vmml::VISIBILITY_PARTIAL:
            {
                const triply::VertexBufferBase* left  = treeNode->getLeft();
                const triply::VertexBufferBase* right = treeNode->getRight();

                if( !left && !right )
                {
                    if( treeNode->getRange()[0] >= range[0] )
                    {
                        treeNode->draw( state );
                        //treeNode->drawBoundingSphere( state );
#ifdef LOGCULL
                        verticesRendered += treeNode->getNumberOfVertices();
                        if( visibility == vmml::VISIBILITY_PARTIAL )
                            verticesOverlap  += treeNode->getNumberOfVertices();
#endif
                    }
                    // else drop, to be drawn by 'previous' channel
                }
                else
                {
                    if( left )
                        candidates.push_back( left );
                    if( right )
                        candidates.push_back( right );
                }
                break;
            }
            case vmml::VISIBILITY_NONE:
                // do nothing
                break;
        }
    }

    _endRendering( state );

#ifdef LOGCULL
    const size_t verticesTotal = model->getNumberOfVertices();
    PLYLIBINFO
        << getName() << " rendered " << verticesRendered * 100 / verticesTotal
        << "% of model, overlap <= " << verticesOverlap * 100 / verticesTotal
        << "%" << std::endl;
#endif
}


/*  Set up the common OpenGL state for rendering of all nodes.  */
void VertexBufferRoot::_beginRendering( VertexBufferState& state ) const
{
    state.resetRegion();
    switch( state.getRenderMode() )
    {
#ifdef GL_ARB_vertex_buffer_object
    case RENDER_MODE_BUFFER_OBJECT:
        glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_NORMAL_ARRAY );
        if( state.useColors() )
            glEnableClientState( GL_COLOR_ARRAY );
#endif
    case RENDER_MODE_DISPLAY_LIST:
    case RENDER_MODE_IMMEDIATE:
    default:
        ;
    }
}


/*  Delegate rendering to node routine.  */
void VertexBufferRoot::draw( VertexBufferState& state ) const
{
    VertexBufferNode::draw( state );
}


/*  Tear down the common OpenGL state for rendering of all nodes.  */
void VertexBufferRoot::_endRendering( VertexBufferState& state ) const
{
    switch( state.getRenderMode() )
    {
#ifdef GL_ARB_vertex_buffer_object
    case RENDER_MODE_BUFFER_OBJECT:
    {
        // deactivate VBO and EBO use
#define glewGetContext state.glewGetContext
        glBindBuffer( GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        glPopClientAttrib();
    }
#endif
    case RENDER_MODE_DISPLAY_LIST:
    case RENDER_MODE_IMMEDIATE:
    default:
        ;
    }
}


/*  Determine number of bits used by the current architecture.  */
size_t getArchitectureBits()
{
    return ( sizeof( void* ) * 8 );
}


/*  Determine whether the current architecture is little endian or not.  */
bool isArchitectureLittleEndian()
{
    unsigned char test[2] = { 1, 0 };
    short* x = reinterpret_cast< short* >( test );
    return ( *x == 1 );
}


/*  Construct architecture dependent file name.  */
std::string getArchitectureFilename( const std::string& filename )
{
    std::ostringstream oss;
    oss << filename << ( isArchitectureLittleEndian() ? ".le" : ".be" );
    oss << getArchitectureBits() << ".bin";
    return oss.str();
}


/*  Functions extracted out of readFromFile to enhance readability.  */
bool VertexBufferRoot::_constructFromPly( const std::string& filename )
{
    PLYLIBINFO << "Reading PLY file." << std::endl;
    boost::progress_display progress( 12 );

    VertexData data;
    if( _invertFaces )
        data.useInvertedFaces();
    if( !data.readPlyFile( filename ) )
    {
        PLYLIBERROR << "Unable to load PLY file." << std::endl;
        return false;
    }
    ++progress;

    data.calculateNormals();
    data.scale( 2.0f );
    ++progress;

    setupTree( data, progress );
    ++progress;
    if( !writeToFile( filename ))
        PLYLIBWARN << "Unable to write binary representation." << std::endl;

    ++progress;
    return true;
}

bool VertexBufferRoot::_readBinary( std::string filename )
{
#ifdef WIN32

    // replace dir delimiters since '\' is often used as escape char
    for( size_t i=0; i<filename.length(); ++i )
        if( filename[i] == '\\' )
            filename[i] = '/';

    // try to open binary file
    HANDLE file = CreateFile( filename.c_str(), GENERIC_READ, FILE_SHARE_READ,
                              0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    if( file == INVALID_HANDLE_VALUE )
        return false;

    PLYLIBINFO << "Reading cached binary representation." << std::endl;

    // create a file mapping
    HANDLE map = CreateFileMapping( file, 0, PAGE_READONLY, 0, 0,
                                    filename.c_str( ));
    CloseHandle( file );
    if( !map )
    {
        PLYLIBERROR << "Unable to read binary file, file mapping failed."
                  << std::endl;
        return false;
    }

    // get a view of the mapping
    char* addr   = static_cast< char* >( MapViewOfFile( map, FILE_MAP_READ, 0,
                                                        0, 0 ) );
    bool  result = false;

    if( addr )
    {
        try
        {
            fromMemory( addr );
            result = true;
        }
        catch( const std::exception& e )
        {
            PLYLIBERROR << "Unable to read binary file, an exception occured:  "
                      << e.what() << std::endl;
        }
        UnmapViewOfFile( addr );
    }
    else
    {
        PLYLIBERROR << "Unable to read binary file, memory mapping failed."
                  << std::endl;
        return false;
    }

    CloseHandle( map );
    return result;

#else
    // try to open binary file
    int fd = open( filename.c_str(), O_RDONLY );
    if( fd < 0 )
        return false;

    // retrieving file information
    struct stat status;
    fstat( fd, &status );

    // create memory mapped file
    char* addr   = static_cast< char* >( mmap( 0, status.st_size, PROT_READ,
                                               MAP_SHARED, fd, 0 ) );
    bool  result = false;
    if( addr != MAP_FAILED )
    {
        try
        {
            fromMemory( addr );
            result = true;
        }
        catch( const std::exception& e )
        {
            PLYLIBERROR << "Unable to read binary file, an exception occured:  "
                      << e.what() << std::endl;
        }
        munmap( addr, status.st_size );
    }
    else
    {
        PLYLIBERROR << "Unable to read binary file, memory mapping failed."
                  << std::endl;
    }

    close( fd );
    return result;
#endif
}

/*  Read binary kd-tree representation, construct from ply if unavailable.  */
bool VertexBufferRoot::readFromFile( const std::string& filename )
{
    if( _readBinary( getArchitectureFilename( filename )))
    {
        _name = filename;
        return true;
    }
    if( _constructFromPly( filename ))
    {
        _name = filename;
        return true;
    }
    return false;
}

/*  Write binary representation of the kd-tree to file.  */
bool VertexBufferRoot::writeToFile( const std::string& filename )
{
    bool result = false;

    std::ofstream output( getArchitectureFilename( filename ).c_str(),
                          std::ios::out | std::ios::binary );
    if( output )
    {
        // enable exceptions on stream errors
        output.exceptions( std::ofstream::failbit | std::ofstream::badbit );
        try
        {
            toStream( output );
            result = true;
        }
        catch( const std::exception& e )
        {
            PLYLIBERROR << "Unable to write binary file, an exception "
                      << "occured:  " << e.what() << std::endl;
        }
        output.close();
    }
    else
    {
        PLYLIBERROR << "Unable to create binary file." << std::endl;
    }

    return result;
}


/*  Read root node from memory and continue with other nodes.  */
void VertexBufferRoot::fromMemory( char* start )
{
    char** addr = &start;
    size_t version;
    memRead( reinterpret_cast< char* >( &version ), addr, sizeof( size_t ) );
    if( version != FILE_VERSION )
        throw MeshException( "Error reading binary file. Version in file "
                             "does not match the expected version." );
    size_t nodeType;
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    if( nodeType != ROOT_TYPE )
        throw MeshException( "Error reading binary file. Expected the root "
                             "node, but found something else instead." );
    _data.fromMemory( addr );
    VertexBufferNode::fromMemory( addr, _data );
}


/*  Write root node to output stream and continue with other nodes.  */
void VertexBufferRoot::toStream( std:: ostream& os )
{
    size_t version = FILE_VERSION;
    os.write( reinterpret_cast< char* >( &version ), sizeof( size_t ) );
    size_t nodeType = ROOT_TYPE;
    os.write( reinterpret_cast< char* >( &nodeType ), sizeof( size_t ) );
    _data.toStream( os );
    VertexBufferNode::toStream( os );
}

}
