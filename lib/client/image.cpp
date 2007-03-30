
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "image.h"

#include "frame.h"
#include "log.h"
#include "windowSystem.h"

#include <eq/net/node.h>
#include <fstream>

using namespace eq;
using namespace std;

Image::Image()
{
    _pixels[INDEX_COLOR].format = GL_BGRA;
    _pixels[INDEX_COLOR].type   = GL_UNSIGNED_BYTE;
    _pixels[INDEX_DEPTH].format = GL_DEPTH_COMPONENT;
    _pixels[INDEX_DEPTH].type   = GL_FLOAT;
}

Image::~Image()
{
}

uint32_t Image::getDepth( const Frame::Buffer buffer ) const
{
    uint32_t depth = 0;
    switch( getFormat( buffer ))
    {
        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA:
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
            EQUNIMPLEMENTED;
    }

    switch( getType( buffer ))
    {
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return depth; // depth *= 1;

        case GL_FLOAT:
            return depth * 4;

        default :
            EQUNIMPLEMENTED;
    }
    return 0;
}

void Image::setFormat( const Frame::Buffer buffer, const uint32_t format )
{
    const uint32_t index  = _getIndexForBuffer( buffer );
    _pixels[index].format = format;
    _pixels[index].valid  = false;
}

void Image::setType( const Frame::Buffer buffer, const uint32_t type )
{
    const uint32_t index = _getIndexForBuffer( buffer );
    _pixels[index].type  = type;
    _pixels[index].valid = false;
}

uint32_t Image::getFormat( const Frame::Buffer buffer ) const
{
    const uint32_t index = _getIndexForBuffer( buffer );
    return _pixels[index].format;
}

uint32_t Image::getType( const Frame::Buffer buffer ) const
{
    const uint32_t index = _getIndexForBuffer( buffer );
    return _pixels[index].type;
}

void Image::startReadback( const uint32_t buffers, const PixelViewport& pvp )
{
    EQLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers 
                          << endl;

    _pvp   = pvp;

    if( buffers & Frame::BUFFER_COLOR )
        _startReadback( Frame::BUFFER_COLOR );
    else
        _pixels[INDEX_COLOR].valid = false;

    if( buffers & Frame::BUFFER_DEPTH )
        _startReadback( Frame::BUFFER_DEPTH );
    else
        _pixels[INDEX_DEPTH].valid = false;

    _pvp.x = 0;
    _pvp.y = 0;
}

void Image::Pixels::resize( const uint32_t size )
{
    valid = true;

    if( maxSize >= size )
        return;

    delete [] data;
    data    = new uint8_t[size];
    maxSize = size;
}

void Image::_startReadback( const Frame::Buffer buffer )
{
    const uint32_t index = _getIndexForBuffer( buffer );
    const size_t   size  = _pvp.w * _pvp.h * getDepth( buffer );

    _pixels[index].resize( size );
    _compressedPixels[index].valid = false;

    glReadPixels( _pvp.x, _pvp.y, _pvp.w, _pvp.h, getFormat( buffer ), 
                  getType( buffer ), _pixels[index].data );
}

void Image::startAssemble( const uint32_t buffers, const vmml::Vector2i& offset)
{
    uint32_t useBuffers = Frame::BUFFER_NONE;

    if( buffers & Frame::BUFFER_COLOR && _pixels[INDEX_COLOR].valid )
        useBuffers |= Frame::BUFFER_COLOR;
    if( buffers & Frame::BUFFER_DEPTH && _pixels[INDEX_DEPTH].valid )
        useBuffers |= Frame::BUFFER_DEPTH;

    if( useBuffers == Frame::BUFFER_NONE )
    {
        EQWARN << "No buffers to assemble" << endl;
        return;
    }

    glRasterPos2i( offset.x + _pvp.x, offset.y + _pvp.y );

    if( useBuffers == Frame::BUFFER_COLOR )
        _startAssemble2D( offset );
    else if( useBuffers == ( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH ))
        _startAssembleDB( offset );
    else
        EQUNIMPLEMENTED;
}

void Image::_startAssemble2D( const vmml::Vector2i& offset )
{
    EQLOG( LOG_ASSEMBLY ) << "_startAssemble2D " << _pvp << endl;
    EQASSERT( _pixels[INDEX_COLOR].valid );

    glDrawPixels( _pvp.w, _pvp.h, getFormat( Frame::BUFFER_COLOR ), 
                  getType( Frame::BUFFER_COLOR ), _pixels[INDEX_COLOR].data );
}

void Image::_startAssembleDB( const vmml::Vector2i& offset )
{
    EQLOG( LOG_ASSEMBLY ) << "_startAssembleDB " << _pvp << endl;
    EQASSERT( _pixels[INDEX_COLOR].valid );
    EQASSERT( _pixels[INDEX_DEPTH].valid );

    // Z-Based sort-last assembly
    glEnable( GL_STENCIL_TEST );
    
    // test who is in front and mark in stencil buffer
    glEnable( GL_DEPTH_TEST );
    glStencilFunc( GL_ALWAYS, 1, 1 );
    glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );

    glDrawPixels( _pvp.w, _pvp.h, getFormat( Frame::BUFFER_DEPTH ), 
                  getType( Frame::BUFFER_DEPTH ), _pixels[INDEX_DEPTH].data );
    
    glDisable( GL_DEPTH_TEST );

    // draw 'our' front pixels using stencil mask
    glStencilFunc( GL_EQUAL, 1, 1 );
    glStencilOp( GL_KEEP, GL_ZERO, GL_ZERO );
    
    glDrawPixels( _pvp.w, _pvp.h, getFormat( Frame::BUFFER_COLOR ), 
                  getType( Frame::BUFFER_COLOR ), _pixels[INDEX_COLOR].data );

    glDisable( GL_STENCIL_TEST );
}

void Image::setPixelViewport( const PixelViewport& pvp )
{
    _pvp = pvp;

    const uint32_t nPixels = pvp.w * pvp.h;
    _pixels[INDEX_COLOR].resize( nPixels * getDepth( Frame::BUFFER_COLOR ));
    _pixels[INDEX_DEPTH].resize( nPixels * getDepth( Frame::BUFFER_DEPTH ));
    _compressedPixels[INDEX_COLOR].valid = false;
    _compressedPixels[INDEX_DEPTH].valid = false;
}

void Image::setPixelData( const Frame::Buffer buffer, const uint8_t* data )
{
    const uint32_t size  = getPixelDataSize( buffer );
    if( size == 0 )
        return;
    
    const uint32_t index = _getIndexForBuffer( buffer );

    _pixels[index].resize( size );
    memcpy( _pixels[index].data, data, size );
    _compressedPixels[index].valid = false;
}

uint32_t Image::decompressPixelData( const Frame::Buffer buffer,
                                     const uint8_t* data )
{
    const uint32_t size = getPixelDataSize( buffer );
    if( size == 0 )
        return 0;
    
    EQASSERT( getDepth( buffer ) == 4 )     // may change with RGB format
    EQASSERT( size < (100 * 1024 * 1024 )); // < 100MB

    const uint32_t index = _getIndexForBuffer( buffer );
    _pixels[index].resize( size );
    _compressedPixels[index].valid = false;

    const uint32_t* in  = reinterpret_cast<const uint32_t*>( data );
    uint32_t* out = reinterpret_cast<uint32_t*>( _pixels[index].data );

    EQASSERT( size > 0 );

    const uint32_t marker = in[0];    
    uint32_t       outpos = 0;
    const uint32_t endpos = size>>2;

    uint32_t i = 1;
    while( outpos < endpos )
    {
        const uint32_t token = in[i++];
        if( token == marker )
        {
            const uint32_t symbol = in[i++];
            const uint32_t nSame  = in[i++];
            for( uint32_t i = 0; i<nSame; ++i )
                out[outpos++] = symbol;
        }
        else // symbol
            out[outpos++] = token;
    }
    EQASSERT( outpos == endpos );

    return (i<<2);
}

const uint8_t* Image::compressPixelData( const Frame::Buffer buffer, 
                                         uint32_t& compressedSize )
{
    const uint32_t size = getPixelDataSize( buffer );
    if( size == 0 ) 
        return 0;

    const uint32_t  index = _getIndexForBuffer( buffer );
    if( _compressedPixels[index].valid )
    {
        compressedSize = _compressedPixels[index].size;
        return _compressedPixels[index].data;
    }

    EQASSERT( getDepth( buffer ) == 4 )     // may change with RGB format
    EQASSERT( size < (100 * 1024 * 1024 )); // < 100MB

    const uint32_t* data     = reinterpret_cast<const uint32_t*>
                                   ( _pixels[index].data );
    uint32_t        marker   = 0xffffffffu;
    const uint32_t  nWords   = size>>2;

#ifdef PERFECT_MARKER
    bool            markerOk = true;
    for( uint32_t i=0; i<nWords; ++i )
    {
        if( data[i] == marker )
        {
            markerOk = false;
            break;
        }
    }
    while( !markerOk )
    {
        --marker; // Could guess random marker?
        EQWARN << "Trying marker " << marker << endl;
        
        for( uint32_t i=0; i<nWords; ++i )
        {
            if( data[i] == marker )
            {
                markerOk = false;
                break;
            }
        }
    }
#endif

#ifdef PERFECT_MARKER
    // Can't get bigger than input since marker is not in input data
    _compressedPixels[index].resize( size + sizeof( uint32_t ));
#else
    _compressedPixels[index].resize( 2 * size + sizeof( uint32_t ));
#endif

    uint32_t* out = reinterpret_cast<uint32_t*>( _compressedPixels[index].data);

    out[ 0 ] = marker;

    uint32_t outpos     = 1;
    uint32_t lastSymbol = data[0];
    uint32_t nSame      = 1;
    for( uint32_t i=1; i<nWords; ++i )
    {
        const uint32_t symbol = data[i];

        if( symbol == lastSymbol )
            ++nSame;
        else
        {
#ifndef PERFECT_MARKER
            if( lastSymbol == marker )
            {
                out[ outpos++ ] = marker;
                out[ outpos++ ] = lastSymbol;
                out[ outpos++ ] = nSame;
            }
            else
#endif
            switch( nSame )
            {
                case 0:
                    EQASSERTINFO( 0, "Unreachable code" );
                    break;
                case 3:
                    out[ outpos++ ] = lastSymbol; // fall through
                case 2:
                    out[ outpos++ ] = lastSymbol; // fall through
                case 1:
                    out[ outpos++ ] = lastSymbol;
                    break;
                default:
                    out[ outpos++ ] = marker;
                    out[ outpos++ ] = lastSymbol;
                    out[ outpos++ ] = nSame;
                    break;
            }     
            lastSymbol = symbol;
            nSame      = 1;
        }
    }

    // write remaining ...C&P code...
#ifndef PERFECT_MARKER
    if( lastSymbol == marker )
    {
        out[ outpos++ ] = marker;
        out[ outpos++ ] = lastSymbol;
        out[ outpos++ ] = nSame;
    }
    else
#endif
    switch( nSame )
    {
        case 0:
            EQASSERTINFO( 0, "Unreachable code" );
            break;
        case 3:
            out[ outpos++ ] = lastSymbol; // fall through
        case 2:
            out[ outpos++ ] = lastSymbol; // fall through
        case 1:
            out[ outpos++ ] = lastSymbol;
            break;
        default:
            out[ outpos++ ] = marker;
            out[ outpos++ ] = lastSymbol;
            out[ outpos++ ] = nSame;
            break;
    }         
    _compressedPixels[index].size = outpos<<2;

    compressedSize = _compressedPixels[index].size;
    return _compressedPixels[index].data;
}

//---------------------------------------------------------------------------
// IO
//---------------------------------------------------------------------------
void Image::writeImages( const std::string& filenameTemplate ) const
{
    if( _pixels[INDEX_COLOR].valid )
        writeImage( filenameTemplate + "_color.rgb", Frame::BUFFER_COLOR );
    if( _pixels[INDEX_DEPTH].valid )
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
    const uint32_t index   = _getIndexForBuffer( buffer );

    if( nPixels == 0 || !_pixels[index].valid )
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
    const size_t   nBytes  = nPixels * depth;
    const uint8_t* pixels  = _pixels[index].data;

    for( size_t i = 0; i < depth; ++i )
        for( size_t j = i; j < nBytes; j += depth )
            image.write( reinterpret_cast<const char*>( &pixels[j] ), 1 );

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
    if( header.bytesPerChannel != 1 || header.nDimensions != 3 || 
        header.minValue != 0 || header.maxValue != 255 || header.colorMode != 0)
    {
        EQERROR << "Unsupported image type " << filename << endl;
        image.close();
        return false;
    }


    const size_t     nPixels = header.width * header.height;
    const size_t     depth   = header.depth;
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

    setPixelViewport( PixelViewport( 0, 0, header.width, header.height ));
    setPixelData( buffer, pixels.data );

    image.close();
    return true;
}

Image::BufferIndex Image::_getIndexForBuffer( const Frame::Buffer buffer )
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR: return INDEX_COLOR;
        case Frame::BUFFER_DEPTH: return INDEX_DEPTH;

        default: 
            EQERROR << "Unimplemented" << endl;
    }
    return static_cast<BufferIndex>(-1);
}


std::ostream& eq::operator << ( std::ostream& os, const Image* image )
{
    os << "image " << image->_pvp;
    return os;
}
