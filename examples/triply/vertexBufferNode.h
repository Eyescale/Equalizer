
/* Copyright (c)      2007, Tobias Wolf <twolf@access.unizh.ch>
 *               2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef PLYLIB_VERTEXBUFFERNODE_H
#define PLYLIB_VERTEXBUFFERNODE_H

#include <triply/api.h>
#include "vertexBufferBase.h"

namespace triply
{
/* The class for regular (non-leaf) kd-tree nodes.  */
class VertexBufferNode : public VertexBufferBase
{
public:
    VertexBufferNode() : _left( 0 ), _right( 0 ) {}
    TRIPLY_API virtual ~VertexBufferNode();

    TRIPLY_API void draw( VertexBufferState& state ) const override;
    Index getNumberOfVertices() const override
        { return _left->getNumberOfVertices()+_right->getNumberOfVertices(); }

    const VertexBufferBase* getLeft() const override { return _left; }
    const VertexBufferBase* getRight() const override { return _right; }
    VertexBufferBase* getLeft() override { return _left; }
    VertexBufferBase* getRight() override { return _right; }

protected:
    TRIPLY_API void toStream( std::ostream& os ) override;
    TRIPLY_API void fromMemory( char** addr, VertexBufferData& globalData )
        override;

    TRIPLY_API void setupTree( VertexData& data, const Index start,
                               const Index length, const Axis axis,
                               const size_t depth,
                               VertexBufferData& globalData,
                               boost::progress_display& ) override;
    TRIPLY_API const BoundingSphere& updateBoundingSphere() override;
    TRIPLY_API void updateRange() override;

private:
    friend class VertexBufferDist;
    VertexBufferBase*   _left;
    VertexBufferBase*   _right;
};
}
#endif // PLYLIB_VERTEXBUFFERNODE_H
