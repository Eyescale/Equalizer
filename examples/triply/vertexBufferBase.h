
/* Copyright (c) 2007-2017, Tobias Wolf <twolf@access.unizh.ch>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef PLYLIB_VERTEXBUFFERBASE_H
#define PLYLIB_VERTEXBUFFERBASE_H

#include "typedefs.h"
#include <fstream>
#include <triply/api.h>

namespace triply
{
/*  The abstract base class for all kinds of kd-tree nodes.  */
class VertexBufferBase
{
public:
    virtual ~VertexBufferBase(){};

    TRIPLY_API virtual void draw(VertexBufferState& state) const = 0;
    TRIPLY_API void drawBoundingSphere(VertexBufferState& state) const;
    TRIPLY_API virtual Index getNumberOfVertices() const = 0;

    const BoundingBox& getBoundingBox() const { return _boundingBox; }
    const float* getRange() const { return &_range[0]; }
    virtual const VertexBufferBase* getLeft() const { return nullptr; }
    virtual const VertexBufferBase* getRight() const { return nullptr; }
    virtual VertexBufferBase* getLeft() { return nullptr; }
    virtual VertexBufferBase* getRight() { return nullptr; }
    TRIPLY_API virtual void updateBounds() = 0;

protected:
    VertexBufferBase()
    {
        _range[0] = 0.0f;
        _range[1] = 1.0f;
    }

    virtual void toStream(std::ostream& os)
    {
        os.write(reinterpret_cast<char*>(&_boundingBox), sizeof(BoundingBox));
        os.write(reinterpret_cast<char*>(&_range), sizeof(Range));
    }

    virtual void fromMemory(char** addr, VertexBufferData& /*globalData*/)
    {
        memRead(reinterpret_cast<char*>(&_boundingBox), addr,
                sizeof(_boundingBox));
        memRead(reinterpret_cast<char*>(&_range), addr, sizeof(Range));
    }

    friend class VertexBufferNode;
    virtual void setupTree(VertexData& data, Index start, Index length,
                           Axis axis, size_t depth,
                           VertexBufferData& globalData,
                           boost::progress_display&) = 0;

    virtual void updateRange() = 0;

    friend class VertexBufferDist;
    virtual Type getType() const = 0;

    BoundingBox _boundingBox;
    Range _range;
};
}

#endif // PLYLIB_VERTEXBUFFERBASE_H
