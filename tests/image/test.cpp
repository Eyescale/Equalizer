
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <eq/client/image.h>

#include <numeric>

using namespace eq::base;
using namespace eq;
using namespace std;

// Tests the functionality of the image compression and computes the bandwidth
// threshold upon which the compression is slower than sending uncompressed
// data.

//#define NET_BANDWIDTH (1024.0f*1024.0f*128.0f)  //1Gb network with no overhead
#define NET_BANDWIDTH (1024.0f*1024.0f*115.0f)  // 1Gb network, real

int main( int argc, char **argv )
{
    Image    image;
    Image    destImage;
    uint32_t size;

    // Touch memory once
    TEST( image.readImage( "noise.rgb",
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
    float          totalTimeColor;
    float          totalTimeDepth;
    float          totalTimeComprColor;
    float          totalTimeComprDepth;

    clock.reset();
    const Image::PixelData& noise =image.compressPixelData(Frame::BUFFER_COLOR);
    time = clock.getTimef();
    totalTimeColor = time;

    size = 0;
    for( vector< uint64_t >::const_iterator i = noise.compressedSize.begin();
         i != noise.compressedSize.end(); ++i )
    {
        size += *i;
    }

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

    totalTimeColor += time;
    totalTimeComprColor = totalTimeColor;
    totalTimeColor += 1000.0f * size / NET_BANDWIDTH; // time on 1Gb/s
    cout << argv[0] << ": Noise " << 1000.0f / totalTimeColor 
        << " fps, instead of " << NET_BANDWIDTH / noiseSize << " fps"
        << " (comp-decomp " << 1000.0f / totalTimeComprColor << " fps)"<< endl;

    //destImage.writeImage( "noise_decomp.rgb", Frame::BUFFER_COLOR );
    data = destImage.getPixelPointer( Frame::BUFFER_COLOR );
#ifdef EQ_IGNORE_ALPHA
    for( uint32_t i=0; i<noiseSize-7; ++i ) // last 7 pixels can be unitialized
        TESTINFO( noiseData[i] == data[i] || (i%4)==3,
                  "got " << (int)data[i] << " expected " << (int)noiseData[i]
                         << " at " << i );
#else
    for( uint32_t i=0; i<noiseSize-7; ++i ) // last 7 pixels can be unitialized
        TESTINFO( noiseData[i] == data[i],
                  "got " << (int)data[i] << " expected " << (int)noiseData[i]
                         << " at " << i );
#endif

    // Real color data 
    TEST( image.readImage( "../compositor/Result_DB_color.rgb",
                           Frame::BUFFER_COLOR ));

    destImage.setPixelViewport( image.getPixelViewport( ));

    const uint8_t* colorData = image.getPixelPointer( Frame::BUFFER_COLOR );
    const uint32_t colorSize = image.getPixelDataSize( Frame::BUFFER_COLOR );

    clock.reset();
    const Image::PixelData& color =image.compressPixelData(Frame::BUFFER_COLOR);
    time = clock.getTimef();
    totalTimeColor = time;

    size = 0;
    for( vector< uint64_t >::const_iterator i = color.compressedSize.begin();
         i != color.compressedSize.end(); ++i )
    {
        size += *i;
    }
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

    totalTimeColor += time;
    totalTimeComprColor = totalTimeColor;
    totalTimeColor += 1000.0f * size / NET_BANDWIDTH; // time on 1Gb/s
    cout << argv[0] << ": Color " << 1000.0f / totalTimeColor 
        << " fps, instead of " << NET_BANDWIDTH / colorSize << " fps"
        << " (comp-decomp " << 1000.0f / totalTimeComprColor << " fps)"<< endl;

    //destImage.writeImage( "../compositor/Image_1_color_decomp.rgb", 
    //                      Frame::BUFFER_COLOR );
    data = destImage.getPixelPointer( Frame::BUFFER_COLOR );
#ifdef EQ_IGNORE_ALPHA
    for( uint32_t i=0; i<colorSize-7; ++i ) // last 7 pixels can be initialized
        TESTINFO( colorData[i] == data[i] || (i%4)==3,
                  "got " << (int)data[i] << " expected " << (int)colorData[i]
                  << " at " << i );
#else
    for( uint32_t i=0; i<colorSize-7; ++i ) // last 7 pixels can be initialized
        TESTINFO( colorData[i] == data[i],
                  "got " << (int)data[i] << " expected " << (int)colorData[i]
                  << " at " << i );
#endif

    // Depth
    TEST( image.readImage( "../compositor/Result_DB_depth.rgb",
                           Frame::BUFFER_DEPTH ));
    const uint8_t* depthData = image.getPixelPointer( Frame::BUFFER_DEPTH);
    const uint32_t depthSize = image.getPixelDataSize( Frame::BUFFER_DEPTH);

    destImage.setPixelViewport( image.getPixelViewport( ));

    clock.reset();
    const Image::PixelData& depth =image.compressPixelData(Frame::BUFFER_DEPTH);
    time = clock.getTimef();
    totalTimeDepth = time;

    size = 0;
    for( vector< uint64_t >::const_iterator i = depth.compressedSize.begin();
         i != depth.compressedSize.end(); ++i )
    {
        size += *i;
    }
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

    totalTimeDepth += time;
    totalTimeComprDepth = totalTimeDepth;
    totalTimeDepth += 1000.0f * size / NET_BANDWIDTH; // time on 1Gb/s
    cout << argv[0] << ": Depth " << 1000.0f / totalTimeDepth 
        << " fps, instead of " << NET_BANDWIDTH / depthSize << " fps"
        << " (compression " << 1000.0f / totalTimeComprDepth << " fps)"<< endl;

    cout << argv[0] << ": Color+Depth " 
        << 1000.0f / (totalTimeDepth + totalTimeColor )
        << " fps, instead of " << NET_BANDWIDTH / (colorSize + depthSize)
        << " fps (comp-decomp "
        << 1000.0f/(totalTimeComprColor+totalTimeComprDepth) << " fps)" << endl;


    //destImage.writeImage( "../compositor/Image_1_depth_decomp.rgb", 
    //                      Frame::BUFFER_DEPTH );
    data = destImage.getPixelPointer( Frame::BUFFER_DEPTH );
#ifdef EQ_IGNORE_ALPHA
    for( uint32_t i=0; i<depthSize-7; ++i ) // last 7 pixels can be unitialized
        TESTINFO( depthData[i] == data[i] || (i%4)==3,
                  "got " << (int)data[i] << " expected " << (int)depthData[i]
                  << " at " << i );
#else
    for( uint32_t i=0; i<depthSize-7; ++i ) // last 7 pixels can be unitialized
        TESTINFO( depthData[i] == data[i],
                  "got " << (int)data[i] << " expected " << (int)depthData[i]
                  << " at " << i );
#endif
}

