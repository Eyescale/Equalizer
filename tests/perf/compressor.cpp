
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#define TEST_RUNTIME 1200 // seconds
#include <lunchbox/test.h>

#include <eq/frame.h>    // enum Eye
#include <eq/image.h>
#include <eq/init.h>
#include <eq/nodeFactory.h>
#include <eq/pixelData.h>

#include <co/global.h>

#include <lunchbox/clock.h>
#include <lunchbox/file.h>
#include <pression/plugin.h>
#include <pression/pluginRegistry.h>
#include <pression/pluginVisitor.h>
#include <pression/plugins/compressor.h>

#include <numeric>
#include <fstream>

// Tests the functionality and speed of the image compression.
//#define WRITE_DECOMPRESSED
//#define WRITE_COMPRESSED

#ifndef NDEBUG
#  define COMPARE_RESULT
#endif

namespace
{
class Finder : public pression::ConstPluginVisitor
{
public:
    virtual lunchbox::VisitorResult visit( const pression::Plugin&,
                                           const EqCompressorInfo& info )
    {
        if( !(info.capabilities & EQ_COMPRESSOR_TRANSFER) )
            names.push_back( info.name );
        return lunchbox::TRAVERSE_CONTINUE;
    }

    std::vector< uint32_t > names;
};

template< typename T >
static void _compare( const void* data, const void* destData,
                      const eq::Frame::Buffer buffer, const bool useAlpha,
                      const int64_t nElem, const float quality )
{
    const T* destValue = reinterpret_cast< const T* >( destData );
    const T* value = reinterpret_cast< const T* >( data );
    double error = 0.;

#pragma omp parallel for reduction(+ : error)
    for( int64_t k = 0; k < nElem; ++k )
    {
        if( !useAlpha && buffer == eq::Frame::BUFFER_COLOR )
        {
            // Don't test alpha if alpha is ignored
            if( k % 4 == 3 )
                continue;
        }

        error += ::fabs( double( value[k] ) - double( destValue[k] ));
    }

    const double max = ( 1.f - quality ) * std::numeric_limits< T >::max() *
                       double( nElem );
    TESTINFO( error <= max,
              "Comparison of initial data and decompressed data failed" <<
              ", error " << error << " max " << max );
}
}

int main( int argc, char **argv )
{
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    eq::Strings images;
    eq::Strings candidates = lunchbox::searchDirectory( "images", ".*\\.rgb");
    stde::usort( candidates ); // have a predictable order
    for( eq::StringsCIter i = candidates.begin(); i != candidates.end(); ++i )
    {
        const std::string& filename = *i;
        const size_t decompPos = filename.find( "out_" );
        if( decompPos == std::string::npos )
            images.push_back( "images/" + filename );
    }

    candidates = lunchbox::searchDirectory( ".", "Result.*\\.rgb" );
    stde::usort( candidates ); // have a predictable order
    for( eq::Strings::const_iterator i = candidates.begin();
        i != candidates.end(); ++i )
    {
        const std::string& filename = *i;
        const size_t decompPos = filename.find( "out_" );
        if( decompPos == std::string::npos )
            images.push_back( filename );
    }
    TEST( !images.empty( ));

    lunchbox::Clock clock;
    eq::Image image;
    eq::Image destImage;

    // For each compressor...
    const pression::PluginRegistry& registry = co::Global::getPluginRegistry();
    Finder finder;
    registry.accept( finder );
    std::vector< uint32_t >& names = finder.names;
    std::cout << images.size() << " test images X " << names.size()
              << " plugins" << std::endl;
    TESTINFO( names.size() > 23, names.size( ));

    std::cout.setf( std::ios::right, std::ios::adjustfield );
    std::cout.precision( 5 );
    std::cout << "COMPRESSOR,                            IMAGE,       SIZE, A,"
              << " COMPRESSED,     t_comp,   t_decomp" << std::endl;

    for( std::vector< uint32_t >::const_iterator i = names.begin();
         i != names.end(); ++i )
    {
        const uint32_t name = *i;

        // For alpha, ignore alpha...
        image.setAlphaUsage( true );
        destImage.setAlphaUsage( true );
        while( true )
        {
            uint64_t totalSize( 0 );
            uint64_t totalCompressedSize( 0 );
            float totalCompressTime( 0.f );
            float totalDecompressTime( 0.f );

            // For each image
            for( eq::StringsCIter j = images.begin(); j != images.end(); ++j )
            {
                const std::string& filename = *j;
                const size_t depthPos = filename.find( "depth" );
                const eq::Frame::Buffer buffer = (depthPos==std::string::npos) ?
                    eq::Frame::BUFFER_COLOR : eq::Frame::BUFFER_DEPTH;

                TEST( image.readImage( filename, buffer ));

                if( !image.getAlphaUsage() &&
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
                image.setAlphaUsage( !image.getAlphaUsage( ));
                image.setAlphaUsage( !image.getAlphaUsage( ));
#endif

                // Compress
                clock.reset();
                const eq::PixelData& pixels = image.compressPixelData( buffer );
                const float compressTime = clock.getTimef();

                TEST( pixels.compressedData.compressor == name );
                TESTINFO( name == pixels.compressedData.compressor,
                          name << " != " << pixels.compressedData.compressor );

                uint32_t compressedSize = 0;
                if( pixels.compressedData.compressor == EQ_COMPRESSOR_NONE )
                    compressedSize = size;
                else
                {
                    compressedSize = pixels.compressedData.getSize();

#ifdef WRITE_COMPRESSED
                    std::ofstream comp( std::string( filename+".comp" ).c_str(),
                                        std::ios::out | std::ios::binary );
                    TEST( comp.is_open( ));

                    BOOST_FOREACH( const pression::CompressorChunk& chunk,
                                   pixels.compressedData.chunks )
                    {
                        comp.write( reinterpret_cast<const char*>( chunk.data ),
                                    chunk.getNumBytes( ));
                    }
                    comp.close();
#endif
                }

                // Decompress
                clock.reset();
                destImage.setPixelData( buffer, pixels );
                const float decompressTime = clock.getTimef();

                std::cout  << "0x" << std::setw(3) << std::setfill( '0' )
                           << std::hex << name << std::dec << std::setfill(' ')
                           << ", " << std::setw(37) << filename
                           << ", " << std::setw(10) << size << ", "
                           << image.getAlphaUsage() << ", " << std::setw(10)
                           << compressedSize << ", " << std::setw(10)
                           << compressTime << ", " << std::setw(10)
                           << decompressTime << std::endl;

                totalSize += size;
                totalCompressedSize += compressedSize;
                totalCompressTime   += compressTime;
                totalDecompressTime += decompressTime;

#ifdef WRITE_DECOMPRESSED
                destImage.writeImage( lunchbox::getDirname( filename ) +
                                      "/out_" +
                                      lunchbox::getFilename( filename ),
                                      buffer );
#endif

#ifdef COMPARE_RESULT
                const uint8_t* data = image.getPixelPointer( buffer );
                const uint8_t* destData = destImage.getPixelPointer( buffer );
                const float quality =
                    registry.findPlugin( name )->findInfo( name ).quality;
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
                        LBERROR << "Unknown image pixel data type" << std::endl;
                        channelSize = 0;
                }
                if( channelSize == 0 )
                    continue;

                const int64_t nElem = size / channelSize;

                switch( channelSize )
                {
                    case 1:
                        _compare< uint8_t >( data, destData, buffer,
                                             image.getAlphaUsage(), nElem,
                                             quality );
                        break;
                    case 2:
                        if( quality == 1.f )
                            _compare< uint16_t >( data, destData, buffer,
                                              image.getAlphaUsage(), nElem,
                                              quality );
                        break;
                    case 4:
                        _compare< float >( data, destData, buffer,
                                           image.getAlphaUsage(), nElem,
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
                    << image.getAlphaUsage() << ", " << std::setw(10)
                    << totalCompressedSize << ", " << std::setw(10)
                    << totalCompressTime << ", " << std::setw(10)
                    << totalDecompressTime << std::endl << std::endl;

            image.setAlphaUsage( !image.getAlphaUsage( ));
            destImage.setAlphaUsage( image.getAlphaUsage( ));
            if( image.getAlphaUsage( ))
                break;
        }
    }

    image.flush();
    destImage.flush();
    eq::exit();

    return EXIT_SUCCESS;
}
