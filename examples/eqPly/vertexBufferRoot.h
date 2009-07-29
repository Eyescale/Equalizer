/*  
 *  Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 *                2009, Stefan Eilemann <eile@equalizergraphics.com>
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
        bool writeToFile( const std::string& filename );
        bool readFromFile( const std::string& filename );
        bool hasColors() const { return _data.colors.size() > 0; }

        void useInvertedFaces() { _invertFaces = true; }

        const std::string& getName() const { return _name; }

    protected:
        virtual void toStream( std::ostream& os );
        virtual void fromMemory( char* start );
        
    private:
        bool _constructFromPly( const std::string& filename );
        bool _readBinary( const std::string& filename );
        
        VertexBufferData _data;
        bool             _invertFaces;
        std::string      _name;

        friend class eqPly::VertexBufferDist;
    };
    
    
}


#endif // MESH_VERTEXBUFFERROOT_H
