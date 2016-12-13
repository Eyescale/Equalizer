
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
#include <co/dataIStream.h> // used inline
#include <co/dataOStream.h> // used inline
#include <lunchbox/memoryMap.h> // member

namespace plydist
{
/** Model distribution of a blob of data, e.g., a 3D volume */
class MemModel : public Model
{
public:
    MemModel( co::Object::ChangeType type, const co::CompressorInfo& compressor,
              const std::string& name, co::LocalNodePtr localNode );
    MemModel( co::NodePtr master, co::LocalNodePtr localNode,
              const co::uint128_t& id );
    virtual ~MemModel();

private:
    void getInstanceData( co::DataOStream& os ) final
    {
        os << uint64_t( _map.getSize( ))
           << co::Array< void >( _map.getAddress(), _map.getSize( ));
    }

    void applyInstanceData( co::DataIStream& is ) final
    {
        const uint64_t size = is.read< uint64_t >();
        void* ptr = malloc( size );
        co::Array< void > array( ptr, size );
        is >> array;
        free( ptr );
    }

    lunchbox::MemoryMap _map;
};
}
