
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/base/clock.h>
#include <eq/client/compositor.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/image.h>
#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>

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
    
    NodeFactory nodeFactory;
    TEST( eq::init( 0, 0, &nodeFactory ));

    // 1) 2D assembly test
    Image* image = frameData->newImage();
    TEST( image->readImage( "Image_1_color.rgb", Frame::BUFFER_COLOR ));
    image = frameData->newImage();
    TEST( image->readImage( "Image_2_color.rgb", Frame::BUFFER_COLOR ));
    image = frameData->newImage();
    TEST( image->readImage( "Image_3_color.rgb", Frame::BUFFER_COLOR ));
    
    FrameVector  frames;
    Clock        clock;
    float        time;
    const size_t size = image->getPixelDataSize( Frame::BUFFER_COLOR ) * 3;
    frames.push_back( &frame );

    clock.reset();
    const Image* result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": 2D first op:  " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    clock.reset();
    result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": 2D second op: " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    result->writeImages( "Result_2D" );

    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": 2D 15 images: " << time << " ms (" 
         << 5000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;
    
    // 2) alpha-blend assembly test
    frames.clear();
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::assembleFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": Alpha first op:  " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    clock.reset();
    result = Compositor::assembleFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": Alpha second op: " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    result->writeImages( "Result_Alpha" );

    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::assembleFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": Alpha 15 images: " << time << " ms (" 
         << 5000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;
    
    // 3) DB assembly test
    const ImageVector& images = frameData->getImages();

    image = images[0];
    TEST( image->readImage( "Image_1_depth.rgb", Frame::BUFFER_DEPTH ));
    image = images[1];
    TEST( image->readImage( "Image_2_depth.rgb", Frame::BUFFER_DEPTH ));
    image = images[2];
    TEST( image->readImage( "Image_3_depth.rgb", Frame::BUFFER_DEPTH ));

    frames.clear();
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": DB first op:  " << time << " ms (" 
         << 1000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    clock.reset();
    result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": DB second op: " << time << " ms (" 
         << 1000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    result->writeImages( "Result_DB" );

    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::assembleFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": DB 15 images: " << time << " ms (" 
         << 5000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)" << endl;
    
    TEST( eq::exit( ));
}
