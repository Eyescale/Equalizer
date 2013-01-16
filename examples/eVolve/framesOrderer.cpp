
/* Copyright (c) 2007       Maxim Makhinya
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

#include "framesOrderer.h"

namespace eVolve
{

static bool cmpRangesDec(const eq::Frame* frame1, const eq::Frame* frame2)
{
    return frame1->getRange().start < frame2->getRange().start;
}


static bool cmpRangesInc(const eq::Frame* frame1, const eq::Frame* frame2)
{
    return frame1->getRange().start > frame2->getRange().start;
}


void orderFrames( eq::Frames& frames, const eq::Matrix4d& modelviewM,
                  const eq::Matrix3d& modelviewITM,
                  const eq::Matrix4f& rotation, const bool orthographic )
{
    if( orthographic )
    {
        const bool orientation = rotation.array[10] < 0;
        sort( frames.begin(), frames.end(),
              orientation ? cmpRangesInc : cmpRangesDec );
        return;
    }
    // else perspective projection

    eq::Vector3d norm = modelviewITM * eq::Vector3d( 0.0, 0.0, 1.0 );
    norm.normalize();

    sort( frames.begin(), frames.end(), cmpRangesInc );

    // cos of angle between normal and vectors from center
    std::vector<double> dotVals;

    // of projection to the middle of slices' boundaries
    for( eq::Frames::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const eq::Frame* frame = *i;
        const double     px    = -1.0 + frame->getRange().end*2.0;

        const eq::Vector4d pS = modelviewM * eq::Vector4d( 0.0, 0.0, px , 1.0 );
        eq::Vector3d pSsub( pS[ 0 ], pS[ 1 ], pS[ 2 ] );
        pSsub.normalize();
        dotVals.push_back( norm.dot( pSsub ));
    }

    const eq::Vector4d pS = modelviewM * eq::Vector4d( 0.0, 0.0,-1.0, 1.0 );
    eq::Vector3d pSsub( pS[ 0 ], pS[ 1 ], pS[ 2 ] );
    pSsub.normalize();
    dotVals.push_back( norm.dot( pSsub ));
    //check if any slices need to be rendered in reverse order
    size_t minPos = std::numeric_limits< size_t >::max();
    for( size_t i=0; i<dotVals.size()-1; i++ )
        if( dotVals[i] > 0 && dotVals[i+1] > 0 )
            minPos = static_cast< int >( i );

    const size_t nFrames = frames.size();
    minPos++;
    if( minPos < frames.size()-1 )
    {
        eq::Frames framesTmp = frames;

        // copy slices that should be rendered first
        memcpy( &frames[ nFrames-minPos-1 ], &framesTmp[0],
                (minPos+1)*sizeof( eq::Frame* ) );
 
         // copy slices that should be rendered last, in reverse order
        for( size_t i=0; i<nFrames-minPos-1; i++ )
            frames[ i ] = framesTmp[ nFrames-i-1 ];
    }
}

}

