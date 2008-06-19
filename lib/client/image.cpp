
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
    _colorPixels.format = GL_BGRA;
    _colorPixels.type   = GL_UNSIGNED_BYTE;
    _depthPixels.format = GL_DEPTH_COMPONENT;
    _depthPixels.type   = GL_FLOAT;

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
    pixels.format = format;
    pixels.valid  = false;
}

void Image::setType( const Frame::Buffer buffer, const uint32_t type )
{
    Pixels& pixels = _getPixels( buffer );
    pixels.type  = type;
    pixels.valid = false;
}

uint32_t Image::getFormat( const Frame::Buffer buffer ) const
{
    const Pixels& pixels = _getPixels( buffer );
    return pixels.format;
}

uint32_t Image::getType( const Frame::Buffer buffer ) const
{
    const Pixels& pixels = _getPixels( buffer );
    return pixels.type;
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

Image::Pixels::~Pixels()
{
    delete [] data;
	data = 0;
}

void Image::Pixels::resize( uint32_t size )
{
    valid = true;

    if( maxSize >= size )
        return;
    
    // round to next 8-byte alignment (compress uses 8-byte tokens)
    if( size%8 )
        size += 8 - (size%8);

    delete [] data;
    data    = new uint8_t[size];
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

//#define EQ_USE_PBO

void Image::_startReadback( const Frame::Buffer buffer )
{
    Pixels&           pixels           = _getPixels( buffer );
    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    const size_t      size             = _pvp.w * _pvp.h * getDepth( buffer );

    compressedPixels.valid = false;

#ifdef EQ_USE_PBO
    if( !_glObjects->supportsBuffers( ))
#endif
    {
        pixels.resize( size );
        glReadPixels( _pvp.x, _pvp.y, _pvp.w, _pvp.h, getFormat( buffer ),
                      getType( buffer ), pixels.data );
        pixels.valid = true;
        return;
    }

    // use PBOs
    pixels.reading = true;

    const void* bufferKey = _getPBOKey( buffer );
    GLuint pbo = _glObjects->obtainBuffer( bufferKey );

    glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo );
    if( pixels.maxSize < size )
        glBufferData( GL_PIXEL_PACK_BUFFER, size, 0, GL_DYNAMIC_READ );

    glReadPixels( _pvp.x, _pvp.y, _pvp.w, _pvp.h, getFormat( buffer ),
                  getType( buffer ), 0 );
    glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
}

void Image::_syncReadback( const Frame::Buffer buffer )
{
    Pixels& pixels = _getPixels( buffer );
    if( pixels.valid || !pixels.reading )
        return;

    // overlapped readbacks are only possible when PBOs are supported
    EQASSERT( _glObjects->supportsBuffers( ));

    const size_t size      = _pvp.w * _pvp.h * getDepth( buffer );
    const void*  bufferKey = _getPBOKey( buffer );
    GLuint       pbo       = _glObjects->getBuffer( bufferKey );
    EQASSERT( pbo != Window::ObjectManager::FAILED );

    pixels.resize( size );

    glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo );
    const void* data = glMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY );
    EQASSERT( data );

    memcpy( pixels.data, data, size );

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
        const float one = 1.0f;
        memset_pattern4( pixels.data, &one, size );
#else
        const size_t nWords = (size >> 2) + 1;
        float*       data   = reinterpret_cast< float* >( pixels.data );
        for( size_t i =0; i < nWords; ++i )
            data[i] = 1.0f;
#endif

    }
    else
    {
        if( getDepth( Frame::BUFFER_COLOR ) == 4 )
        {
#ifdef LEOPARD
            const unsigned char pixel[4] = { 0, 0, 0, 255 };
            memset_pattern4( pixels.data, &pixel, size );
#else
            bzero( pixels.data, size );

            if( getDepth( Frame::BUFFER_COLOR ) == 4 )
                for( uint32_t i = 3; i < size; i+=4 )
                    pixels.data[i] = 255;
#endif
        }
        else
            bzero( pixels.data, size );
    }

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    compressedPixels.valid = false;
}

void Image::setPixelData( const Frame::Buffer buffer, const uint8_t* data )
{
    const uint32_t size  = getPixelDataSize( buffer );
    if( size == 0 )
        return;
    
    Pixels&           pixels           = _getPixels( buffer );
    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );

    pixels.resize( size );
    memcpy( pixels.data, data, size );
    compressedPixels.valid = false;
}

uint32_t Image::decompressPixelData( const Frame::Buffer buffer,
                                     const uint8_t* data )
{
    const uint32_t size = getPixelDataSize( buffer );
    if( size == 0 )
        return 0;
    
    EQASSERT( getDepth( buffer ) == 4 )     // may change with RGB format
    EQASSERT( size < (100 * 1024 * 1024 )); // < 100MB

    Pixels&           pixels           = _getPixels( buffer );
    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );

	EQASSERT( size > 0 );
    pixels.resize( size );
    compressedPixels.valid = false;

    const uint64_t* in  = reinterpret_cast<const uint64_t*>( data );
    uint64_t*       out = reinterpret_cast<uint64_t*>( pixels.data );

    const uint64_t marker = in[0];    
    uint32_t       outpos = 0;
    const uint32_t endpos = (size%8) ? (size>>3)+1 : (size>>3);

    uint32_t i = 1;
    while( outpos < endpos )
    {
        const uint64_t token = in[i++];
        if( token == marker )
        {
            const uint64_t symbol = in[i++];
            const uint64_t nSame  = in[i++];
            for( uint32_t j = 0; j<nSame; ++j )
                out[outpos++] = symbol;
        }
        else // symbol
            out[outpos++] = token;
        EQASSERTINFO( ((outpos-1) << 3) <= pixels.maxSize, 
                      "Overwrite array bounds during image decompress" );
    }
    EQASSERT( outpos == endpos );

    return (i<<3);
}


const uint8_t* Image::compressPixelData( const Frame::Buffer buffer, 
                                         uint32_t& compressedSize )
{
    const uint32_t size = getPixelDataSize( buffer );
    if( size == 0 ) 
        return 0;

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    if( compressedPixels.valid )
    {
        compressedSize = compressedPixels.size;
        return compressedPixels.data;
    }

    EQASSERT( getDepth( buffer ) == 4 )     // may change with RGB format
    EQASSERT( size < (100 * 1024 * 1024 )); // < 100MB

    Pixels&         pixels   = _getPixels( buffer );
    const uint64_t* data     = reinterpret_cast<const uint64_t*>
                                   ( pixels.data );
    const uint32_t  nWords   = (size%8) ? (size>>3)+1 : (size>>3);

    // conservative output allocation
    compressedPixels.resize( nWords<<4 );
    uint64_t* out = reinterpret_cast<uint64_t*>( compressedPixels.data );

    const uint64_t marker = 0xF3C553FF64F6477Full; // just a random number
    out[ 0 ] = marker;

#ifdef EQ_USE_OPENMP
    const int   nThreads = eqBase::OMP::getNThreads();
    const float width    = static_cast< float >( nWords ) / 
                           static_cast< float >( nThreads );

    uint32_t*   outSizes = static_cast< uint32_t* >(
                               alloca( nThreads * sizeof( uint32_t )));
#  pragma omp parallel for
    for ( int i = 0; i < nThreads; ++i )
    {
        const int      threadNum  = omp_get_thread_num();
        const uint32_t startIndex = threadNum * width;
        const uint32_t endIndex   = (threadNum+1) * width;
        outSizes[ threadNum ] = _compressPixelData( &data[ startIndex ], 
                                                    endIndex - startIndex, 
                                                    marker,
                                                    &out[(startIndex<<1) + 1] );
    }

    uint32_t outSize = outSizes[0] + 8 /*marker header*/;

    for ( int i = 1; i < nThreads; ++i )
    {
        const uint32_t startIndex = i * width;
        memcpy( &compressedPixels.data[outSize], &out[ (startIndex<<1) + 1 ],
                outSizes[i] );
        outSize += outSizes[i];
    }

#else
    const uint32_t outSize = _compressPixelData( data, nWords, marker, &out[1] )
                                 + 8 /*marker header*/;
#endif

    compressedPixels.size = outSize;
    compressedSize        = outSize;

    return compressedPixels.data;
}

#define WRITE_OUTPUT                                                    \
    {                                                                   \
        if( lastSymbol == marker )                                      \
        {                                                               \
            out[ outpos++ ] = marker;                                   \
            out[ outpos++ ] = lastSymbol;                               \
            out[ outpos++ ] = nSame;                                    \
        }                                                               \
        else                                                            \
            switch( nSame )                                             \
            {                                                           \
                case 0:                                                 \
                    EQUNREACHABLE;                                      \
                    break;                                              \
                case 3:                                                 \
                    out[ outpos++ ] = lastSymbol; /* fall through */    \
                case 2:                                                 \
                    out[ outpos++ ] = lastSymbol; /* fall through */    \
                case 1:                                                 \
                    out[ outpos++ ] = lastSymbol;                       \
                    break;                                              \
                default:                                                \
                    out[ outpos++ ] = marker;                           \
                    out[ outpos++ ] = lastSymbol;                       \
                    out[ outpos++ ] = nSame;                            \
                    break;                                              \
            }                                                           \
        EQASSERTINFO( nWords<<1 >= outpos,                             \
                      "Overwrite array bounds during image compress" ); \
    }

uint32_t Image::_compressPixelData( const uint64_t* data, const uint32_t nWords,
                                    const uint64_t marker, uint64_t* out )
{
    uint32_t outpos     = 0;
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
    return (outpos<<3);
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
    const uint8_t* data   = pixels.data;

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

    // Each channel is saved separately
    for( size_t i = 0; i < depth; ++i )
        for( size_t j = i; j < nBytes; j += depth )
            image.read( reinterpret_cast<char*>( &pixels.data[j] ), 1 );

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
        setFormat( buffer, (depth==3) ? GL_RGB : GL_RGBA );
        setType( buffer, GL_UNSIGNED_BYTE );
    }
    else
    {
        EQASSERT( buffer == Frame::BUFFER_DEPTH );
        setFormat( buffer, GL_DEPTH_COMPONENT );
        setType( buffer, GL_FLOAT );
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
