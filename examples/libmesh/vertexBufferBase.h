
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef MESH_VERTEXBUFFERBASE_H
#define MESH_VERTEXBUFFERBASE_H

#include "splitAxis.h" // member

#include <eq/vmmlib/VMMLib.h>

/** Simple library to organize and render a triangle mesh in a 3D kd-tree. */
namespace mesh
{
    class VertexData;

    /** The base class for tree nodes. */
    class VertexBufferBase
    {
    public:
        const BoundingSphere& getBoundingSphere() const 
            { return _boundingSphere; }

        virtual VertexBuffer* getLeftChild()  { return 0; }
        virtual VertexBuffer* getRightChild() { return 0; }

    protected:
        virtual void setupTree( VertexData& data, const size_t start, 
                                const size_t length, const SplitAxis axis ) = 0;
        virtual BoundingBox updateBoundingSphere() = 0;

    private:
        SplitAxis _axis;
        vmml::Vector4f _boundingSphere;
    };        
}

#endif // MESH_VERTEXBUFFERBASE_H
