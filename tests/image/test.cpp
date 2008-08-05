
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/base/clock.h>
#include <eq/client/image.h>

#include <numeric>

using namespace eq::base;
using namespace eq;
using namespace std;

// Tests the functionality of the image compression and computes the bandwidth
// threshold upon which the compression is slower than sending uncompressed
// data.

int main( int argc, char **argv )
{
    Image    image;
    Image    destImage;
    uint32_t size;

    // Touch memory once
    TEST( image.readImage( "../compositor/Image_1_color.rgb",
                           Frame::BUFFER_COLOR ));
    destImage.setPixelViewport( image.getPixelViewport( ));
    destImage.setPixelData( Frame::BUFFER_COLOR,     
                            image.compressPixelData( Frame::BUFFER_COLOR ));

    // Random Noise
    TEST( image.readImage( "noise.rgb", Frame::BUFFER_COLOR ));
    destImage.setPixelViewport( image.getPixelViewport( ));

    const uint8_t* noiseData = image.getPixelPointer( Frame::BUFFER_COLOR );
    const uint32_t noiseSize = image.getPixelDataSize( Frame::BUFFER_COLOR );
    const uint8_t* data;
    Clock          clock;
    float          time;

    clock.reset();
    const Image::PixelData& noise =image.compressPixelData(Frame::BUFFER_COLOR);
    time = clock.getTimef();

    size = accumulate( noise.chunkSizes.begin(), noise.chunkSizes.end(), 0 );
    const ssize_t saved = static_cast< ssize_t >( noiseSize ) - 
                          static_cast< ssize_t >( size );
    cout << argv[0] << ": Noise " << noiseSize << "->" << size << " " 
         << 100.0f * size / noiseSize << "%, " << time << " ms ("
         << 1000.0f * noiseSize / time / 1024.0f / 1024.0f 
         << " MB/s), max network bw " 
         << 1000.0f * saved / time / 1024.0f / 1024.0f << "MB/s" << endl;

    clock.reset();
    destImage.setPixelData( Frame::BUFFER_COLOR, noise );
    time = clock.getTimef();

    cout << argv[0] << ": Noise " << size  << "->" << noiseSize << " " << time
         << " ms, max bw " 
         << 1000.0f * saved / time / 1024.0f / 1024.0f << "MB/s" << endl;

    //destImage.writeImage( "noise_decomp.rgb", Frame::BUFFER_COLOR );
    data = destImage.getPixelPointer( Frame::BUFFER_COLOR );
    for( uint32_t i=0; i<noiseSize-7; ++i ) // last 7 pixels can be unitialized
        TEST( noiseData[i] == data[i] );


    // Real color data 
    TEST( image.readImage( "../compositor/Image_1_color.rgb",
                           Frame::BUFFER_COLOR ));

    destImage.setPixelViewport( image.getPixelViewport( ));

    const uint8_t* colorData = image.getPixelPointer( Frame::BUFFER_COLOR );
    const uint32_t colorSize = image.getPixelDataSize( Frame::BUFFER_COLOR );

    clock.reset();
    const Image::PixelData& color =image.compressPixelData(Frame::BUFFER_COLOR);
    time = clock.getTimef();

    size = accumulate( color.chunkSizes.begin(), color.chunkSizes.end(), 0 );
    cout << argv[0] << ": Color " << colorSize << "->" << size << " " 
         << 100.0f * size / colorSize << "%, " << time << " ms ("
         << 1000.0f * colorSize / time / 1024.0f / 1024.0f 
         << " MB/s), max network bw " 
         << 1000.0f * (colorSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    clock.reset();
    destImage.setPixelData( Frame::BUFFER_COLOR, color );
    time = clock.getTimef();
    
    cout << argv[0] << ": Color " << size  << "->" << colorSize << " " << time
         << " ms, max bw " 
         << 1000.0f * (colorSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    //destImage.writeImage( "../compositor/Image_1_color_decomp.rgb", 
    //                      Frame::BUFFER_COLOR );
    data = destImage.getPixelPointer( Frame::BUFFER_COLOR );
    for( uint32_t i=0; i<colorSize-7; ++i ) // last 7 pixels can be unitialized
        TEST( colorData[i] == data[i] );


    // Depth
    TEST( image.readImage( "../compositor/Image_1_depth.rgb",
                           Frame::BUFFER_DEPTH ));
    const uint8_t* depthData = image.getPixelPointer( Frame::BUFFER_DEPTH);
    const uint32_t depthSize = image.getPixelDataSize( Frame::BUFFER_DEPTH);

    destImage.setPixelViewport( image.getPixelViewport( ));

    clock.reset();
    const Image::PixelData& depth =image.compressPixelData(Frame::BUFFER_DEPTH);
    time = clock.getTimef();

    size = accumulate( depth.chunkSizes.begin(), depth.chunkSizes.end(), 0 );
    cout << argv[0] << ": Depth " << depthSize << "->" << size << " " 
         << 100.0f * size / depthSize << "%, " << time << " ms ("
         << 1000.0f * depthSize / time / 1024.0f / 1024.0f 
         << " MB/s), max network bw " 
         << 1000.0f * (depthSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    clock.reset();
    destImage.setPixelData( Frame::BUFFER_DEPTH, depth );
    time = clock.getTimef();

    cout << argv[0] << ": Depth " << size  << "->" << depthSize << " " << time
         << " ms, max bw " 
         << 1000.0f * (depthSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    //destImage.writeImage( "../compositor/Image_1_depth_decomp.rgb", 
    //                      Frame::BUFFER_DEPTH );
    data = destImage.getPixelPointer( Frame::BUFFER_DEPTH );
    for( uint32_t i=0; i<depthSize-7; ++i ) // last 7 pixels can be unitialized
        TEST( depthData[i] == data[i] );
}
