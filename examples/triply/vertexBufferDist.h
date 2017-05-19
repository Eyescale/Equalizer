
/* Copyright (c) 2008-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef PLYLIB_VERTEXBUFFERDIST_H
#define PLYLIB_VERTEXBUFFERDIST_H

#include "typedefs.h"
#include <triply/api.h>

#include <co/co.h>
#include <pression/data/CompressorInfo.h>

namespace triply
{
static const co::CompressorInfo COMPRESSOR_AUTO(-1.f, -1.f);

/** Uses co::Object to distribute a model, holds a VertexBufferBase node. */
class VertexBufferDist : public co::Object
{
public:
    /** Register the master version of a ply tree. */
    TRIPLY_API VertexBufferDist(
        triply::VertexBufferRoot& root, co::LocalNodePtr node,
        co::Object::ChangeType type = STATIC,
        const co::CompressorInfo& compressor = COMPRESSOR_AUTO);

    /** Map a slave version of a ply tree. */
    TRIPLY_API VertexBufferDist(triply::VertexBufferRoot& root,
                                co::NodePtr master, co::LocalNodePtr localNode,
                                const co::uint128_t& modelID);
    TRIPLY_API virtual ~VertexBufferDist();

protected:
    TRIPLY_API VertexBufferDist(VertexBufferRoot& root, VertexBufferBase& node,
                                co::LocalNodePtr localNode,
                                co::Object::ChangeType type,
                                const co::CompressorInfo& compressor);

    TRIPLY_API VertexBufferDist(triply::VertexBufferRoot& root,
                                triply::VertexBufferBase& node,
                                co::NodePtr master, co::LocalNodePtr localNode,
                                const co::uint128_t& modelID);

    TRIPLY_API void getInstanceData(co::DataOStream& os) override;
    TRIPLY_API void applyInstanceData(co::DataIStream& is) override;

private:
    bool _isRoot() const { return (void*)(&_root) == (void*)(&_node); }
    std::unique_ptr<VertexBufferBase> _createNode(Type) const;

    ChangeType getChangeType() const final { return _changeType; }
    co::CompressorInfo chooseCompressor() const final { return _compressor; }
    VertexBufferRoot& _root;
    VertexBufferBase& _node;
    std::unique_ptr<VertexBufferDist> _left;
    std::unique_ptr<VertexBufferDist> _right;
    const co::Object::ChangeType _changeType;
    const co::CompressorInfo _compressor;
};
}

#endif // PLYLIB_VERTEXBUFFERDIST_H
