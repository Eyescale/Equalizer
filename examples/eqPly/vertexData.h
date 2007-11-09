/*  
    vertexData.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Header file of the VertexData class.
*/


#ifndef MESH_VERTEXDATA_H
#define MESH_VERTEXDATA_H


#include "typedefs.h"
#include <vector>


// defined elsewhere
class PlyFile;

namespace mesh 
{
    /*  Holds the flat data and offers routines to read, scale and sort it.  */
    class VertexData
    {
    public:
        VertexData();
        
        bool readPlyFile( const char* file, const bool ignoreColors = false );
        void sort( const Index start, const Index length, const Axis axis );
        void scale( const float baseSize = 2.0f );
        void calculateNormals( const bool vertexNormals = true );
        void calculateBoundingBox();
        const BoundingBox& getBoundingBox() const { return _boundingBox; }
        
        std::vector< Vertex >   vertices;
        std::vector< Color >    colors;
        std::vector< Normal >   normals;
        std::vector< Triangle > triangles;
        
    private:
        void readVertices( PlyFile* file, const int nVertices, 
                           const bool readColors );
        void readTriangles( PlyFile* file, const int nFaces );
        
        BoundingBox _boundingBox;
    };
}


#endif // MESH_VERTEXDATA_H
