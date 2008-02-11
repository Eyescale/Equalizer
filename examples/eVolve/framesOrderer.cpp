/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#include "framesOrderer.h"

namespace eVolve
{


static bool cmpRangesDec(const Frame& frame1, const Frame& frame2)
{
    return frame1.getRange().start < frame2.getRange().start;
}


static bool cmpRangesInc(const Frame& frame1, const Frame& frame2)
{
    return frame1.getRange().start > frame2.getRange().start;
}


void orderFrames(       std::vector< Frame > &frames,
                  const vmml::Matrix4d       &modelviewM,
                  const vmml::Matrix3d       &modelviewITM,
                  const vmml::Matrix4f       &rotation,
                  const bool                  perspective    )
{
    if( !perspective ) // parallel/ortho projection
    {
        const bool orientation = rotation.ml[10] < 0;
        sort( frames.begin(), frames.end(),
              orientation ? cmpRangesDec : cmpRangesInc );
        return;
    }

    vmml::Vector3d norm = modelviewITM * vmml::Vector3d( 0.0, 0.0, 1.0 );
    norm.normalize();

    sort( frames.begin(), frames.end(), cmpRangesDec );

    // cos of angle between normal and vectors from center
    std::vector<double> dotVals;

    // of projection to the middle of slices' boundaries
    for( std::vector< Frame >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const Frame& frame = *i;
        double       px    = -1.0 + frame.getRange().start*2.0;

        vmml::Vector4d pS = 
            modelviewM * vmml::Vector4d( 0.0, 0.0, px , 1.0 );
            
        dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );
    }

    const vmml::Vector4d pS = modelviewM * vmml::Vector4d( 0.0, 0.0, 1.0, 1.0 );
    dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );

    //check if any slices need to be rendered in rederse order
    size_t minPos = 0xffffffffu;
    for( size_t i=0; i<dotVals.size()-1; i++ )
        if( dotVals[i] < 0 && dotVals[i+1] < 0 )
            minPos = static_cast< int >( i );

    if( minPos < frames.size() )
    {
        uint32_t        nFrames   = frames.size();
        std::vector< Frame > framesTmp = frames;

        //Copy slices that should be rendered first
        if( minPos < nFrames-1 )
            memcpy( &frames[0], &framesTmp[minPos+1],
                    (nFrames-minPos-1)*sizeof( Frame ) );

        //Copy sliced that shouls be rendered last in reversed order
        for( size_t i=0; i<=minPos; i++ )
            frames[ nFrames-i-1 ] = framesTmp[i];
    }
}

}
