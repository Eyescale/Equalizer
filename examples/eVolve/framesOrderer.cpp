
/* Copyright (c) 2007       Maxim Makhinya
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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


void orderFrames( eq::FrameVector&      frames,
                  const vmml::Matrix4d& modelviewM,
                  const vmml::Matrix3d& modelviewITM,
                  const vmml::Matrix4f& rotation,
                  const bool            orthographic )
{
    if( orthographic )
    {
        const bool orientation = rotation.ml[10] < 0;
        sort( frames.begin(), frames.end(),
              orientation ? cmpRangesInc : cmpRangesDec );
        return;
    }
    // else perspective projection

    vmml::Vector3d norm = modelviewITM * vmml::Vector3d( 0.0, 0.0, 1.0 );
    norm.normalize();

    sort( frames.begin(), frames.end(), cmpRangesInc );

    // cos of angle between normal and vectors from center
    std::vector<double> dotVals;

    // of projection to the middle of slices' boundaries
    for( eq::FrameVector::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const eq::Frame* frame = *i;
        const double     px    = -1.0 + frame->getRange().end*2.0;

        vmml::Vector4d pS = 
            modelviewM * vmml::Vector4d( 0.0, 0.0, px , 1.0 );
            
        dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );
    }

    const vmml::Vector4d pS = modelviewM * vmml::Vector4d( 0.0, 0.0,-1.0, 1.0 );
    dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );

    //check if any slices need to be rendered in reverse order
    size_t minPos = std::numeric_limits< size_t >::max();
    for( size_t i=0; i<dotVals.size()-1; i++ )
        if( dotVals[i] > 0 && dotVals[i+1] > 0 )
            minPos = static_cast< int >( i );

    const uint32_t nFrames = frames.size();
    minPos++;
    if( minPos < frames.size()-1 )
    {
        eq::FrameVector framesTmp = frames;

        // copy slices that should be rendered first
        memcpy( &frames[ nFrames-minPos-1 ], &framesTmp[0],
                (minPos+1)*sizeof( eq::Frame* ) );
 
         // copy slices that should be rendered last, in reverse order
        for( size_t i=0; i<nFrames-minPos-1; i++ )
            frames[ i ] = framesTmp[ nFrames-i-1 ];
    }
}

}
