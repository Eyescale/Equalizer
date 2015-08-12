
/* Copyright (c) 2007-2015, Tobias Wolf <twolf@access.unizh.ch>
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

#ifndef PLYLIB_VERTEXBUFFERLEAF_H
#define PLYLIB_VERTEXBUFFERLEAF_H

#include "vertexBufferBase.h"

namespace triply
{
/*  The class for kd-tree leaf nodes.  */
class VertexBufferLeaf : public VertexBufferBase
{
public:
    explicit VertexBufferLeaf( VertexBufferData& data )
        : _globalData( data ), _vertexStart( 0 ), _indexStart( 0 )
        , _indexLength( 0 ), _vertexLength( 0 ) {}
    virtual ~VertexBufferLeaf() {}

    virtual void draw( VertexBufferState& state ) const;
    virtual Index getNumberOfVertices() const { return _indexLength; }

protected:
    virtual void toStream( std::ostream& os );
    virtual void fromMemory( char** addr, VertexBufferData& globalData );

    virtual void setupTree( VertexData& data, const Index start,
                            const Index length, const Axis axis,
                            const size_t depth,
                            VertexBufferData& globalData,
                            boost::progress_display& );
    virtual const BoundingSphere& updateBoundingSphere();
    virtual void updateRange();

private:
    void setupRendering( VertexBufferState& state, GLuint* data ) const;
    void renderImmediate( VertexBufferState& state ) const;
    void renderDisplayList( VertexBufferState& state ) const;
    void renderBufferObject( VertexBufferState& state ) const;

    friend class VertexBufferDist;
    VertexBufferData&   _globalData;
    BoundingBox         _boundingBox;
    Index               _vertexStart;
    Index               _indexStart;
    Index               _indexLength;
    ShortIndex          _vertexLength;
};
}

#endif // PLYLIB_VERTEXBUFFERLEAF_H
