
/*
 * Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

 *
 * The init data manages static, per-instance application data. In this example,
 * it holds the model file name, and manages the instantiation of the frame
 * data. The frame data is instantiated seperately for each (pipe) thread, so
 * that multiple pipe threads on a node can render different frames
 * concurrently.
 */

#include "initData.h"

namespace eVolve
{

InitData::InitData()
    : _frameDataID()
#ifdef AGL
    , _windowSystem( "AGL" )
#elif GLX
    , _windowSystem( "GLX" )
#elif WGL
    , _windowSystem( "WGL" )
#endif
    , _precision( 2 )
    , _brightness( 1.0f )
    , _alpha( 1.0f )
    , _filename( lunchbox::getRootPath() +
                 "/share/Equalizer/data/Bucky32x32x32_d.raw" )
{}

InitData::~InitData()
{
    setFrameDataID( eq::uint128_t( ));
}

void InitData::getInstanceData( co::DataOStream& os )
{
    os << _frameDataID << _windowSystem << _precision << _brightness << _alpha
       << _filename;
}

void InitData::applyInstanceData( co::DataIStream& is )
{
    is >> _frameDataID >> _windowSystem >> _precision >> _brightness >> _alpha
       >> _filename;

    LBASSERT( _frameDataID != 0 );
}
}
