
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/base/clock.h>
#include <eq/fabric/drawableConfig.h>
#include <eq/compositor.h>
#include <eq/frame.h>
#include <eq/frameData.h>
#include <eq/image.h>
#include <eq/init.h>
#include <eq/nodeFactory.h>

using namespace eq::base;
using namespace eq;
using namespace std;

// Tests the functionality of the compositor and computes the performance.

int main( int argc, char **argv )
{
    NodeFactory nodeFactory;
    TEST( eq::init( 0, 0, &nodeFactory ));

    Frame      frame;
    FrameData* frameData = new eq::FrameData;

    frameData->setBuffers( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH );
    frame.setData( frameData );

    // 1) 2D assembly test
    Image* image = frameData->newImage( Frame::TYPE_MEMORY, DrawableConfig( ));
    TEST( image->readImage( "Image_1_color.rgb", Frame::BUFFER_COLOR ));
    image = frameData->newImage( Frame::TYPE_MEMORY, DrawableConfig( ));
    TEST( image->readImage( "Image_2_color.rgb", Frame::BUFFER_COLOR ));
    image = frameData->newImage( Frame::TYPE_MEMORY, DrawableConfig( ));
    TEST( image->readImage( "Image_3_color.rgb", Frame::BUFFER_COLOR ));
    
    Frames frames;
    Clock clock;
    float time;
    const size_t size = image->getPixelDataSize( Frame::BUFFER_COLOR ) * 3;
    frames.push_back( &frame );

    clock.reset();
    const Image* result = Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": 2D first op:  " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    clock.reset();
    result = Compositor::mergeFramesCPU( frames );
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
    result = Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": 2D 15 images: " << time << " ms (" 
         << 5000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    // 2) DB assembly test
#ifdef EQ_USE_PARACOMP_DEPTH
     cout << "Using Paracomp PC compositing (depth)" << endl;
#endif
    const Images& images = frameData->getImages();

    image = images[0];
    TEST( image->readImage( "Image_1_depth.rgb", Frame::BUFFER_DEPTH ));
    image = images[1];
    TEST( image->readImage( "Image_2_depth.rgb", Frame::BUFFER_DEPTH ));
    image = images[2];
    TEST( image->readImage( "Image_3_depth.rgb", Frame::BUFFER_DEPTH ));

    frames.clear();
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": DB first op:  " << time << " ms (" 
         << 1000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    clock.reset();
    result = Compositor::mergeFramesCPU( frames );
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
    result = Compositor::mergeFramesCPU( frames );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": DB 15 images: " << time << " ms (" 
         << 5000.0f * size * 2.f / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    // 3) alpha-blend assembly test
#ifdef EQ_USE_PARACOMP_BLEND
     cout << "Using Paracomp PC compositing (blend)" << endl;
#endif
    frameData->clear();
    frameData->setBuffers( Frame::BUFFER_COLOR );

    image = frameData->newImage( Frame::TYPE_MEMORY, DrawableConfig( ));
    TEST( image->readImage( "Image_15_color.rgb", Frame::BUFFER_COLOR ));
    image = frameData->newImage( Frame::TYPE_MEMORY, DrawableConfig( ));
    TEST( image->readImage( "Image_14_color.rgb", Frame::BUFFER_COLOR ));
    image = frameData->newImage( Frame::TYPE_MEMORY, DrawableConfig( ));
    TEST( image->readImage( "Image_13_color.rgb", Frame::BUFFER_COLOR ));
    frames.clear();
    frames.push_back( &frame );

    clock.reset();
    result = Compositor::mergeFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": Alpha first op:  " << time << " ms (" 
         << 1000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;

    clock.reset();
    result = Compositor::mergeFramesCPU( frames, true );
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
    result = Compositor::mergeFramesCPU( frames, true );
    time = clock.getTimef();
    TEST( result );

    cout << argv[0] << ": Alpha 15 images: " << time << " ms (" 
         << 5000.0f * size / time / 1024.0f / 1024.0f << " MB/s)" << endl;
    
    TEST( eq::exit( ));

    return EXIT_SUCCESS;
}
