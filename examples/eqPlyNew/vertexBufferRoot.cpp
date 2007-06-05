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
#include <sys/mman.h>


#undef NDEBUG
#include <cassert>


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


/*  Set up and tear down rendering state, delegate rest to node routine.  */
void VertexBufferRoot::render( VertexBufferState& state ) const
{
    // set up
    switch( state.getRenderMode() )
    {
    default:
    case BUFFER_OBJECT_MODE:
    case VERTEX_ARRAY_MODE:
            glEnableClientState( GL_VERTEX_ARRAY );
            glEnableClientState( GL_NORMAL_ARRAY );
            if( state.hasColors() )
                glEnableClientState( GL_COLOR_ARRAY );
    case DISPLAY_LIST_MODE:
    case IMMEDIATE_MODE:
        ;
    }
//    if( !state.hasColors() )
//        glEnable( GL_LIGHTING );
    
    // delegate
    VertexBufferNode::render( state );
    
    // tear down
    switch( state.getRenderMode() )
    {
    default:
    case BUFFER_OBJECT_MODE:
    case VERTEX_ARRAY_MODE:
        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_NORMAL_ARRAY );
        if( state.hasColors() )
            glDisableClientState( GL_COLOR_ARRAY );
    case DISPLAY_LIST_MODE:
    case IMMEDIATE_MODE:
        ;
    }
//    if( !state.hasColors() )
//        glDisable( GL_LIGHTING );
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


/*  Read binary kd-tree representation, construct from ply if unavailable.  */
bool VertexBufferRoot::readFromFile( const char* filename )
{
    bool result = false;
    
    // try to open binary file
    int fd = open( getArchitectureFilename( filename ).c_str(), O_RDONLY );
    if( fd < 0 )
    {
        #ifndef NDEBUG
        cout << "constructing new from ply file" << endl;
        #endif
        
        VertexData data;
        if( data.readPlyFile( filename ) )
        {
            data.calculateNormals();
            data.scale( 2.0f );
            setupTree( data );
            writeToFile( filename );
            return true;
        }
        else
            return false;
    }
    else
    {
        #ifndef NDEBUG
        cout << "reading cached binary representation" << endl;
        #endif
        
        // retrieving file information
        struct stat status;
        fstat( fd, &status );
        
        // create memory mapped file
        char* addr = static_cast< char* >( mmap( 0, status.st_size, PROT_READ, 
                                                 MAP_SHARED, fd, 0 ) );
        if( addr != MAP_FAILED )
        {
            fromMemory( addr );
            result = true;
            munmap( addr, status.st_size );
        }
        
        close( fd );
        return result;
    }
}


/*  Write binary representation of the kd-tree to file.  */
bool VertexBufferRoot::writeToFile( const char* filename )
{
    ofstream output( getArchitectureFilename( filename ).c_str(), 
                     ios::out | ios::binary );
    if( !output )
        return false;
    toStream( output );
    output.close();
    return true;
}


/*  Read root node from memory and continue with other nodes.  */
void VertexBufferRoot::fromMemory( char* start )
{
    char** addr = &start;
    size_t version;
    memRead( reinterpret_cast< char* >( &version ), addr, sizeof( size_t ) );
    assert( version == FILE_VERSION );
    size_t nodeType;
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    assert( nodeType == ROOT_TYPE );
    _data.fromMemory( addr );
    VertexBufferNode::fromMemory( addr, _data );
    memRead( reinterpret_cast< char* >( &nodeType ), addr, sizeof( size_t ) );
    assert( nodeType == ROOT_TYPE );
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
