/*  
    vertexBufferNode.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
  *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  
    
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
        virtual Index getNumberOfVertices() const
            {return _left->getNumberOfVertices()+_right->getNumberOfVertices();}
        
        virtual const VertexBufferBase* getLeft() const { return _left; }
        virtual const VertexBufferBase* getRight() const { return _right; }
        
    protected:
        virtual void toStream( std::ostream& os );
        virtual void fromMemory( char** addr, VertexBufferData& globalData );
        
        virtual void setupTree( VertexData& data, const Index start,
                                const Index length, const Axis axis,
                                const size_t depth, 
                                VertexBufferData& globalData );
        virtual const BoundingSphere& updateBoundingSphere();
        virtual void updateRange();
                                
    private:
        size_t countUniqueVertices( VertexData& data, const Index start,
                                    const Index length ) const;
        
        VertexBufferBase*   _left;
        VertexBufferBase*   _right;
        friend class eqPly::VertexBufferDist;
    };
}


#endif // MESH_VERTEXBUFFERNODE_H
