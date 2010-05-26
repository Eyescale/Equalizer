
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

        VertexBufferBase*   _left;
        VertexBufferBase*   _right;
        friend class eqPly::VertexBufferDist;
    };
}


#endif // MESH_VERTEXBUFFERNODE_H
