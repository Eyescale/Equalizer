
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@eyescale.ch>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#include "pipe.h"

#include "config.h"

namespace osgScaleViewer
{

Pipe::Pipe( eq::Node* parent )
    : eq::Pipe( parent )
{
}

Pipe::~Pipe()
{
}

const FrameData& Pipe::getFrameData() const
{
    return _frameData;
}

bool Pipe::configInit( const eq::uint128_t& initID )
{
    if( !eq::Pipe::configInit( initID ))
        return false;

    Config* config = static_cast<Config*>( getConfig( ));
    const InitData& initData = config->getInitData();
    const eq::UUID& frameDataID = initData.getFrameDataID();
    const bool mapped = config->mapObject( &_frameData, frameDataID );
    LBASSERT( mapped );

    return mapped;
}

bool Pipe::configExit()
{
    getConfig()->unmapObject( &_frameData );
    return eq::Pipe::configExit();
}

void Pipe::frameStart( const eq::uint128_t& frameID,
                       const uint32_t frameNumber )
{
    _frameData.sync( frameID );
    eq::Pipe::frameStart( frameID, frameNumber );
}

}
