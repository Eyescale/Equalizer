/*  
    vertexBufferLeaf.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
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
  
    
    Header file of the VertexBufferLeaf class.
*/


#ifndef MESH_VERTEXBUFFERLEAF_H
#define MESH_VERTEXBUFFERLEAF_H


#include "vertexBufferBase.h"


namespace mesh 
{
    /*  The class for kd-tree leaf nodes.  */
    class VertexBufferLeaf : public VertexBufferBase
    {
    public:
        VertexBufferLeaf( VertexBufferData& data )
            : _globalData( data ), _vertexStart( 0 ),
              _indexStart( 0 ), _indexLength( 0 ) {}
        virtual ~VertexBufferLeaf() {}
        
        virtual void render( VertexBufferState& state ) const;
        virtual Index getNumberOfVertices() const { return _indexLength; }
        
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
        void setupRendering( VertexBufferState& state, GLuint* data ) const;
        void renderImmediate( VertexBufferState& state ) const;
        void renderDisplayList( VertexBufferState& state ) const;
        void renderBufferObject( VertexBufferState& state ) const;
        
        VertexBufferData&   _globalData;
        Index               _vertexStart;
        ShortIndex          _vertexLength;
        Index               _indexStart;
        Index               _indexLength;
        friend class eqPly::VertexBufferDist;
    };
    
    
}


#endif // MESH_VERTEXBUFFERLEAF_H
