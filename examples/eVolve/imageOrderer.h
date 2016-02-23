
/* Copyright (c) 2007-2016 Maxim Makhinya
 *                         Stefan.Eilemann@epfl.ch
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

#ifndef EVOLVE_IMAGE_ORDERER_H
#define EVOLVE_IMAGE_ORDERER_H

#include <eq/imageOp.h>

namespace eVolve
{
/** @cond IGNORE */
namespace
{
bool _cmpRangesDec( const eq::ImageOp& a, const eq::ImageOp& b )
{
    return a.image->getContext().range.start <
           b.image->getContext().range.start;
}

bool _cmpRangesInc( const eq::ImageOp& a, const eq::ImageOp& b )
{
    return a.image->getContext().range.start >
           b.image->getContext().range.start;
}
}

void orderImages( eq::ImageOps& images, const eq::Matrix4f& modelviewM,
                  const eq::Matrix3f& modelviewITM,
                  const eq::Matrix4f& rotation, const bool orthographic )
{
    if( orthographic )
    {
        const bool orientation = rotation.array[10] < 0;
        std::sort( images.begin(), images.end(),
                   orientation ? _cmpRangesInc : _cmpRangesDec );
        return;
    }
    // else perspective projection

    std::sort( images.begin(), images.end(), _cmpRangesInc );

    // cos of angle between normal and vectors from center
    std::vector< float > dotVals;
    eq::Vector3f norm = modelviewITM * eq::Vector3f( 0.f, 0.f, 1.f );
    norm.normalize();

    // of projection to the middle of slices' boundaries
    for( const eq::ImageOp& op : images )
    {
        const float px = -1.f + op.image->getContext().range.end * 2.f;
        const eq::Vector4f pS = modelviewM * eq::Vector4f( 0.f, 0.f, px , 1.f );
        eq::Vector3f pSsub( pS[ 0 ], pS[ 1 ], pS[ 2 ] );
        pSsub.normalize();
        dotVals.emplace_back( norm.dot( pSsub ));
    }

    const eq::Vector4f pS = modelviewM * eq::Vector4f( 0.f, 0.f,-1.f, 1.f );
    eq::Vector3f pSsub( pS[ 0 ], pS[ 1 ], pS[ 2 ] );
    pSsub.normalize();
    dotVals.emplace_back( norm.dot( pSsub ));

    //check if any slices need to be rendered in reverse order
    size_t minPos = std::numeric_limits< size_t >::max();
    for( size_t i = 0; i < dotVals.size() - 1; ++i )
        if( dotVals[i] > 0 && dotVals[i+1] > 0 )
            minPos = static_cast< int >( i );

    const size_t nImages = images.size();
    minPos++;
    if( minPos < images.size()-1 )
    {
        eq::ImageOps imagesTmp = images;

        // copy slices that should be rendered first
        memcpy( &images[ nImages-minPos-1 ], &imagesTmp[0],
                (minPos+1) * sizeof( eq::ImageOp ));

         // copy slices that should be rendered last, in reverse order
        for( size_t i=0; i < nImages-minPos-1; ++i )
            images[ i ] = imagesTmp[ nImages-i-1 ];
    }
}
/** @endcond */
}

#endif //EVOLVE_IMAGE_ORDERER_H
