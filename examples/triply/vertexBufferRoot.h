
/* Copyright (c)      2007, Tobias Wolf <twolf@access.unizh.ch>
 *               2009-2014, Stefan Eilemann <eile@equalizergraphics.com>
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


#ifndef PLYLIB_VERTEXBUFFERROOT_H
#define PLYLIB_VERTEXBUFFERROOT_H

#include <triply/api.h>
#include "vertexBufferData.h"
#include "vertexBufferNode.h"

namespace triply
{
/*  The class for kd-tree root nodes.  */
class VertexBufferRoot : public VertexBufferNode
{
public:
    TRIPLY_API VertexBufferRoot() : VertexBufferNode(), _invertFaces(false) {}

    TRIPLY_API virtual void cullDraw( VertexBufferState& state ) const;
    TRIPLY_API virtual void draw( VertexBufferState& state ) const;

    TRIPLY_API void setupTree( VertexData& data, boost::progress_display&  );
    TRIPLY_API bool writeToFile( const std::string& filename );
    TRIPLY_API bool readFromFile( const std::string& filename );
    bool hasColors() const { return !_data.colors.empty(); }

    void useInvertedFaces() { _invertFaces = true; }

    const std::string& getName() const { return _name; }

protected:
    TRIPLY_API virtual void toStream( std::ostream& os );
    TRIPLY_API virtual void fromMemory( char* start );

private:
    bool _constructFromPly( const std::string& filename );
    bool _readBinary( std::string filename );

    void _beginRendering( VertexBufferState& state ) const;
    void _endRendering( VertexBufferState& state ) const;

    friend class VertexBufferDist;
    VertexBufferData _data;
    bool             _invertFaces;
    std::string      _name;
};
}


#endif // PLYLIB_VERTEXBUFFERROOT_H
