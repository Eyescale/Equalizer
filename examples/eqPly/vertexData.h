/*  
    vertexData.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
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
        Axis getLongestAxis( const size_t start, const size_t elements ) const;

        void useInvertedFaces() { _invertFaces = true; }

        std::vector< Vertex >   vertices;
        std::vector< Color >    colors;
        std::vector< Normal >   normals;
        std::vector< Triangle > triangles;

    private:
        void readVertices( PlyFile* file, const int nVertices, 
                           const bool readColors );
        void readTriangles( PlyFile* file, const int nFaces );

        BoundingBox _boundingBox;
        bool        _invertFaces;
    };
}


#endif // MESH_VERTEXDATA_H
