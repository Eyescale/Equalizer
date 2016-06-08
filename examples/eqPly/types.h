
/* Copyright (c) 2014-2016, Stefan.Eilemann@epfl.ch
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

#ifndef EQPLY_TYPES_H
#define EQPLY_TYPES_H

#include <eq/eq.h>

#include <triply/vertexBufferDist.h>
#include <triply/vertexBufferRoot.h>

#ifndef M_PI_2
#  define M_PI_2 1.57079632679489661923
#endif

namespace eqPly
{
class FrameData;
class LocalInitData;
class View;

typedef triply::VertexBufferRoot  Model;
typedef triply::VertexBufferDist  ModelDist;

typedef std::vector< Model* > Models;
typedef std::vector< ModelDist* > ModelDists;

typedef Models::const_iterator ModelsCIter;
typedef ModelDists::const_iterator ModelDistsCIter;

enum ColorMode
{
    COLOR_MODEL, //!< Render using the colors defined in the ply file
    COLOR_DEMO,  //!< Use a unique color to demonstrate decomposition
    COLOR_WHITE, //!< Render in solid white (mostly for anaglyph stereo)
    COLOR_ALL    //!< @internal, must be last
};

enum LogTopics
{
    LOG_STATS = eq::LOG_CUSTOM << 0, // 65536
    LOG_CULL  = eq::LOG_CUSTOM << 1  // 131072
};
}

namespace lunchbox
{
template<> inline void byteswap( eqPly::ColorMode& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}
#endif // EQPLY_TYPES_H
