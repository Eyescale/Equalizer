/*  
    vertexBufferRoot.cpp
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Implementation of the VertexBufferRoot class.
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


using namespace std;
using namespace mesh;


/*  Begin kd-tree setup, go through full range starting with x axis.  */
void VertexBufferRoot::setupTree( VertexData& data )
{
    // data is VertexData, _data is VertexBufferData
    _data.clear();
    VertexBufferNode::setupTree( data, 0, data.triangles.size(), 
                                 AXIS_X, _data );
    VertexBufferNode::updateBoundingSphere();
    VertexBufferNode::updateRange();
}


/*  Set up the common OpenGL state for rendering of all nodes.  */
void VertexBufferRoot::beginRendering( VertexBufferState& state ) const
{
    switch( state.getRenderMode() )
    {
#ifdef GL_ARB_vertex_buffer_object
    case BUFFER_OBJECT_MODE:
        glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_NORMAL_ARRAY );
        if( state.useColors() )
            glEnableClientState( GL_COLOR_ARRAY );
#endif
    case DISPLAY_LIST_MODE:
    case IMMEDIATE_MODE:
    default:
        ;
    }
}


/*  Delegate rendering to node routine.  */
void VertexBufferRoot::render( VertexBufferState& state ) const
{
    VertexBufferNode::render( state );
}


/*  Tear down the common OpenGL state for rendering of all nodes.  */
void VertexBufferRoot::endRendering( VertexBufferState& state ) const
{
    switch( state.getRenderMode() )
    {
#ifdef GL_ARB_vertex_buffer_object
    case BUFFER_OBJECT_MODE:
        glPopClientAttrib();
#endif
    case DISPLAY_LIST_MODE:
    case IMMEDIATE_MODE:
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
    short x = *( reinterpret_cast< short* >( test ) );
    return ( x == 1 );
}


/*  Construct architecture dependent file name.  */
string getArchitectureFilename( const char* filename )
{
    ostringstream oss;
    oss << filename << ( isArchitectureLittleEndian() ? ".le" : ".be" );
    oss << getArchitectureBits() << ".bin";
    return oss.str();    
}


/*  Function extracted out of readFromFile to enhance readability.  */
bool VertexBufferRoot::constructFromPly( const char* filename )
{
    bool result = false;
    
    MESHINFO << "Constructing new from PLY file." << endl;
    
    VertexData data;
    if( data.readPlyFile( filename ) )
    {
        data.calculateNormals();
        data.scale( 2.0f );
        setupTree( data );
        result = writeToFile( filename );
    }
    else
    {
        MESHERROR << "Unable to load PLY file." << endl;
    }
    
    return result;
}


/*  Read binary kd-tree representation, construct from ply if unavailable.  */
bool VertexBufferRoot::readFromFile( const char* filename )
{
    bool result = false;
    
#ifdef WIN32
    // try to open binary file
    HANDLE file = CreateFile( getArchitectureFilename( filename ).c_str(), 
                              GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 
                              FILE_ATTRIBUTE_NORMAL, 0 );
    if( file == INVALID_HANDLE_VALUE )
        return constructFromPly( filename );
    
    MESHINFO << "Reading cached binary representation." << endl;
    
    // create a file mapping
    HANDLE map = CreateFileMapping( file, 0, PAGE_READONLY, 0, 0, filename );
    CloseHandle( file );
    if( !map )
    {
        MESHERROR << "Unable to read binary file, file mapping failed." 
                  << endl;
        return false;
    }
    
    // get a view of the mapping
    char* addr = static_cast< char* >( MapViewOfFile( map, FILE_MAP_READ, 0, 
                                                      0, 0 ) );
    if( addr )
    {
        try
        {
            fromMemory( addr );
            result = true;
        }
        catch( exception& e )
        {
            MESHERROR << "Unable to read binary file, an exception occured:  "
                      << e.what() << endl;
        }
        UnmapViewOfFile( addr );
    }
    else
    {
        MESHERROR << "Unable to read binary file, memory mapping failed."
                  << endl;
    }
    
    CloseHandle( map );
    
#else
    // try to open binary file
    int fd = open( getArchitectureFilename( filename ).c_str(), O_RDONLY );
    if( fd < 0 )
        return constructFromPly( filename );
    
    MESHINFO << "Reading cached binary representation." << endl;
    
    // retrieving file information
    struct stat status;
    fstat( fd, &status );
    
    // create memory mapped file
    char* addr = static_cast< char* >( mmap( 0, status.st_size, PROT_READ, 
                                             MAP_SHARED, fd, 0 ) );
    if( addr != MAP_FAILED )
    {
        try
        {
            fromMemory( addr );
            result = true;
        }
        catch( exception& e )
        {
            MESHERROR << "Unable to read binary file, an exception occured:  "
                      << e.what() << endl;
        }
        munmap( addr, status.st_size );
    }
    else
    {
        MESHERROR << "Unable to read binary file, memory mapping failed."
                  << endl;
    }
    
    close( fd );
#endif
    
    return result;
}


/*  Write binary representation of the kd-tree to file.  */
bool VertexBufferRoot::writeToFile( const char* filename )
{
    bool result = false;
    
    ofstream output( getArchitectureFilename( filename ).c_str(), 
                     ios::out | ios::binary );
    if( output )
    {
        // enable exceptions on stream errors
        output.exceptions( ofstream::failbit | ofstream::badbit );
        try
        {
            toStream( output );
            result = true;
        }
        catch( exception& e )
        {
            MESHERROR << "Unable to write binary file, an exception "
                      << "occured:  " << e.what() << endl;
        }
        output.close();
    }
    else
    {
        MESHERROR << "Unable to create binary file." << endl;
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
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    if( nodeType != ROOT_TYPE )
        throw MeshException( "Error reading binary file. Expected a custom "
                             "EOF marker, but found something else instead." );
}


/*  Write root node to output stream and continue with other nodes.  */
void VertexBufferRoot::toStream( ostream& os )
{
    size_t version = FILE_VERSION;
    os.write( reinterpret_cast< char* >( &version ), sizeof( size_t ) );
    size_t nodeType = ROOT_TYPE;
    os.write( reinterpret_cast< char* >( &nodeType ), sizeof( size_t ) );
    _data.toStream( os );
    VertexBufferNode::toStream( os );
    os.write( reinterpret_cast< char* >( &nodeType ), sizeof( size_t ) );
}
