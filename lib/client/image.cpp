
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "image.h"
#include "log.h"

#include <eq/client/windowSystem.h>

#include <fstream>

using namespace eq;
using namespace std;

Image::~Image()
{
    for( uint32_t i=0; i<INDEX_ALL; ++i )
        _pixels[i].clear();
}

size_t Image::getDepth( const Frame::Buffer buffer ) const
{
    // TODO: OpenGL format/type
    switch( buffer )
    {
        case Frame::BUFFER_COLOR: return 4; // assume GL_RGBA/GL_UNSIGNED_BYTE
        case Frame::BUFFER_DEPTH: return 4; //assume GL_DEPTH_COMPONENT/GL_FLOAT
        default:
            EQUNIMPLEMENTED;
            return 0;
    }
}

uint32_t Image::getFormat( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR: return GL_RGBA;
        case Frame::BUFFER_DEPTH: return GL_DEPTH_COMPONENT;
        default:
            EQUNIMPLEMENTED;
            return 0;
    }
}

uint32_t Image::getType( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR: return GL_UNSIGNED_BYTE;
        case Frame::BUFFER_DEPTH: return GL_FLOAT;
        default:
            EQUNIMPLEMENTED;
            return 0;
    }
}

void Image::startReadback(const PixelViewport& pvp, const Frame::Buffer buffers)
{
    EQLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers 
                          << endl;

    _pvp   = pvp;
    _pvp.x = 0;
    _pvp.y = 0;

    if( buffers & Frame::BUFFER_COLOR )
        _startReadback( Frame::BUFFER_COLOR );
    else
        _pixels[INDEX_COLOR].clear();

    if( buffers & Frame::BUFFER_DEPTH )
        _startReadback( Frame::BUFFER_DEPTH );
    else
        _pixels[INDEX_DEPTH].clear();
}

void Image::_startReadback( const Frame::Buffer buffer )
{
    const uint32_t index = _getIndexForBuffer( buffer );
    const size_t   size  = _pvp.w * _pvp.h * getDepth( buffer );

    _pixels[index].resize( size );

    glReadPixels( _pvp.x, _pvp.y, _pvp.w, _pvp.h, getFormat( buffer ), 
                  getType( buffer ), &_pixels[index][0] );
}

void Image::startAssemble( const vmml::Vector2i& offset, 
                           const Frame::Buffer buffers )
{
    uint32_t useBuffers = Frame::BUFFER_NONE;

    if( buffers & Frame::BUFFER_COLOR && !_pixels[INDEX_COLOR].empty( ))
        useBuffers |= Frame::BUFFER_COLOR;
    if( buffers & Frame::BUFFER_DEPTH && !_pixels[INDEX_DEPTH].empty( ))
        useBuffers |= Frame::BUFFER_DEPTH;

    if( useBuffers == Frame::BUFFER_NONE )
    {
        EQWARN << "No buffers to assemble" << endl;
        return;
    }

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
    EQASSERT( !_pixels[INDEX_COLOR].empty( ));

    glRasterPos2i( offset.x + _pvp.x, offset.y + _pvp.y );
    glDrawPixels( _pvp.w, _pvp.h, getFormat( Frame::BUFFER_COLOR ), 
                  getType( Frame::BUFFER_COLOR ), &_pixels[INDEX_COLOR][0] );
}

void Image::_startAssembleDB( const vmml::Vector2i& offset )
{
    EQASSERT( !_pixels[INDEX_COLOR].empty( ));
    EQASSERT( !_pixels[INDEX_DEPTH].empty( ));

    // Z-Based sort-last assembly
    EQUNIMPLEMENTED;
}

void Image::setPixelViewport( const PixelViewport& pvp )
{
    _pvp = pvp;

    const size_t nPixels = pvp.w * pvp.h;
    _pixels[INDEX_COLOR].resize( nPixels * getDepth( Frame::BUFFER_COLOR ));
    _pixels[INDEX_DEPTH].resize( nPixels * getDepth( Frame::BUFFER_DEPTH ));
}

void Image::setData( const Frame::Buffer buffer, const uint8_t* data )
{
    const uint32_t index = _getIndexForBuffer( buffer );
    const size_t   size  = _pvp.w * _pvp.h * getDepth( Frame::BUFFER_COLOR );

    memcpy( &_pixels[index][0], data, size );
}

void Image::writeImages( const std::string& filenameTemplate )
{
    if( !_pixels[INDEX_COLOR].empty( ))
        writeImage( filenameTemplate + "_color.rgb", Frame::BUFFER_COLOR );
    if( _pixels[INDEX_DEPTH].empty( ))
        writeImage( filenameTemplate + "_depth.rgb", Frame::BUFFER_DEPTH );
}

#define SWAP_SHORT(v) ( v = (v&0xff) << 8 | (v&0xff00) >> 8 )
#define SWAP_INT(v)   ( v = (v&0xff) << 24 | (v&0xff00) << 8 |      \
                        (v&0xff0000) >> 8 | (v&0xff000000) >> 24)

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
#       ifdef i386
            SWAP_SHORT(magic);
            SWAP_SHORT(nDimensions);
            SWAP_SHORT(width);
            SWAP_SHORT(height);
            SWAP_SHORT(depth);
            SWAP_INT(minValue);
            SWAP_INT(maxValue);
            SWAP_INT(colorMode);
#       endif
        }
    
    unsigned short magic       __attribute__ ((packed));
    char compression           __attribute__ ((packed));
    char bytesPerChannel       __attribute__ ((packed));
    unsigned short nDimensions __attribute__ ((packed));
    unsigned short width       __attribute__ ((packed));
    unsigned short height      __attribute__ ((packed));
    unsigned short depth       __attribute__ ((packed));
    long minValue              __attribute__ ((packed));
    long maxValue              __attribute__ ((packed));
    char unused[4]             __attribute__ ((packed));
    char filename[80]          __attribute__ ((packed));
    long colorMode             __attribute__ ((packed));
    char fill[404]             __attribute__ ((packed));
};

void Image::writeImage( const std::string& filename, const Frame::Buffer buffer)
{
    const size_t           nPixels = _pvp.w * _pvp.h;
    const size_t           depth   = getDepth( buffer );
    const size_t           nBytes  = nPixels * depth;
    const uint32_t         index   = _getIndexForBuffer( buffer );
    const vector<uint8_t>& pixels  = _pixels[index];

    if( nPixels == 0 || pixels.size() < nBytes )
        return;

    ofstream image( filename.c_str(), ios::out | ios::binary );
    if( !image.is_open( ))
    {
        EQERROR << "Can't open " << filename << " for writing" << endl;
        return;
    }

    RGBHeader header;
    
    header.width  = _pvp.w;
    header.height = _pvp.h;
    header.depth  = depth;
    strncpy( header.filename, filename.c_str(), 80 );

    header.convert();
    image.write( (const char *)&header, sizeof( header ));

    // Each channel is written separately
    for( size_t i = 0; i < depth; ++i )
        for( size_t j = i; j < nBytes; j += depth )
            image << pixels[j];

    image.close();
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
