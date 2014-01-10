
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
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

#ifndef EVOLVE_H
#define EVOLVE_H

#include <eq/eq.h>

#include "rawVolModelRenderer.h"

/** The Equalizer Volume Rendering Example */
namespace eVolve
{
class LocalInitData;
typedef RawVolumeModelRenderer Renderer;

class EVolve : public eq::Client
{
public:
    EVolve( const LocalInitData& initData );

    /** Run an eqPly instance. */
    int run();

    static const std::string& getHelp();

protected:
    virtual ~EVolve() {}

    /** @sa eq::Client::clientLoop. */
    virtual void clientLoop();

private:
    const LocalInitData& _initData;
};

enum ColorMode
{
    COLOR_MODEL,    //!< Render using the colors defined in the ply file
    COLOR_DEMO,     //!< Use a unique color to demonstrate decomposition
    COLOR_HALF_DEMO,//!< 50% unique color + 50% original color
    COLOR_ALL       //!< @internal, must be last
};

enum BackgroundMode
{
    BG_BLACK,   //!< Black background
    BG_WHITE,   //!< White background
    BG_COLOR,   //!< Unique color
    BG_ALL      //!< @internal, must be last
};

enum NormalsQuality
{
    NQ_FULL,    //!< Highest normals quality
    NQ_MEDIUM,  //!< Average normals quality
    NQ_MINIMAL, //!< Basic normal approximation
    NQ_ALL      //!< @internal, must be last
};

enum LogTopics
{
    LOG_STATS = eq::LOG_CUSTOM      // 65536
};
}

namespace lunchbox
{
template<> inline void byteswap( eVolve::ColorMode& value )
{ byteswap( reinterpret_cast< uint32_t& >( value )); }

template<> inline void byteswap( eVolve::BackgroundMode& value )
{ byteswap( reinterpret_cast< uint32_t& >( value )); }

template<> inline void byteswap( eVolve::NormalsQuality& value )
{ byteswap( reinterpret_cast< uint32_t& >( value )); }
}
#endif // EVOLVE_H
