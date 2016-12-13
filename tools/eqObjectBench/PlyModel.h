
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#pragma once

#include "Model.h"
#include <triply/vertexBufferDist.h>
#include <triply/vertexBufferRoot.h>

namespace plydist
{
/** Model distribution a triply kd-Tree, as in eqPly */
class PlyModel : public Model
{
public:
    PlyModel( co::Object::ChangeType type, const co::CompressorInfo& compressor,
              const std::string& name, co::LocalNodePtr localNode );
    PlyModel( co::NodePtr master, co::LocalNodePtr localNode,
              const co::uint128_t& id );
    virtual ~PlyModel();

private:
    PlyModel( const PlyModel& ) = delete;
    PlyModel( PlyModel&& ) = delete;
    PlyModel& operator = ( const PlyModel& ) = delete;
    PlyModel& operator = ( PlyModel&& ) = delete;

    co::uint128_t commit( const uint32_t incarnation = CO_COMMIT_NEXT ) final
        { return _dist.commit( incarnation ); }
    co::uint128_t sync( const co::uint128_t& version ) final
        { return _dist.sync( version ); }

    triply::VertexBufferRoot _ply;
    triply::VertexBufferDist _dist;
};
}
