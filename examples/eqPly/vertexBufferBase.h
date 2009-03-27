/*  
    vertexBufferBase.h
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
  
    
    Header file of the abstract VertexBufferBase class.
*/


#ifndef MESH_VERTEXBUFFERBASE_H
#define MESH_VERTEXBUFFERBASE_H


#include "typedefs.h"
#include <fstream>

namespace eqPly
{
    class VertexBufferDist;
}

namespace mesh 
{
    // defined elsewhere
    class VertexData;
    class VertexBufferData;
    class VertexBufferState;
        
    /*  The abstract base class for all kinds of kd-tree nodes.  */
    class VertexBufferBase
    {
    public:
        virtual void render( VertexBufferState& state ) const = 0;
        void renderBoundingSphere( VertexBufferState& state ) const;
        virtual Index getNumberOfVertices() const = 0;

        const BoundingSphere& getBoundingSphere() const 
        {
            return _boundingSphere;
        }
        
        const float* getRange() const
        {
            return &_range[0];
        }
        
        virtual const VertexBufferBase* getLeft() const { return 0; }
        virtual const VertexBufferBase* getRight() const { return 0; }

        virtual const BoundingSphere& updateBoundingSphere() = 0;
                
    protected:
        VertexBufferBase() : _boundingSphere( 0.0f ) 
        {
            _range[0] = 0.0f;
            _range[1] = 1.0f;
        }
        
        virtual ~VertexBufferBase() {};
        
        virtual void toStream( std::ostream& os )
        {
            os.write( reinterpret_cast< char* >( &_boundingSphere ), 
                      sizeof( BoundingSphere ) );
            os.write( reinterpret_cast< char* >( &_range ), 
                      sizeof( Range ) );
        }
        
        virtual void fromMemory( char** addr, VertexBufferData& globalData )
        {
            memRead( reinterpret_cast< char* >( &_boundingSphere ), addr, 
                     sizeof( BoundingSphere ) );
            memRead( reinterpret_cast< char* >( &_range ), addr, 
                     sizeof( Range ) );
        }
        
        virtual void setupTree( VertexData& data, const Index start,
                                const Index length, const Axis axis,
                                const size_t depth,
                                VertexBufferData& globalData ) = 0;
        
        virtual void updateRange() = 0;
        
        BoundingSphere  _boundingSphere;
        Range           _range;
        friend class eqPly::VertexBufferDist;

    private:
    };
}


#endif // MESH_VERTEXBUFFERBASE_H
