
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/base/clock.h>
#include <eq/base/compressor.h>
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

    const vector<uint8_t>& colorData = image.getPixelData( Frame::BUFFER_COLOR);
    const size_t           colorSize = colorData.size();
    const vector<uint8_t>& depthData = image.getPixelData( Frame::BUFFER_DEPTH);
    const size_t           depthSize = depthData.size();
    vector<uint8_t>        compressedData;
    vector<uint8_t>        decompressedData;

    Clock clock;

    // Color, LZ
    clock.reset();
    Compressor::compressLZ( colorData, compressedData );
    float time = clock.getTimef();
    cout << argv[0] << ": Color, LZ " << colorSize << "->" 
         << compressedData.size() << " " 
         << 100.0f * compressedData.size() / colorSize << "%, " 
         << time << " ms, max bw " 
         << 1000.0f * (colorSize - compressedData.size()) / 
                time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    
    clock.reset();
    TEST( compressedData.size() == 
          Compressor::decompressLZ( &compressedData[0], decompressedData ));
    time = clock.getTimef();
    TEST( decompressedData.size() == colorSize );
    TEST( memcmp( &decompressedData[0], &colorData[0], colorSize ) == 0 );

    cout << argv[0] << ": Color, LZ " << compressedData.size()  << "->" 
         << colorSize << " " << time << " ms, max bw " 
         << 1000.0f * (colorSize - compressedData.size()) / 
                time / 1024.0f / 1024.0f << "MB/s" 
         << endl;
    compressedData.clear();
    decompressedData.clear();

    // Depth, LZ
    clock.reset();
    Compressor::compressLZ( depthData, compressedData );
    time = clock.getTimef();
    cout << argv[0] << ": Depth, LZ " << depthSize << "->" 
         << compressedData.size() << " " 
         << 100.0f * compressedData.size() / depthSize << "%, " 
         << time << " ms, max bw " 
         << 1000.0f * (depthSize - compressedData.size()) / 
                time / 1024.0f / 1024.0f << "MB/s" 
         << endl;

    
    clock.reset();
    TEST( compressedData.size() == 
          Compressor::decompressLZ( &compressedData[0], decompressedData ));
    time = clock.getTimef();
    TEST( decompressedData.size() == depthSize );
    TEST( memcmp( &decompressedData[0], &depthData[0], depthSize ) == 0 );

    cout << argv[0] << ": Depth, LZ " << compressedData.size()  << "->" 
         << depthSize << " " << time << " ms, max bw " 
         << 1000.0f * (depthSize - compressedData.size()) / 
                time / 1024.0f / 1024.0f << "MB/s" 
         << endl;
    compressedData.clear();
    decompressedData.clear();
}
