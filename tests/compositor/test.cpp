
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/base/clock.h>
#include <eq/client/compositor.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/image.h>

using namespace eqBase;
using namespace eq;
using namespace std;

// Tests the functionality of the compositor and computes the performance.

int main( int argc, char **argv )
{
    Frame      frame;
    FrameData* frameData = new eq::FrameData;

    frameData->setBuffers( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH );
    frame.setData( frameData );

    Image* image = frameData->newImage();
    TEST( image->readImage( "Image_1_color.rgb", Frame::BUFFER_COLOR ));
    TEST( image->readImage( "Image_1_depth.rgb", Frame::BUFFER_DEPTH ));
    image = frameData->newImage();
    TEST( image->readImage( "Image_2_color.rgb", Frame::BUFFER_COLOR ));
    TEST( image->readImage( "Image_2_depth.rgb", Frame::BUFFER_DEPTH ));
    image = frameData->newImage();
    TEST( image->readImage( "Image_3_color.rgb", Frame::BUFFER_COLOR ));
    TEST( image->readImage( "Image_3_depth.rgb", Frame::BUFFER_DEPTH ));
    
    FrameVector  frames;
    Clock        clock;
    float        time;
    const size_t size = image->getPixelDataSize( Frame::BUFFER_COLOR ) * 6;
    frames.push_back( &frame );

    clock.reset();
    const Image* result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": first op:  " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    clock.reset();
    result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": second op: " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    result->writeImages( "Result" );

    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": 15 images: " << time << " ms (" 
         << 5000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;
}
