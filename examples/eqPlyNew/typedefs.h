/*  
    typedefs.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Type definitions for the mesh classes.
*/


#ifndef MESH_TYPEDEFS_H
#define MESH_TYPEDEFS_H


#include <vmmlib/vmmlib.h>
#include <OpenGL/gl.h>


namespace mesh 
{
    
    
    // basic type definitions   
    typedef vmml::Vector3< GLfloat >    Vertex;
    typedef vmml::Vector4< GLubyte >    Color;
    typedef vmml::Vector3< GLfloat >    Normal;
    typedef size_t                      Index;
    typedef GLushort                    ShortIndex;
    
    
    // wrapper to enable array use where arrays would not be allowed otherwise
    template< class T, size_t d >
    struct ArrayWrapper
    {
        T data[d];
        
        T& operator[]( const size_t i )
        {
            assert( i < d );
            return data[i];
        }
        
        const T& operator[]( const size_t i ) const
        {
            assert( i < d );
            return data[i];
        }
    };
    
    
    // compound type definitions
    typedef vmml::Vector3< Index >          Triangle;
    typedef ArrayWrapper< Vertex, 2 >       BoundingBox;
    typedef vmml::Vector4< GLfloat >        BoundingSphere;
    typedef vmml::FrustumCuller< GLfloat >  Culler;
    typedef ArrayWrapper< float, 2 >        Range;
    
    
    // maximum triangle count per leaf node (keep in mind that the number of
    // different vertices per leaf must stay below ShortIndex range; usually
    // #vertices ~ #triangles/2, but max #vertices = #triangles * 3)
    const Index             LEAF_SIZE( 21845 );
    
    // binary mesh file version
    const unsigned short    FILE_VERSION ( 0x0108 );
    
    
    // enumeration for the sort axis
    enum Axis
    {
        AXIS_X,
        AXIS_Y,
        AXIS_Z
    };
    
    // enumeration for the buffer objects
    enum BufferObject
    {
        VERTEX_OBJECT,
        NORMAL_OBJECT,
        COLOR_OBJECT,
        INDEX_OBJECT
    };
    
    // enumeration for the render modes
    enum RenderMode
    {
        IMMEDIATE_MODE,
        DISPLAY_LIST_MODE,
        VERTEX_ARRAY_MODE,
        BUFFER_OBJECT_MODE
    };
    
    // enumeration for kd-tree node types
    enum NodeType
    {
        ROOT_TYPE = 0x07,
        NODE_TYPE = 0xde,
        LEAF_TYPE = 0xef
    };
    
    
    // helper function for MMF (memory mapped file) reading
    inline
    void memRead( char* destination, char** source, size_t length )
    {
        memcpy( destination, *source, length );
        *source += length;
    }
    
    
}


#endif // MESH_TYPEDEFS_H
