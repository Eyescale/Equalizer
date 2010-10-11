
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#define EQ_TEST_RUNTIME 120 // seconds
#include <test.h>

#include <eq/plugins/compressor.h>

#include <eq/client/image.h>
#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>
#include <eq/base/clock.h>
#include <eq/base/file.h>
#include <eq/base/global.h>
#include <eq/base/pluginRegistry.h>

#include <eq/client/frame.h>    // enum Eye

#include <numeric>
#include <fstream>

#include "base/compressorInfo.h" // private header
#include "base/plugin.h" // private header


// Tests the functionality and speed of the image compression.
//#define WRITE_DECOMPRESSED
//#define WRITE_COMPRESSED

#ifndef NDEBUG
#  define COMPARE_RESULT
#endif

namespace
{
static std::vector< uint32_t > _getCompressorNames()
{
    const eq::base::PluginRegistry& registry = 
        eq::base::Global::getPluginRegistry();
    const eq::base::Plugins& plugins = registry.getPlugins();

    std::vector< uint32_t > names;
    for( eq::base::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const eq::base::CompressorInfos& infos = (*i)->getInfos();
        for( eq::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const EqCompressorInfo& info = *j;
            if( info.capabilities & EQ_COMPRESSOR_TRANSFER )
                continue;
            names.push_back( info.name );
        }
    }
    
    return names;
}

#ifdef COMPARE_RESULT
static float _getCompressorQuality( const uint32_t name )
{
    const eq::base::PluginRegistry& registry = 
        eq::base::Global::getPluginRegistry();
    const eq::base::Plugins& plugins = registry.getPlugins();

    float quality = 1.0f;
    for( eq::base::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const eq::base::CompressorInfos& infos = (*i)->getInfos();
        for( eq::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            if( name != (*j).name )
                continue;

            quality = (*j).quality;
            break;
        }
    }
    
    return quality;
}
#endif

template< typename T >
static void _compare( const void* data, const void* destData,
                      const eq::Frame::Buffer buffer, bool ignoreAlpha,
                      const int64_t nElem, const float quality )
{
    const T* destValue = reinterpret_cast< const T* >( destData );
    const T* value = reinterpret_cast< const T* >( data );

#pragma omp parallel for
    for( int64_t k = 0; k < nElem; ++k )
    { 
        if( ignoreAlpha && buffer == eq::Frame::BUFFER_COLOR )
        {
            // Don't test alpha if alpha is ignored
            if( k % 4 == 3 )
                continue;
        }

        const float max = ( 1.f - quality ) * std::numeric_limits< T >::max();
        TESTINFO( fabs( value[k] - destValue[k] ) <= max,
                  "Comparison of initial data and decompressed data failed" );
    }
}
}

int main( int argc, char **argv )
{
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    eq::Strings images;
    eq::Strings candidates = eq::base::searchDirectory( "images", "*.rgb");
    stde::usort( candidates ); // have a predictable order
    for( eq::Strings::const_iterator i = candidates.begin();
        i != candidates.end(); ++i )
    {
        const std::string& filename = *i;
        const size_t decompPos = filename.find( "out_" );
        if( decompPos == std::string::npos )
            images.push_back( "images/" + filename );
    }

    candidates = eq::base::searchDirectory( "../compositor", "Result*.rgb" );
    stde::usort( candidates ); // have a predictable order
    for( eq::Strings::const_iterator i = candidates.begin();
        i != candidates.end(); ++i )
    {
        const std::string& filename = *i;
        const size_t decompPos = filename.find( "out_" );
        if( decompPos == std::string::npos )
            images.push_back( "../compositor/" + filename );
    }
    TEST( !images.empty( ));

    eq::base::Clock clock;
    eq::Image image;
    eq::Image destImage;
    
    std::cout.setf( std::ios::right, std::ios::adjustfield );
    std::cout.precision( 5 );
    std::cout << "COMPRESSOR,                            IMAGE,       SIZE, A,"
              << " COMPRESSED,     t_comp,   t_decomp" << std::endl;

    // For each compressor...
    std::vector< uint32_t > names( _getCompressorNames( ));
    for( std::vector< uint32_t >::const_iterator i = names.begin();
         i != names.end(); ++i )
    {
        const uint32_t name = *i;

        // For alpha, ignore alpha...
        bool alpha = true;
        while( true )
        {
            uint64_t totalSize( 0 );
            uint64_t totalCompressedSize( 0 );
            float totalCompressTime( 0.f );
            float totalDecompressTime( 0.f );

            // For each image
            for( eq::Strings::const_iterator j = images.begin();
                 j != images.end(); ++j )
            {
                const std::string& filename = *j;
                const size_t depthPos = filename.find( "depth" );
                const eq::Frame::Buffer buffer = (depthPos==std::string::npos) ?
                    eq::Frame::BUFFER_COLOR : eq::Frame::BUFFER_DEPTH;

                TEST( image.readImage( filename, buffer ));

                if( image.ignoreAlpha() && 
                    ( buffer != eq::Frame::BUFFER_COLOR || !image.hasAlpha( )))
                {
                    continue; // Ignoring alpha doesn't make sense
                }

                const std::vector<uint32_t> compressors(
                    image.findCompressors( buffer ));

                if( std::find( compressors.begin(), compressors.end(), name ) ==
                    compressors.end( ))
                {
                    continue; // Compressor not suitable for current image
                }

                image.allocCompressor( buffer, name );
                destImage.setPixelViewport( image.getPixelViewport( ));
            
                const uint32_t size = image.getPixelDataSize( buffer );
                
#ifndef NDEBUG
                // touch memory once
                destImage.setPixelData( buffer,
                                        image.compressPixelData( buffer ));
                // force recompression
                if( image.ignoreAlpha( ))
                {
                    image.enableAlphaUsage();
                    image.disableAlphaUsage();
                }
                else
                {
                    image.disableAlphaUsage();
                    image.enableAlphaUsage();
                }
#endif

                // Compress
                clock.reset();
                const eq::Image::PixelData& compressedPixels =
                    image.compressPixelData( buffer );
                const float compressTime = clock.getTimef();

                TEST( compressedPixels.compressorName == name );
                TEST( compressedPixels.compressedSize.size() ==
                      compressedPixels.compressedData.size( ));
                TESTINFO( name == compressedPixels.compressorName,
                          name << " != " << compressedPixels.compressorName );

                uint32_t compressedSize = 0;
                if( compressedPixels.compressorName == EQ_COMPRESSOR_NONE )
                    compressedSize = size;
                else 
                {
#ifdef WRITE_COMPRESSED
                    std::ofstream comp( std::string( filename+".comp" ).c_str(),
                                        std::ios::out | std::ios::binary ); 
                    TEST( comp.is_open( ));
                    std::vector< void* >::const_iterator compData = 
                        compressedPixels.compressedData.begin();
#endif

                    for( std::vector< uint64_t >::const_iterator k = 
                             compressedPixels.compressedSize.begin();
                         k != compressedPixels.compressedSize.end(); ++k )
                    {
                        compressedSize += *k;
#ifdef WRITE_COMPRESSED
                        comp.write( reinterpret_cast<const char*>( *compData ),
                                    *k );
                        ++compData;
#endif
                    }
#ifdef WRITE_COMPRESSED
                    comp.close();
#endif
                }

                // Decompress
                clock.reset();
                destImage.setPixelData( buffer, compressedPixels );
                const float decompressTime = clock.getTimef();

                std::cout  << "0x" << std::setw(3) << std::setfill( '0' )
                           << std::hex << name << std::dec << std::setfill(' ')
                           << ", " << std::setw(37) << filename
                           << ", " << std::setw(10) << size << ", "
                           << !image.ignoreAlpha() << ", " << std::setw(10) 
                           << compressedSize << ", " << std::setw(10)
                           << compressTime << ", " << std::setw(10)
                           << decompressTime << std::endl;

                totalSize += size;
                totalCompressedSize += compressedSize;
                totalCompressTime   += compressTime;
                totalDecompressTime += decompressTime;

#ifdef WRITE_DECOMPRESSED
                destImage.writeImage( eq::base::getDirname( filename ) +
                                      "/out_" + 
                                      eq::base::getFilename( filename ),
                                      buffer );
#endif

#ifdef COMPARE_RESULT
                const uint8_t* data = image.getPixelPointer( buffer );
                const uint8_t* destData = destImage.getPixelPointer( buffer );
                const float quality = _getCompressorQuality( name );
                uint8_t channelSize = 0;
                switch( image.getExternalFormat( buffer ))
                {      
                    case EQ_COMPRESSOR_DATATYPE_RGBA:
                    case EQ_COMPRESSOR_DATATYPE_BGRA:
                    case EQ_COMPRESSOR_DATATYPE_RGB:
                    case EQ_COMPRESSOR_DATATYPE_BGR:
                    case EQ_COMPRESSOR_DATATYPE_RGB10_A2 :
                    case EQ_COMPRESSOR_DATATYPE_BGR10_A2 :
                        channelSize = 1;
                        break;
                    case EQ_COMPRESSOR_DATATYPE_BGRA32F:
                    case EQ_COMPRESSOR_DATATYPE_BGR32F:
                    case EQ_COMPRESSOR_DATATYPE_RGBA32F:
                    case EQ_COMPRESSOR_DATATYPE_RGB32F:
                    case EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT:
                        channelSize = 4;
                        break;
                    case EQ_COMPRESSOR_DATATYPE_RGB16F:
                    case EQ_COMPRESSOR_DATATYPE_RGBA16F:
                    case EQ_COMPRESSOR_DATATYPE_BGR16F:
                    case EQ_COMPRESSOR_DATATYPE_BGRA16F:
                        channelSize = 2;
                        break;
                    default:
                        EQERROR << "Unknown image pixel data type" << std::endl;
                        channelSize = 0;
                }
                const int64_t nElem = size / channelSize;
                
                switch( channelSize )
                {
                    case 1:
                        _compare< uint8_t >( data, destData, buffer,
                                             image.ignoreAlpha(), nElem,
                                             quality );
                        break;
                    case 2:
                        if( quality == 1.f )
                            _compare< uint16_t >( data, destData, buffer,
                                              image.ignoreAlpha(), nElem,
                                              quality );
                        break;
                    case 4:
                        _compare< float >( data, destData, buffer,
                                           image.ignoreAlpha(), nElem,
                                           quality );
                        break;
                    default:
                        break;
                }
#endif
            }

            if( totalSize > 0 )
                std::cout
                    << "0x" << std::setw(3) << std::setfill( '0' ) << std::hex
                    << name << std::dec << std::setfill(' ')
                    << ",                                 Total, "
                    << std::setw(10) << totalSize << ", "
                    << !image.ignoreAlpha() << ", " << std::setw(10)
                    << totalCompressedSize << ", " << std::setw(10)
                    << totalCompressTime << ", " << std::setw(10)
                    << totalDecompressTime << std::endl << std::endl;
            
            if( alpha )
            {
                alpha = false;
                image.disableAlphaUsage();
                destImage.disableAlphaUsage();
            }
            else
            {
                image.enableAlphaUsage();
                destImage.enableAlphaUsage();
                break;
            }
        }
    }

    image.flush();
    destImage.flush();
    eq::exit();

    return EXIT_SUCCESS;
}

