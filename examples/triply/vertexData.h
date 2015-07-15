
/* Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 *               2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
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


#ifndef PLYLIB_VERTEXDATA_H
#define PLYLIB_VERTEXDATA_H

#include <triply/api.h>
#include "typedefs.h"
#include <vector>


// defined elsewhere
struct PlyFile;

namespace triply
{
    /*  Holds the flat data and offers routines to read, scale and sort it.  */
    class VertexData
    {
    public:
        TRIPLY_API VertexData();

        TRIPLY_API bool readPlyFile( const std::string& file );
        TRIPLY_API void sort( const Index start, const Index length, const Axis axis );
        TRIPLY_API void scale( const float baseSize = 2.0f );
        TRIPLY_API void calculateNormals();
        TRIPLY_API void calculateBoundingBox();
        const BoundingBox& getBoundingBox() const { return _boundingBox; }
        TRIPLY_API Axis getLongestAxis( const size_t start, const size_t elements ) const;

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


#endif // PLYLIB_VERTEXDATA_H
