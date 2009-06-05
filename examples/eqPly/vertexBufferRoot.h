/*  
    vertexBufferRoot.h
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
  
    
    Header file of the VertexBufferRoot class.
*/


#ifndef MESH_VERTEXBUFFERROOT_H
#define MESH_VERTEXBUFFERROOT_H


#include "vertexBufferNode.h"
#include "vertexBufferData.h"


namespace mesh 
{
    
    
    /*  The class for kd-tree root nodes.  */
    class VertexBufferRoot : public VertexBufferNode
    {
    public:
        VertexBufferRoot() : VertexBufferNode(), _invertFaces(false) {}

        virtual void render( VertexBufferState& state ) const;
        
        void beginRendering( VertexBufferState& state ) const;
        void endRendering( VertexBufferState& state ) const;
        
        void setupTree( VertexData& data );
        bool writeToFile( const char* filename );
        bool readFromFile( const char* filename );
        bool hasColors() const { return _data.colors.size() > 0; }

        void useInvertedFaces() { _invertFaces = true; }

    protected:
        virtual void toStream( std::ostream& os );
        virtual void fromMemory( char* start );
        
    private:
        bool _constructFromPly( const char* filename );
        bool _readBinary( const char* filename );
        
        VertexBufferData _data;
        bool             _invertFaces;
        friend class eqPly::VertexBufferDist;
    };
    
    
}


#endif // MESH_VERTEXBUFFERROOT_H
