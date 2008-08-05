
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "image.h"

#include "frame.h"
#include "frameData.h"
#include "pixel.h"
#include "log.h"
#include "windowSystem.h"

#include <eq/base/omp.h>
#include <eq/net/node.h>

#include <fstream>

#ifdef WIN32
#  include <malloc.h>
#  define bzero( ptr, size ) memset( ptr, 0, size );
#else
#  include <alloca.h>
#endif

using namespace std;

namespace eq
{

namespace
{
const uint64_t _rleMarker = 0xF3C553FF64F6477Full; // just a random number
}

Image::Image()
        : _glObjects( 0 )
{
    reset();
}

Image::~Image()
{
}

void Image::reset()
{
    _colorPixels.data.format = GL_BGRA;
    _colorPixels.data.type   = GL_UNSIGNED_BYTE;
    _depthPixels.data.format = GL_DEPTH_COMPONENT;
    _depthPixels.data.type   = GL_FLOAT;

    _usePBO = false;

    setPixelViewport( PixelViewport( ));
}

uint32_t Image::getDepth( const Frame::Buffer buffer ) const
{
    uint32_t depth = 0;
    switch( getFormat( buffer ))
    {
        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA:
        case GL_DEPTH_STENCIL_NV:
            depth = 4;
            break;

        case GL_RGB:
        case GL_BGR:
            depth = 3;
            break;

        case GL_DEPTH_COMPONENT:
            depth = 1;
            break;

        default :
            EQWARN << "Unknown number of components for format "
                   << getFormat( buffer ) << " of buffer " << buffer << endl;
            EQUNIMPLEMENTED;
    }

    switch( getType( buffer ))
    {
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return depth; // depth *= 1;

        case GL_FLOAT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_24_8_NV:
            return depth * 4;

        default :
            EQUNIMPLEMENTED;
    }
    return 0;
}

Image::Pixels& Image::_getPixels( const Frame::Buffer buffer )
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _colorPixels;

        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _depthPixels;
    }
}

Image::CompressedPixels& Image::_getCompressedPixels(
    const Frame::Buffer buffer )
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _compressedColorPixels;

        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _compressedDepthPixels;
    }
}

const Image::Pixels& Image::_getPixels( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _colorPixels;

        case Frame::BUFFER_DEPTH:
        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _depthPixels;
    }
}

const Image::CompressedPixels& Image::_getCompressedPixels(
    const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _compressedColorPixels;

        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _compressedDepthPixels;
    }
}

void Image::setFormat( const Frame::Buffer buffer, const uint32_t format )
{
    Pixels& pixels = _getPixels( buffer );
    pixels.data.format = format;
    pixels.valid  = false;
}

void Image::setType( const Frame::Buffer buffer, const uint32_t type )
{
    Pixels& pixels = _getPixels( buffer );
    pixels.data.type  = type;
    pixels.valid = false;
}

uint32_t Image::getFormat( const Frame::Buffer buffer ) const
{
    const Pixels& pixels = _getPixels( buffer );
    return pixels.data.format;
}

uint32_t Image::getType( const Frame::Buffer buffer ) const
{
    const Pixels& pixels = _getPixels( buffer );
    return pixels.data.type;
}

bool Image::hasAlpha() const
{
    switch( getFormat( Frame::BUFFER_COLOR ))
    {
        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA:
            return true;

        default:
            return false;
    }
}

const uint8_t* Image::getPixelPointer( const Frame::Buffer buffer ) const
{
    EQASSERT( hasPixelData( buffer ));
    EQASSERT( _getPixels( buffer ).data.chunks.size() == 1 );

    return _getPixels( buffer ).data.chunks[0];
}

uint8_t* Image::getPixelPointer( const Frame::Buffer buffer )
{
    EQASSERT( hasPixelData( buffer ));
    EQASSERT( _getPixels( buffer ).data.chunks.size() == 1 );

    return _getPixels( buffer ).data.chunks[0];
}

const Image::PixelData& Image::getPixelData( const Frame::Buffer buffer ) const
{
    EQASSERT(hasPixelData(buffer));
    return _getPixels( buffer ).data;
}

void Image::startReadback( const uint32_t buffers, const PixelViewport& pvp,
                           Window::ObjectManager* glObjects )
{
    EQASSERT( glObjects );
    EQASSERTINFO( !_glObjects, "Another readback in progress?" );
    EQLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers
                          << endl;

    _glObjects = glObjects;
    _pvp       = pvp;

    _colorPixels.valid = false;
    _depthPixels.valid = false;

    if( buffers & Frame::BUFFER_COLOR )
        _startReadback( Frame::BUFFER_COLOR );

    if( buffers & Frame::BUFFER_DEPTH )
        _startReadback( Frame::BUFFER_DEPTH );

    _pvp.x = 0;
    _pvp.y = 0;
}

void Image::syncReadback()
{
    _syncReadback( Frame::BUFFER_COLOR );
    _syncReadback( Frame::BUFFER_DEPTH );
    _glObjects = 0;
}

void Image::Pixels::resize( uint32_t size )
{
    valid = true;

    data.chunkSizes[0] = size;

    if( maxSize >= size )
        return;

    // round to next 8-byte alignment (compress uses 8-byte tokens)
    if( size%8 )
        size += 8 - (size%8);

    delete [] data.chunks[0];
    data.chunks[0] = new uint8_t[size];
    maxSize = size;
}

const void* Image::_getPBOKey( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return ( reinterpret_cast< const char* >( this ) + 1 );
        case Frame::BUFFER_DEPTH:
            return ( reinterpret_cast< const char* >( this ) + 2 );
        default:
            EQUNIMPLEMENTED;
            return ( reinterpret_cast< const char* >( this ) + 0 );
    }
}


void Image::_startReadback( const Frame::Buffer buffer )
{
    Pixels&           pixels           = _getPixels( buffer );
    const size_t      size             = getPixelDataSize( buffer );

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    compressedPixels.valid = false;

    if( _usePBO && _glObjects->supportsBuffers( )) // use async PBO readback
    {
        pixels.reading = true;

        const void* bufferKey = _getPBOKey( buffer );
        GLuint pbo = _glObjects->obtainBuffer( bufferKey );

        EQ_GL_CALL( glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo ));
        if( pixels.pboSize < size )
        {
            EQ_GL_CALL( glBufferData( GL_PIXEL_PACK_BUFFER, size, 0,
                                      GL_DYNAMIC_READ ));
            pixels.pboSize = size;
        }

        EQ_GL_CALL( glReadPixels( _pvp.x, _pvp.y, _pvp.w, _pvp.h,
                                  getFormat( buffer ), getType( buffer ), 0 ));
        EQ_GL_CALL( glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 ));
    }
    else
    {
        pixels.resize( size );
        glReadPixels( _pvp.x, _pvp.y, _pvp.w, _pvp.h, getFormat( buffer ),
                      getType( buffer ), pixels.data.chunks[0] );
        pixels.valid = true;
    }
}

void Image::_syncReadback( const Frame::Buffer buffer )
{
    Pixels& pixels = _getPixels( buffer );
    if( pixels.valid || !pixels.reading )
        return;

    // async readback only possible when PBOs are used and supported
    EQASSERT( _usePBO );
    EQASSERT( _glObjects->supportsBuffers( ));
    EQ_GL_ERROR( "before Image::_syncReadback" );

    const size_t size      = getPixelDataSize( buffer );
    const void*  bufferKey = _getPBOKey( buffer );
    GLuint       pbo       = _glObjects->getBuffer( bufferKey );
    EQASSERT( pbo != Window::ObjectManager::FAILED );

    pixels.resize( size );

    EQ_GL_CALL( glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo ));
    const void* data = glMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY );
    EQ_GL_ERROR( "glMapBuffer" );
    EQASSERT( data );

    memcpy( pixels.data.chunks[0], data, size );

    glUnmapBuffer( GL_PIXEL_PACK_BUFFER );
    glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

    pixels.reading = false;
}

void Image::setPixelViewport( const PixelViewport& pvp )
{
    _pvp = pvp;
    _colorPixels.valid = false;
    _depthPixels.valid = false;
    _compressedColorPixels.valid = false;
    _compressedDepthPixels.valid = false;
}

void Image::clearPixelData( const Frame::Buffer buffer )
{
    const uint32_t size  = getPixelDataSize( buffer );
    if( size == 0 )
        return;

    Pixels& pixels = _getPixels( buffer );

    pixels.resize( size );

    if( buffer == Frame::BUFFER_DEPTH )
    {
#ifdef LEOPARD
        void*       data = pixels.data.chunks[0];
        const float one  = 1.0f;
        memset_pattern4( data, &one, size );
#else
        EQASSERT( (size % 4) == 0 );
        const size_t nWords = (size >> 2);
        float*       data   = reinterpret_cast< float* >(pixels.data.chunks[0]);
        for( size_t i =0; i < nWords; ++i )
            data[i] = 1.0f;
#endif

    }
    else
    {
        if( getDepth( Frame::BUFFER_COLOR ) == 4 )
        {
            uint8_t* data = pixels.data.chunks[0];
#ifdef LEOPARD
            const unsigned char pixel[4] = { 0, 0, 0, 255 };
            memset_pattern4( data, &pixel, size );
#else
            bzero( data, size );

            if( getDepth( Frame::BUFFER_COLOR ) == 4 )
#pragma omp parallel for
                for( ssize_t i = 3; i < size; i+=4 )
                    data[i] = 255;
#endif
        }
        else
            bzero( pixels.data.chunks[0], size );
    }

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    compressedPixels.valid = false;
}

void Image::setPixelData( const Frame::Buffer buffer, const uint8_t* data )
{
    const uint32_t size  = getPixelDataSize( buffer );
    if( size == 0 )
        return;

    Pixels& pixels = _getPixels( buffer );
    pixels.resize( size );
    memcpy( pixels.data.chunks[0], data, size );

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    compressedPixels.valid = false;
}

void Image::setPixelData( const Frame::Buffer buffer, const PixelData& pixels )
{
    setFormat( buffer, pixels.format );
    setType( buffer, pixels.type );

    const uint32_t size = getPixelDataSize( buffer );
    if( size == 0 )
        return;

    if( !pixels.compressed )
    {
        EQASSERT( pixels.chunks.size() == 1 );
        EQASSERT( size == pixels.chunkSizes[0] );

        setPixelData( buffer, pixels.chunks[0] );
        return;
    }

    EQASSERT( getDepth( buffer ) == 4 )     // may change with RGB format
    EQASSERT( size < (100 * 1024 * 1024 )); // < 100MB

    Pixels& outPixels = _getPixels( buffer );
	EQASSERT( size > 0 );
    outPixels.resize( size );

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    compressedPixels.valid = false;

    // Get number of blocks in compressed data
    const ssize_t nChunks  = pixels.chunks.size();
    uint64_t**    outTable = static_cast< uint64_t** >(
        alloca( nChunks * sizeof( uint64_t* )));

    // Prepare table with input pointer into decompressed data
    //   Needed since decompress loop is parallelized
    {
        uint8_t* out = outPixels.data.chunks[0];
        for( ssize_t i = 0; i < nChunks; ++i )
        {
            outTable[i] = reinterpret_cast< uint64_t* >( out );

            const uint64_t* in  = 
                reinterpret_cast< const uint64_t* >( pixels.chunks[i] );
            const uint64_t nWords = in[0];
            out += nWords * sizeof( uint64_t );
        }
        EQASSERTINFO( size >= (uint32_t)( out - outPixels.data.chunks[0] - 7 ),
                      "Pixel data size does not match expected image size: "
                      << size << " != " 
                      << (uint32_t)( out - outPixels.data.chunks[0] ));
    }

    // decompress each block
    // On OS X the loop is sometimes slower when parallelized. Investigate this!
//#pragma omp parallel for
    for( ssize_t i = 0; i < nChunks; ++i )
    {
        uint64_t* in  = reinterpret_cast< uint64_t* >( pixels.chunks[i] );
        uint64_t* out = outTable[i];

        uint32_t       outPos = 0;
        const uint64_t endPos = in[0];
        uint32_t       inPos  = 1;

        EQVERB << "In chunk " << i << " size " << pixels.chunkSizes[i] 
               << " @ " << (void*)in << endl;
        EQVERB << "Out chunk " << i << " size " << endPos * sizeof( uint64_t )
               << " @ " << (void*)out << endl;

        while( outPos < endPos )
        {
            const uint64_t token = in[inPos++];
            if( token == _rleMarker )
            {
                const uint64_t symbol = in[inPos++];
                const uint64_t nSame  = in[inPos++];
                EQASSERT( outPos + nSame <= endPos );

                for( uint32_t j = 0; j<nSame; ++j )
                    out[outPos++] = symbol;
            }
            else // symbol
                out[outPos++] = token;

            EQASSERTINFO( ((outPos-1) << 3) <= outPixels.maxSize,
                          "Overwrite array bounds during image decompress" );
        }
        EQASSERT( outPos == endPos );
    }
}

Image::PixelData::~PixelData()
{
    while( !chunks.empty( ))
    {
        delete [] chunks.back();
        chunks.pop_back();
    }
}

const Image::PixelData& Image::compressPixelData( const Frame::Buffer buffer )
{
    const uint32_t size = getPixelDataSize( buffer );

    EQASSERT( size > 0 );
    EQASSERT( getDepth( buffer ) == 4 )     // may change with RGB format

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    if( compressedPixels.valid )
        return compressedPixels.data;

    compressedPixels.data.format     = getFormat( buffer );
    compressedPixels.data.type       = getType( buffer );
    compressedPixels.data.compressed = true;

    const uint64_t* data     = 
        reinterpret_cast<const uint64_t*>( getPixelPointer( buffer ));
    const uint32_t  nWords   = (size%8) ? (size>>3)+1 : (size>>3);

    // determine number of chunks and set up output data structure
#ifdef EQ_USE_OPENMP
    const ssize_t nChunks = base::OMP::getNThreads() * 4;
#else
    const ssize_t nChunks = 1;
#endif
    const uint32_t maxChunkSize = (size/nChunks + 1) << 1;

    for( ssize_t i = 0; i<nChunks; ++i )
    {
        EQASSERT( compressedPixels.data.chunks.size() ==
                  compressedPixels.data.chunkSizes.size( ));
        EQASSERT( compressedPixels.data.chunks.size() ==
                  compressedPixels.chunkMaxSizes.size( ));

        if( compressedPixels.data.chunks.size() <= static_cast< size_t >( i ))
        {
            compressedPixels.data.chunks.push_back( new uint8_t[maxChunkSize] );
            compressedPixels.data.chunkSizes.push_back( 0 );
            compressedPixels.chunkMaxSizes.push_back( maxChunkSize );
        }
        else
        {
            compressedPixels.data.chunkSizes[i] = 0;
            if( compressedPixels.chunkMaxSizes[i] < maxChunkSize )
            {
                delete [] compressedPixels.data.chunks[i];
                compressedPixels.data.chunks[i]   = new uint8_t[ maxChunkSize ];
                compressedPixels.chunkMaxSizes[i] = maxChunkSize;
            }
        }
    }


    const float width = static_cast< float >( nWords ) /
                        static_cast< float >( nChunks );

//#pragma omp parallel for
    for ( ssize_t i = 0; i < nChunks; ++i )
    {
        const uint32_t startIndex = i * width;
        const uint32_t endIndex   = (i+1) * width;
        uint64_t*      out        =
            reinterpret_cast< uint64_t* >( compressedPixels.data.chunks[i] );

        compressedPixels.data.chunkSizes[i] =
            _compressPixelData( &data[ startIndex ], endIndex-startIndex, out );

        EQVERB << "In chunk " << i << " size "
               << (endIndex-startIndex) * sizeof( uint64_t ) << " @ " 
               << (void*)&data[ startIndex ] << endl
               << "Out chunk " << i << " size "
               << compressedPixels.data.chunkSizes[i] << " @ " << (void*)out
               << endl;
    }

    compressedPixels.valid = true;
    return compressedPixels.data;
}

#define WRITE_OUTPUT                                                    \
    {                                                                   \
        if( lastSymbol == _rleMarker )                                  \
        {                                                               \
            out[ outPos++ ] = _rleMarker;                               \
            out[ outPos++ ] = lastSymbol;                               \
            out[ outPos++ ] = nSame;                                    \
        }                                                               \
        else                                                            \
            switch( nSame )                                             \
            {                                                           \
                case 0:                                                 \
                    EQUNREACHABLE;                                      \
                    break;                                              \
                case 3:                                                 \
                    out[ outPos++ ] = lastSymbol; /* fall through */    \
                case 2:                                                 \
                    out[ outPos++ ] = lastSymbol; /* fall through */    \
                case 1:                                                 \
                    out[ outPos++ ] = lastSymbol;                       \
                    break;                                              \
                default:                                                \
                    out[ outPos++ ] = _rleMarker;                       \
                    out[ outPos++ ] = lastSymbol;                       \
                    out[ outPos++ ] = nSame;                            \
                    break;                                              \
            }                                                           \
        EQASSERTINFO( nWords<<1 >= outPos,                             \
                      "Overwrite array bounds during image compress" ); \
    }

uint32_t Image::_compressPixelData( const uint64_t* data, const uint32_t nWords,
                                    uint64_t* out )
{
    out[ 0 ] = nWords;

    uint32_t outPos     = 1;
    uint32_t nSame      = 1;
    uint64_t lastSymbol = data[0];

    for( uint32_t i=1; i<nWords; ++i )
    {
        const uint64_t symbol = data[i];

        if( symbol == lastSymbol )
            ++nSame;
        else
        {
            WRITE_OUTPUT;
            lastSymbol = symbol;
            nSame      = 1;
        }
    }

    WRITE_OUTPUT;
    return (outPos<<3);
}

//---------------------------------------------------------------------------
// IO
//---------------------------------------------------------------------------
void Image::writeImages( const std::string& filenameTemplate ) const
{
    if( _colorPixels.valid )
        writeImage( filenameTemplate + "_color.rgb", Frame::BUFFER_COLOR );
    if( _depthPixels.valid )
        writeImage( filenameTemplate + "_depth.rgb", Frame::BUFFER_DEPTH );
}

#define SWAP_SHORT(v) ( v = (v&0xff) << 8 | (v&0xff00) >> 8 )
#define SWAP_INT(v)   ( v = (v&0xff) << 24 | (v&0xff00) << 8 |      \
                        (v&0xff0000) >> 8 | (v&0xff000000) >> 24)

#ifdef WIN32
#  pragma pack(1)
#endif
struct RGBHeader
{
    RGBHeader()
        : magic(474),
          compression(0),
          bytesPerChannel(1),
          nDimensions(3),
          width(0),
          height(0),
          depth(0),
          minValue(0),
          maxValue(255),
          colorMode(0)
        {
            filename[0] = '\0';
        }

        /**
         * Convert to and from big endian by swapping bytes on little endian
         * machines.
         */
        void convert()
        {
#if defined(__i386__) || defined(__amd64__) || defined (__ia64) || defined(WIN32)
            SWAP_SHORT(magic);
            SWAP_SHORT(nDimensions);
            SWAP_SHORT(width);
            SWAP_SHORT(height);
            SWAP_SHORT(depth);
            SWAP_INT(minValue);
            SWAP_INT(maxValue);
            SWAP_INT(colorMode);
#endif
        }

    unsigned short magic;
    char compression;
    char bytesPerChannel;
    unsigned short nDimensions;
    unsigned short width;
    unsigned short height;
    unsigned short depth;
    unsigned minValue;
    unsigned maxValue;
    char unused[4];
    char filename[80];
    unsigned colorMode;
    char fill[404];
}
#ifndef WIN32
  __attribute__((packed))
#endif
;

void Image::writeImage( const std::string& filename,
                        const Frame::Buffer buffer ) const
{
    const size_t   nPixels = _pvp.w * _pvp.h;
    const Pixels&  pixels  = _getPixels( buffer );

    if( nPixels == 0 || !pixels.valid )
        return;

    ofstream image( filename.c_str(), ios::out | ios::binary );
    if( !image.is_open( ))
    {
        EQERROR << "Can't open " << filename << " for writing" << endl;
        return;
    }

    const size_t depth   = getDepth( buffer );
    RGBHeader    header;

    header.width  = _pvp.w;
    header.height = _pvp.h;
    header.depth  = depth;
    strncpy( header.filename, filename.c_str(), 80 );

    header.convert();
    image.write( reinterpret_cast<const char *>( &header ), sizeof( header ));

    // Each channel is saved separately
    const size_t   nBytes = nPixels * depth;
    const uint8_t* data   = getPixelPointer( buffer );

    switch( getFormat( buffer ))
    {
        case GL_BGR:
            for( size_t j = 2; j < nBytes; j += depth )
                image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
            for( size_t j = 1; j < nBytes; j += depth )
                image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
            for( size_t j = 0; j < nBytes; j += depth )
                image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
            break;

        case GL_BGRA:
            for( size_t j = 2; j < nBytes; j += depth )
                image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
            for( size_t j = 1; j < nBytes; j += depth )
                image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
            for( size_t j = 0; j < nBytes; j += depth )
                image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
            for( size_t j = 3; j < nBytes; j += depth )
                image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
            break;

        default:
            for( size_t i = 0; i < depth; ++i )
                for( size_t j = i; j < nBytes; j += depth )
                    image.write( reinterpret_cast<const char*>( &data[j] ), 1 );
    }

    image.close();
}

bool Image::readImage( const std::string& filename, const Frame::Buffer buffer )
{
    ifstream image( filename.c_str(), ios::in | ios::binary );
    if( !image.is_open( ))
    {
        EQERROR << "Can't open " << filename << " for reading" << endl;
        return false;
    }

    RGBHeader header;
    image.read( reinterpret_cast<char *>( &header ), sizeof( header ));

    if( image.bad() || image.eof( ))
    {
        EQERROR << "Error during image header input " << filename << endl;
        image.close();
        return false;
    }

    header.convert();

    if( header.magic != 474)
    {
        EQERROR << "Bad magic number " << filename << endl;
        image.close();
        return false;
    }
    if( header.width == 0 || header.height == 0 )
    {
        EQERROR << "Zero-sized image " << filename << endl;
        image.close();
        return false;
    }
    if( header.depth != getDepth( buffer ))
    {
        EQERROR << "Pixel depth mismatch " << filename << endl;
        image.close();
        return false;
    }
    if( header.compression != 0)
    {
        EQERROR << "Unsupported compression " << filename << endl;
        image.close();
        return false;
    }

    const size_t depth = header.depth;

    if( header.bytesPerChannel != 1 || header.nDimensions != 3 ||
        header.minValue != 0 || header.maxValue != 255 ||
        header.colorMode != 0 ||
        ( buffer == Frame::BUFFER_COLOR && depth != 3 && depth != 4 ) ||
        ( buffer == Frame::BUFFER_DEPTH && depth != 4 ))
    {
        EQERROR << "Unsupported image type " << filename << endl;
        image.close();
        return false;
    }

    const size_t     nPixels = header.width * header.height;
    const size_t     nBytes  = nPixels * depth;
    Pixels           pixels;

    pixels.resize( nBytes );
    uint8_t* data = pixels.data.chunks[0];

    // Each channel is saved separately
    for( size_t i = 0; i < depth; ++i )
        for( size_t j = i; j < nBytes; j += depth )
            image.read( reinterpret_cast<char*>( &data[j] ), 1 );

    if( image.bad() || image.eof( ))
    {
        EQERROR << "Error during image data input " << filename << endl;
        image.close();
        return false;
    }

    const PixelViewport pvp( 0, 0, header.width, header.height );
    if( pvp != getPixelViewport( ))
        setPixelViewport( pvp );

    if( buffer == Frame::BUFFER_COLOR )
    {
        pixels.data.format = (depth==3) ? GL_RGB : GL_RGBA;
        pixels.data.type   = GL_UNSIGNED_BYTE;
    }
    else
    {
        EQASSERT( buffer == Frame::BUFFER_DEPTH );
        pixels.data.format = GL_DEPTH_COMPONENT;
        pixels.data.type   = GL_FLOAT;
    }

    setPixelData( buffer, pixels.data );

    image.close();
    return true;
}

std::ostream& operator << ( std::ostream& os, const Image* image )
{
    os << "image " << image->_pvp;
    return os;
}
}
