
/* Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 *               2009, Stefan Eilemann <eile@equalizergraphics.com>
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
        bool _readBinary( std::string filename );
        
        VertexBufferData _data;
        bool             _invertFaces;
        std::string      _name;

        friend class eqPly::VertexBufferDist;
    };
    
    
}


#endif // MESH_VERTEXBUFFERROOT_H
