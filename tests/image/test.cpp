
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/base/clock.h>
#include <eq/client/image.h>

using namespace eqBase;
using namespace eq;
using namespace std;

// Tests the functionality of the compressor and computes the bandwidth
// threshold upon which the compression is slower than sending uncompressed
// data.

int main( int argc, char **argv )
{
    Image image;
    TEST( image.readImage( "Image_7_color.rgb", Frame::BUFFER_COLOR ));
    TEST( image.readImage( "Image_7_depth.rgb", Frame::BUFFER_DEPTH ));

    Image destImage;
    destImage.setPixelViewport( image.getPixelViewport( ));

    const uint8_t* colorData = image.getPixelData( Frame::BUFFER_COLOR);
    const size_t   colorSize = image.getPixelDataSize( Frame::BUFFER_COLOR);
    const uint8_t* depthData = image.getPixelData( Frame::BUFFER_DEPTH);
    const size_t   depthSize = image.getPixelDataSize( Frame::BUFFER_DEPTH);
    const uint8_t* compressedData;
    const uint8_t* data;
    uint32_t       size;
    Clock          clock;
    float          time;

    // Color
    clock.reset();
    compressedData = image.compressPixelData( Frame::BUFFER_COLOR, size );
    time = clock.getTimef();

    cout << argv[0] << ": Color " << colorSize << "->" << size << " " 
         << 100.0f * size / colorSize << "%, " << time << " ms, max bw " 
         << 1000.0f * (colorSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    clock.reset();
    TEST( size == 
          destImage.decompressPixelData( Frame::BUFFER_COLOR, compressedData ));
    time = clock.getTimef();

    cout << argv[0] << ": Color " << size  << "->" << colorSize << " " << time
         << " ms, max bw " 
         << 1000.0f * (colorSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    data = destImage.getPixelData( Frame::BUFFER_COLOR );
    for( uint32_t i=0; i<colorSize; ++i )
        TEST( colorData[i] == data[i] );


    // Depth
    clock.reset();
    compressedData = image.compressPixelData( Frame::BUFFER_DEPTH, size );
    time = clock.getTimef();

    cout << argv[0] << ": Depth " << depthSize << "->" << size << " " 
         << 100.0f * size / depthSize << "%, " << time << " ms, max bw " 
         << 1000.0f * (depthSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    clock.reset();
    TEST( size == 
          destImage.decompressPixelData( Frame::BUFFER_DEPTH, compressedData ));
    time = clock.getTimef();

    cout << argv[0] << ": Depth " << size  << "->" << depthSize << " " << time
         << " ms, max bw " 
         << 1000.0f * (depthSize - size) / time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    data = destImage.getPixelData( Frame::BUFFER_DEPTH );
    for( uint32_t i=0; i<depthSize; ++i )
        TEST( depthData[i] == data[i] );
}
