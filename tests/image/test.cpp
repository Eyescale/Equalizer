
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/plugins/compressor.h>
#include <eq/client/global.h>
#include <eq/client/image.h>
#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>
#include <eq/client/pluginRegistry.h>
#include <eq/base/clock.h>
#include <eq/base/fileSearch.h>

#include <numeric>


// Tests the functionality and speed of the image compression.

int main( int argc, char **argv )
{
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    const eq::PluginRegistry& plugins = eq::Global::getPluginRegistry();
    const eq::CompressorVector& compressors = plugins.getCompressors();
    TEST( !compressors.empty( ));

    eq::StringVector images = eq::base::fileSearch( "images", "*.rgb" );
    for( size_t i=0; i < images.size(); ++i )
    {
        images[i] = "images/" + images[i];
    }

    const eq::StringVector images2 = eq::base::fileSearch( "../compositor", 
                                                           "Result*.rgb" );
    for( eq::StringVector::const_iterator i = images2.begin();
         i != images2.end(); ++i )
    {
        images.push_back( "../compositor/" + *i );
    }
    TEST( !images.empty( ));


    // Touch memory once
    eq::Image image;
    eq::Image destImage;
    TEST( image.readImage( images.front(), eq::Frame::BUFFER_COLOR ));
    destImage.setPixelViewport( image.getPixelViewport( ));
    destImage.setPixelData( eq::Frame::BUFFER_COLOR,     
                            image.compressPixelData( eq::Frame::BUFFER_COLOR ));

    TEST( image.readImage( images.front(), eq::Frame::BUFFER_DEPTH ));
    destImage.setPixelViewport( image.getPixelViewport( ));
    destImage.setPixelData( eq::Frame::BUFFER_DEPTH,     
                            image.compressPixelData( eq::Frame::BUFFER_DEPTH ));

    std::cout.setf( std::ios::right, std::ios::adjustfield );
    std::cout.precision( 5 );
    std::cout << "COMPRESSOR,                            IMAGE,       SIZE,"
              << " COMPRESSED,     t_comp,   t_decomp" << std::endl;

    for( eq::StringVector::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const std::string& filename = *i;
        const size_t depthPos = filename.find( "depth" );
        const eq::Frame::Buffer buffer = (depthPos == std::string::npos) ?
            eq::Frame::BUFFER_COLOR : eq::Frame::BUFFER_DEPTH;

        TEST( image.readImage( filename, buffer ));
        destImage.setPixelViewport( image.getPixelViewport( ));

        const uint8_t* data = image.getPixelPointer( buffer );
        const uint32_t size = image.getPixelDataSize( buffer );

        eq::base::Clock clock;
        clock.reset();
        const eq::Image::PixelData& compressedPixels =
            image.compressPixelData( buffer );
        const float compressTime = clock.getTimef();

        uint32_t compressedSize = 0;
        if( compressedPixels.compressorName == EQ_COMPRESSOR_NONE )
            compressedSize = size;
        else 
            for( std::vector< uint64_t >::const_iterator j = 
                     compressedPixels.compressedSize.begin();
                 j != compressedPixels.compressedSize.end(); ++j )
            {
                compressedSize += *j;
            }

        clock.reset();
        destImage.setPixelData( buffer, compressedPixels );
        const float decompressTime = clock.getTimef();

        std::cout  << std::setw(2) << compressedPixels.compressorName << ", "
                   << std::setw(40) << filename << ", " << std::setw(10) << size
                   << ", " << std::setw(10) << compressedSize << ", " 
                   << std::setw(10) << compressTime << ", " << std::setw(10)
                   << decompressTime << std::endl;

        //destImage.writeImage( "decomp_" + filename, buffer );

        const uint8_t* destData = destImage.getPixelPointer( buffer );
#ifdef EQ_IGNORE_ALPHA
        // last 7 pixels can be unitialized
        for( uint32_t j = 0; j < size-7; ++j )
            TESTINFO( data[j] == destData[j] || (i%4)==3,
                      "got " << (int)destData[j] << " expected " << (int)data[j]
                             << " at " << j );
#else
        for( uint32_t j = 0; j < size-7; ++j )
            TESTINFO( data[j] == destData[j],
                      "got " << (int)destData[j] << " expected " << (int)data[j]
                             << " at " << j );
#endif
    }

    eq::exit();
}

