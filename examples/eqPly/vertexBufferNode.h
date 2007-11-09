/*  
    vertexBufferNode.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Header file of the VertexBufferNode class.
*/


#ifndef MESH_VERTEXBUFFERNODE_H
#define MESH_VERTEXBUFFERNODE_H


#include "vertexBufferBase.h"


namespace mesh 
{
    /*  The class for regular (non-leaf) kd-tree nodes.  */
    class VertexBufferNode : public VertexBufferBase
    {
    public:
        VertexBufferNode() : _left( 0 ), _right( 0 ) {}
        virtual ~VertexBufferNode();
        
        virtual void render( VertexBufferState& state ) const;
        
        virtual const VertexBufferBase* getLeft() const { return _left; }
        virtual const VertexBufferBase* getRight() const { return _right; }
        
    protected:
        virtual void toStream( std::ostream& os );
        virtual void fromMemory( char** addr, VertexBufferData& globalData );
        
        virtual void setupTree( VertexData& data, const Index start,
                                const Index length, const Axis axis,
                                const size_t depth, 
                                VertexBufferData& globalData );
        virtual BoundingBox updateBoundingSphere();
        virtual void updateRange();
                                
    private:
        size_t countUniqueVertices( VertexData& data, const Index start,
                                    const Index length ) const;
        
        VertexBufferBase*   _left;
        VertexBufferBase*   _right;
    };
}


#endif // MESH_VERTEXBUFFERNODE_H
