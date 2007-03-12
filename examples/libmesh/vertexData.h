
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef MESH_VERTEXDATA_H
#define MESH_VERTEXDATA_H

#include "splitAxis.h" // member

#include <eq/vmmlib/VMMLib.h>
#include <vector>

namespace mesh
{
    typedef vmml::Vector3< size_t > Triangle;

    /** Unsorted input vertex data. */
    class VertexData
    {
    public:
        std::vector< vmml::Vector3f >   vertices;
        std::vector< vmml::Vector3f >   normals;
        std::vector< vmml::Vector4ub >  color;
        std::vector< Triangle >         triangles;

        void sort( const size_t start, const size_t length, 
                   const SplitAxis axis );
    };
}

#endif // MESH_VERTEXDATA_H
