
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <test.h>

#include <eq/client/compositor.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/image.h>
#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>
#include <eq/fabric/drawableConfig.h>
#include <lunchbox/clock.h>

// Tests the functionality of the compositor and computes the performance.

int main( int argc, char **argv )
{
    eq::NodeFactory nodeFactory;
    TEST( eq::init( 0, 0, &nodeFactory ));

    eq::Frame      frame;
    eq::FrameData* frameData = new eq::FrameData;

    frameData->setBuffers( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH );
    frame.setData( frameData );

    // 1) 2D assembly test
    eq::Image* image = frameData->newImage( eq::Frame::TYPE_MEMORY,
                                            eq::DrawableConfig( ));
    TEST( image->readImage( "Image_1_color.rgb", eq::Frame::BUFFER_COLOR ));
    TEST( image->hasPixelData( eq::Frame::BUFFER_COLOR ));
    image = frameData->newImage( eq::Frame::TYPE_MEMORY, eq::DrawableConfig( ));
    TEST( image->readImage( "Image_2_color.rgb", eq::Frame::BUFFER_COLOR ));
    TEST( image->hasPixelData( eq::Frame::BUFFER_COLOR ));
    image = frameData->newImage( eq::Frame::TYPE_MEMORY, eq::DrawableConfig( ));
    TEST( image->readImage( "Image_3_color.rgb", eq::Frame::BUFFER_COLOR ));
    TEST( image->hasPixelData( eq::Frame::BUFFER_COLOR ));
    
    eq::Frames frames;
    co::base::Clock clock;
    float time;
    const size_t size = image->getPixelDataSize( eq::Frame::BUFFER_COLOR ) * 3;
    frames.push_back( &frame );

    clock.reset();
    const eq::Image* result = eq::Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": 2D first op:  " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << std::endl;

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": 2D second op: " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << std::endl;

    result->writeImages( "Result_2D" );

    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": 2D 15 images: " << time << " ms (" 
         << 5000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << std::endl;

    // 2) DB assembly test
#ifdef EQ_USE_PARACOMP_DEPTH
    std::cout << "Using Paracomp PC compositing (depth)" << std::endl;
#endif
    const eq::Images& images = frameData->getImages();

    image = images[0];
    TEST( image->hasPixelData( eq::Frame::BUFFER_COLOR ));
    TEST( image->readImage( "Image_1_depth.rgb", eq::Frame::BUFFER_DEPTH ));
    TESTINFO( image->hasPixelData( eq::Frame::BUFFER_COLOR ),
              "Color buffer removed - probably different image dimensions?" );
    TEST( image->hasPixelData( eq::Frame::BUFFER_DEPTH ));
    image = images[1];
    TEST( image->readImage( "Image_2_depth.rgb", eq::Frame::BUFFER_DEPTH ));
    TEST( image->hasPixelData( eq::Frame::BUFFER_COLOR ));
    TEST( image->hasPixelData( eq::Frame::BUFFER_DEPTH ));
    image = images[2];
    TEST( image->readImage( "Image_3_depth.rgb", eq::Frame::BUFFER_DEPTH ));
    TEST( image->hasPixelData( eq::Frame::BUFFER_COLOR ));
    TEST( image->hasPixelData( eq::Frame::BUFFER_DEPTH ));

    frames.clear();
    frames.push_back( &frame );

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": DB first op:  " << time << " ms (" 
              << 1000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)"
              << std::endl;

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": DB second op: " << time << " ms (" 
              << 1000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)"
              << std::endl;

    result->writeImages( "Result_DB" );

    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": DB 15 images: " << time << " ms (" 
              << 5000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)"
              << std::endl;

    // 3) alpha-blend assembly test
#ifdef EQ_USE_PARACOMP_BLEND
     std::cout << "Using Paracomp PC compositing (blend)" << std::endl;
#endif
    frameData->clear();
    frameData->setBuffers( eq::Frame::BUFFER_COLOR );

    image = frameData->newImage( eq::Frame::TYPE_MEMORY, eq::DrawableConfig( ));
    TEST( image->readImage( "Image_15_color.rgb", eq::Frame::BUFFER_COLOR ));
    image = frameData->newImage( eq::Frame::TYPE_MEMORY, eq::DrawableConfig( ));
    TEST( image->readImage( "Image_14_color.rgb", eq::Frame::BUFFER_COLOR ));
    image = frameData->newImage( eq::Frame::TYPE_MEMORY, eq::DrawableConfig( ));
    TEST( image->readImage( "Image_13_color.rgb", eq::Frame::BUFFER_COLOR ));
    frames.clear();
    frames.push_back( &frame );

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": Alpha first op:  " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << std::endl;

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": Alpha second op: " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << std::endl;

    result->writeImages( "Result_Alpha" );

    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );
    frames.push_back( &frame );

    clock.reset();
    result = eq::Compositor::mergeFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    std::cout << argv[0] << ": Alpha 15 images: " << time << " ms (" 
         << 5000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << std::endl;
    
    TEST( eq::exit( ));

    return EXIT_SUCCESS;
}
