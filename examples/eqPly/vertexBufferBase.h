
/* Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 * Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
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
