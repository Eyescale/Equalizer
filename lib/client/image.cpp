
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

#include "image.h"

#include "frame.h"
#include "frameBufferObject.h"
#include "frameData.h"
#include "pixel.h"
#include "log.h"
#include "windowSystem.h"

#include <eq/base/omp.h>
#include <eq/net/node.h>
#include <eq/client/compressor.h>
#include <eq/client/pluginRegistry.h>

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
        , _type( Frame::TYPE_MEMORY )
{
    reset();
}

Image::~Image()
{
}

void Image::reset()
{
    _usePBO = false;
    setPixelViewport( PixelViewport( ));
}

void Image::flush()
{
    _color.pixels.flush();
    _depth.pixels.flush();
    _color.compressedPixels.flush();
    _depth.compressedPixels.flush();
    _color.texture.flush();
    _depth.texture.flush();
}

uint32_t Image::getDepth( const Frame::Buffer buffer ) const
{
    return getNumChannels( buffer ) * getChannelSize( buffer );
}

uint8_t Image::getNumChannels( const Frame::Buffer buffer ) const
{
    switch( getFormat( buffer ) )
    {
        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA:
            return 4;

        case GL_RGB:
        case GL_BGR:
            return 3;

        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL_NV:
            return 1;

        default :
            EQWARN << "Unknown number of components for format "
                   << getFormat( buffer ) << " of buffer " << buffer << endl;
            EQUNIMPLEMENTED;
    } 
    return 0;
}

uint8_t Image::getChannelSize( const Frame::Buffer buffer ) const
{
    switch( getType( buffer ))
    {
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return 1;

        case GL_HALF_FLOAT: 
            return 2;

        case GL_FLOAT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_24_8_NV:
            return 4;

        default :
            EQUNIMPLEMENTED;
    }
    return 0;
}

uint32_t Image::getInternalTextureFormat( const Frame::Buffer which ) const
{
    switch( getFormat( which ))
    {
        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA:
            return GL_RGBA;

        case GL_RGB:
        case GL_BGR:
            return GL_RGB;

        case GL_DEPTH_STENCIL_NV:
        case GL_DEPTH_COMPONENT:
            return GL_DEPTH_COMPONENT;

        default :
            EQWARN << "Unknown format " << getFormat( which ) << " of buffer "
                   << which << endl;
            EQUNIMPLEMENTED;
            return GL_RGBA;
    }
}

Image::Pixels& Image::_getPixels( const Frame::Buffer buffer )
{
    return _getAttachment( buffer ).pixels;
}

Image::CompressedPixels& Image::_getCompressedPixels(
    const Frame::Buffer buffer )
{
    return _getAttachment( buffer ).compressedPixels;
}

const Image::Pixels& Image::_getPixels( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _color.pixels;

        case Frame::BUFFER_DEPTH:
        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _depth.pixels;
    }
}

const Image::CompressedPixels& Image::_getCompressedPixels(
    const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _color.compressedPixels;

        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _depth.compressedPixels;
    }
}

void Image::setFormat( const Frame::Buffer buffer, const uint32_t format )
{
    Pixels& pixels = _getPixels( buffer );
    if( pixels.data.format == format )
        return;

    pixels.data.format = format;
    pixels.state = Pixels::INVALID;

    _getTexture( buffer ).setFormat( format );
}

void Image::setType( const Frame::Buffer buffer, const uint32_t type )
{
    Pixels& pixels = _getPixels( buffer );
    if( pixels.data.type == type )
        return;

    pixels.data.type = type;
    pixels.state = Pixels::INVALID;
}

uint32_t Image::getFormat( const Frame::Buffer buffer ) const
{
    const Pixels& pixels = _getPixels( buffer );
    EQASSERT( pixels.data.format );
    return pixels.data.format;
}

uint32_t Image::getType( const Frame::Buffer buffer ) const
{
    const Pixels& pixels = _getPixels( buffer );
    EQASSERT( pixels.data.type );
    return pixels.data.type;
}
 
uint32_t Image::_getCompressorName(const Frame::Buffer buffer)
{
    const uint32_t numChannels = getNumChannels( buffer ); 
    const uint32_t type = getType( buffer );

    if ( numChannels == 4 )
    {
        switch( type )
        {
        case GL_FLOAT:
            return EQ_COMPRESSOR_RLE_4_FLOAT;
        case GL_HALF_FLOAT:
            return EQ_COMPRESSOR_RLE_4_HALF_FLOAT;
        case GL_UNSIGNED_BYTE:
            return EQ_COMPRESSOR_DIFF_RLE_4_BYTE;
        default:
            break;
        }
    }
    
    if ( numChannels == 3 && type == GL_UNSIGNED_BYTE )
    {
        return EQ_COMPRESSOR_RLE_3_BYTE;
    }
    
    if ( numChannels == 1 && type == GL_UNSIGNED_INT)
    {
        return EQ_COMPRESSOR_DIFF_RLE_4_BYTE;
    }
    
    return EQ_COMPRESSOR_RLE_BYTE;
}

bool Image::hasAlpha() const
{
    switch( getFormat( Frame::BUFFER_COLOR ))
    {
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA:
            return true;

        default:
            return false;
    }
}

bool Image::hasData( const Frame::Buffer buffer ) const
{
    if( _type == Frame::TYPE_MEMORY )
        return hasPixelData( buffer );

    EQASSERT( _type == Frame::TYPE_TEXTURE );
    return hasTextureData( buffer );
}

bool Image::hasTextureData( const Frame::Buffer buffer ) const
{
    return getTexture( buffer ).isValid(); 
}

const uint8_t* Image::getPixelPointer( const Frame::Buffer buffer ) const
{
    EQASSERT( hasPixelData( buffer ));
    return _getPixels( buffer ).data.chunk.data;
}

uint8_t* Image::getPixelPointer( const Frame::Buffer buffer )
{
    EQASSERT( hasPixelData( buffer ));
    return _getPixels( buffer ).data.chunk.data;
}

const Image::PixelData& Image::getPixelData( const Frame::Buffer buffer ) const
{
    EQASSERT(hasPixelData(buffer));
    return _getPixels( buffer ).data;
}

void Image::startReadback( const uint32_t buffers, const PixelViewport& pvp,
                           const Zoom& zoom, Window::ObjectManager* glObjects )
{
    EQASSERT( glObjects );
    EQASSERTINFO( !_glObjects, "Another readback in progress?" );
    EQLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers
                          << endl;

    _glObjects = glObjects;
    _pvp       = pvp;

    _color.pixels.state = Pixels::INVALID;
    _depth.pixels.state = Pixels::INVALID;

    if( buffers & Frame::BUFFER_COLOR )
        _startReadback( Frame::BUFFER_COLOR, zoom );

    if( buffers & Frame::BUFFER_DEPTH )
        _startReadback( Frame::BUFFER_DEPTH, zoom );


    _pvp.apply( zoom );
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
    // round to next 8-byte alignment (compress might use 8-byte tokens)
    if( size%8 )
        size += 8 - (size%8);

    data.chunk.reserve( size );
}

const void* Image::_getBufferKey( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return ( reinterpret_cast< const char* >( this ) + 0 );
        case Frame::BUFFER_DEPTH:
            return ( reinterpret_cast< const char* >( this ) + 1 );
        default:
            EQUNIMPLEMENTED;
            return ( reinterpret_cast< const char* >( this ) + 2 );
    }
}

void Image::_startReadback( const Frame::Buffer buffer, const Zoom& zoom )
{
    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    compressedPixels.valid = false;

    if ( _type == Frame::TYPE_TEXTURE )
    {
        EQASSERTINFO( zoom == Zoom::NONE, "Texture readback zoom not "
                      << "implemented, zoom happens during compositing" );
        Texture& texture = _getTexture( buffer );
        texture.copyFromFrameBuffer( _pvp );
        return;
    }

    if( _usePBO && _glObjects->supportsBuffers( )) // use async PBO readback
    {
        EQASSERTINFO( zoom == Zoom::NONE, "Not Implemented" );
        _startReadbackPBO( buffer );
        return;
    }

    if( zoom == Zoom::NONE ) // normal glReadPixels
    {
        Pixels&    pixels = _getPixels( buffer );
        const size_t size = getPixelDataSize( buffer );

        pixels.resize( size );
        glReadPixels( _pvp.x, _pvp.y, _pvp.w, _pvp.h, getFormat( buffer ),
                      getType( buffer ), pixels.data.chunk.data );
        pixels.data.chunk.size = size;
        pixels.state = Pixels::VALID;
        return;
    }

    // else copy to texture, draw zoomed quad into FBO, (read FBO texture)
    _startReadbackZoom( buffer, zoom );
}

const Texture& Image::getTexture( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _color.texture;
            
        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _depth.texture;
    }
}   

Texture& Image::_getTexture( const Frame::Buffer buffer )
{
    return _getAttachment( buffer ).texture;
}   

void Image::_startReadbackPBO( const Frame::Buffer buffer )
{
    Pixels& pixels = _getPixels( buffer );
    pixels.state = Pixels::PBO_READBACK;

    const void* bufferKey = _getBufferKey( buffer );
    GLuint pbo = _glObjects->obtainBuffer( bufferKey );
    
    EQ_GL_CALL( glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo ));

    const size_t size = getPixelDataSize( buffer );
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

void Image::_startReadbackZoom( const Frame::Buffer buffer, const Zoom& zoom )
{
    EQASSERT( _glObjects );
    EQASSERT( _glObjects->supportsEqTexture( ));
    EQASSERT( _glObjects->supportsEqFrameBufferObject( ));

    PixelViewport pvp = _pvp;
    pvp.apply( zoom );
    if( !pvp.hasArea( ))
        return;

    Pixels& pixels = _getPixels( buffer );
    pixels.state = Pixels::ZOOM_READBACK;
    
    // copy frame buffer to texture
    const void* bufferKey = _getBufferKey( buffer );
    Texture*    texture   = _glObjects->obtainEqTexture( bufferKey );

    texture->setFormat( getInternalTextureFormat( buffer ));
    texture->copyFromFrameBuffer( _pvp );

    // draw zoomed quad into FBO
    //  uses the same FBO for color and depth, with masking.
    const void*     fboKey = _getBufferKey( Frame::BUFFER_COLOR );
    FrameBufferObject* fbo = _glObjects->getEqFrameBufferObject( fboKey );

    if( fbo )
    {
        EQCHECK( fbo->resize( pvp.w, pvp.h ));
    }
    else
    {
        fbo = _glObjects->newEqFrameBufferObject( fboKey );
        fbo->setColorFormat( getInternalTextureFormat( buffer ) );
        fbo->init( pvp.w, pvp.h, 24, 0 );
    }
    fbo->bind();
    texture->bind();

    if ( buffer == Frame::BUFFER_COLOR )
        glDepthMask( false );
    else
    {
        EQASSERT( buffer == Frame::BUFFER_DEPTH )
        glColorMask( false, false, false, false );
    }

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glColor3f( 1.0f, 1.0f, 1.0f );

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( 0, 0, 0.0f );

        glTexCoord2f( static_cast< float >( _pvp.w ), 0.0f );
        glVertex3f( static_cast< float >( pvp.w ), 0, 0.0f );

        glTexCoord2f( static_cast< float >( _pvp.w ),
                      static_cast< float >( _pvp.h ));
        glVertex3f( static_cast< float >( pvp.w ),
                    static_cast< float >( pvp.h ), 0.0f );

        glTexCoord2f( 0.0f, static_cast< float >( _pvp.h ));
        glVertex3f( 0, static_cast< float >( pvp.h ), 0.0f );
    glEnd();

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );

    if ( buffer == Frame::BUFFER_COLOR )
        glDepthMask( true );
    else
    {
        const ColorMask colorMask; // TODO = channel->getDrawBufferMask();
        glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
    }
    // TODO channel->bindFramebuffer()
    fbo->unbind();

    EQLOG( LOG_ASSEMBLY ) << "Scale " << _pvp << " -> " << pvp << std::endl;
}

void Image::_syncReadback( const Frame::Buffer buffer )
{
    Pixels& pixels = _getPixels( buffer );
    switch( pixels.state )
    {
        case Pixels::PBO_READBACK:
            _syncReadbackPBO( buffer );
            break;

        case Pixels::ZOOM_READBACK:
            _syncReadbackZoom( buffer );
            break;

        default:
            break;
    }
}

void Image::_syncReadbackPBO( const Frame::Buffer buffer )
{
    // async readback only possible when PBOs are used and supported
    EQASSERT( _usePBO );
    EQASSERT( _glObjects->supportsBuffers( ));
    EQ_GL_ERROR( "before Image::_syncReadback" );

    const size_t size      = getPixelDataSize( buffer );
    const void*  bufferKey = _getBufferKey( buffer );
    GLuint       pbo       = _glObjects->getBuffer( bufferKey );
    EQASSERT( pbo != Window::ObjectManager::INVALID );

    Pixels& pixels = _getPixels( buffer );
    pixels.resize( size );
    pixels.data.chunk.size = size;
    EQ_GL_CALL( glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo ));
    const void* data = glMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY );
    EQ_GL_ERROR( "glMapBuffer" );
    EQASSERT( data );

    memcpy( pixels.data.chunk.data, data, size );

    glUnmapBuffer( GL_PIXEL_PACK_BUFFER );
    glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

    pixels.state = Pixels::VALID;
}

void Image::_syncReadbackZoom( const Frame::Buffer buffer )
{
    Pixels& pixels = _getPixels( buffer );
    const size_t size = getPixelDataSize( buffer );
    pixels.resize( size );
    pixels.data.chunk.size = size;
    const void*  bufferKey = _getBufferKey( buffer );
    FrameBufferObject* fbo = _glObjects->getEqFrameBufferObject( bufferKey );
    EQASSERT( fbo != 0 );
    
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            fbo->getColorTextures()[0]->download( pixels.data.chunk.data,
                                                  getFormat( buffer ), 
                                                  getType( buffer ));
            break;

        default:
            EQUNIMPLEMENTED;
        case Frame::BUFFER_DEPTH:
            fbo->getDepthTexture().download( pixels.data.chunk.data,
                                             getFormat( buffer ), 
                                             getType( buffer ));
            break;
    }

    pixels.state = Pixels::VALID;
    EQLOG( LOG_ASSEMBLY ) << "Read texture " << _pvp << std::endl;
}

void Image::setPixelViewport( const PixelViewport& pvp )
{
    _pvp = pvp;
    _color.pixels.state = Pixels::INVALID;
    _depth.pixels.state = Pixels::INVALID;
    _color.compressedPixels.valid = false;
    _depth.compressedPixels.valid = false;
}

void Image::clearPixelData( const Frame::Buffer buffer )
{
    const ssize_t size  = getPixelDataSize( buffer );
    if( size == 0 )
        return;

    validatePixelData( buffer );
    Pixels& pixels = _getPixels( buffer );

    if( buffer == Frame::BUFFER_DEPTH )
    {
        memset( pixels.data.chunk.data, 0xFF, size );
    }
    else
    {
        if( getDepth( Frame::BUFFER_COLOR ) == 4 )
        {
            uint8_t* data = pixels.data.chunk.data;
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
            bzero( pixels.data.chunk.data, size );
    }
}

void Image::validatePixelData( const Frame::Buffer buffer )
{
    Pixels& pixels = _getPixels( buffer );
    const size_t size = getPixelDataSize( buffer );

    pixels.resize( size );
    pixels.state = Pixels::VALID;

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
    pixels.data.chunk.size = size;
    memcpy( pixels.data.chunk.data, data, size );
    pixels.state = Pixels::VALID;

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

    if( pixels.compressorName == EQ_COMPRESSOR_NONE )
    {

        EQASSERT( size == pixels.chunk.size );

        setPixelData( buffer, pixels.chunk.data );
        return;
    }

    _allocCompressor( buffer, 
                      pixels.compressorName );

    const uint32_t depth = getDepth( buffer );

    Pixels& outPixels = _getPixels( buffer );
    EQASSERT( size > 0 );
    outPixels.resize( size );

    Attachment& attachment = _getAttachment( buffer );
    attachment.compressedPixels.valid = false;

    // Get number of blocks in compressed data
    const uint64_t nChunks  = pixels.lengthData.size;
    EQASSERT(( depth % nChunks ) == 0 );

    void* outData = 
        reinterpret_cast< uint8_t* >( outPixels.data.chunk.data );

    uint64_t outDim[4] = { 0, _pvp.w, 0, _pvp.h}; 

    EQASSERT( attachment.plugin != 0 );
    
    attachment.plugin->decompress( attachment.compressor,
                                   attachment.compressorName, 
                                   ( const void ** )pixels.outCompressed.data,
                                   pixels.lengthData.data,
                                   nChunks,
                                   outData, 
                                   outDim,
                                   EQ_COMPRESSOR_DATA_2D );
    


    outPixels.state = Pixels::VALID;
}

Image::Attachment& Image::_getAttachment( const Frame::Buffer buffer )
{
   switch( buffer )
   {
       case Frame::BUFFER_COLOR:
           return _color;
       case Frame::BUFFER_DEPTH:
           return _depth;
       default:
           EQUNIMPLEMENTED;
   }
   return _color;
}

/** Find and activate a compressor engine */
void Image::_allocCompressor( const Frame::Buffer buffer,
                              uint32_t compressorName )
{
    Attachment& attachment = _getAttachment( buffer );
    if ( !attachment.plugin || 
       ( attachment.compressorName != compressorName ))
    {
        attachment.compressorName = compressorName;
        if ( attachment.compressor )
            attachment.plugin->deleteCompressor( attachment.compressor );
                 
        attachment.plugin = 
               Global::getPluginRegistry()->findCompressor( compressorName );
        attachment.compressor = 
                    attachment.plugin->newCompressor( compressorName );
    }
}

void Image::CompressedPixels::flush()
{
    data.flush();
    valid = false;
}

void Image::Pixels::flush()
{
    pboSize = 0;
    state = INVALID;
    data.flush();
}

void Image::PixelData::flush()
{
    chunk.clear();
    format = GL_FALSE;
    type   = GL_FALSE;
    compressorName = EQ_COMPRESSOR_NONE;
}

Image::PixelData::~PixelData()
{
    flush();
}

const Image::PixelData& Image::compressPixelData( const Frame::Buffer buffer )
{
    const uint64_t size = getPixelDataSize( buffer );

    EQASSERT( size > 0 );

    CompressedPixels& compressedPixels = _getCompressedPixels( buffer );
    if( compressedPixels.valid )
        return compressedPixels.data;

    compressedPixels.data.compressorName  = _getCompressorName(buffer);
    _allocCompressor( buffer, compressedPixels.data.compressorName );

    compressedPixels.data.format  = getFormat( buffer );
    compressedPixels.data.type    = getType( buffer );

    uint64_t inDims[4]  = { 0, _pvp.w, 0, _pvp.h}; 

    Attachment& attachment = _getAttachment( buffer );
    
    EQASSERT( attachment.plugin != 0 );
    
    attachment.plugin->compress( attachment.compressor, 
                                 getPixelPointer( buffer ), 
                                 inDims, EQ_COMPRESSOR_DATA_2D );

    const size_t numResults = 
        attachment.plugin->getNumResults( attachment.compressor );
    
    compressedPixels.data.outCompressed.resize( numResults );
    compressedPixels.data.lengthData.resize( numResults );

    for( size_t i = 0; i < numResults ; i++ )
    {
        attachment.plugin->getResult( 
                    attachment.compressor,
                    i, 
                    &compressedPixels.data.outCompressed.data[i], 
                    &compressedPixels.data.lengthData.data[i] );
    }


    compressedPixels.valid = true;
    return compressedPixels.data;
}




//---------------------------------------------------------------------------
// IO
//---------------------------------------------------------------------------

void Image::writeImages( const std::string& filenameTemplate ) const
{
    writeImages( filenameTemplate + "_color", Frame::BUFFER_COLOR );
    writeImages( filenameTemplate + "_depth", Frame::BUFFER_DEPTH );
}


void Image::writeImages( const std::string& filenameTemplate,
                         const Frame::Buffer buffer ) const
{
    const Pixels& pixels = _getPixels( buffer );

    if( pixels.state == Pixels::VALID )
    {
        const uint32_t depth = getDepth( buffer );
        for( uint32_t d = 0; d < depth; d+=4 )
        {
            ostringstream stringstream;
            writeImage( filenameTemplate + stringstream.str() + ".rgb",
                        buffer );
        }
    }

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
        {
            memset( this, 0, sizeof( RGBHeader ));
            magic           = 474;
            bytesPerChannel = 1;
            nDimensions     = 3;
            maxValue        = 255;
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
    const size_t  nPixels = _pvp.w * _pvp.h;
    const Pixels& pixels  = _getPixels( buffer );

    if( nPixels == 0 || pixels.state != Pixels::VALID )
        return;

    ofstream image( filename.c_str(), ios::out | ios::binary );
    if( !image.is_open( ))
    {
        EQERROR << "Can't open " << filename << " for writing" << endl;
        return;
    }

    RGBHeader    header;

    header.width  = _pvp.w;
    header.height = _pvp.h;

    header.bytesPerChannel = getChannelSize( buffer );
    header.depth = getNumChannels( buffer );

    if( header.depth == 1 ) // depth
    {
        EQASSERT( (header.bytesPerChannel % 4) == 0 );
        header.depth = 4;
        header.bytesPerChannel /= 4;
    }
    EQASSERT( header.bytesPerChannel > 0 );
    if( header.bytesPerChannel > 2 )
        EQWARN << static_cast< int >( header.bytesPerChannel ) 
               << " bytes per channel not supported by RGB spec" << std::endl;

    const uint8_t bpc = header.bytesPerChannel;
    const uint16_t nChannels = header.depth;

    strncpy( header.filename, filename.c_str(), 80 );

    header.convert();
    image.write( reinterpret_cast<const char *>( &header ), sizeof( header ));

    // Each channel is saved separately
    const size_t depth  = nChannels * bpc;
    const size_t nBytes = nPixels * depth;
    const char* data = reinterpret_cast<const char*>( getPixelPointer( buffer));

    switch( getFormat( buffer ))
    {
        case GL_BGR:
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
            for( size_t j = 1 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
            for( size_t j = 0; j < nBytes; j += depth )
                image.write( &data[j], bpc );
            break;

        case GL_BGRA:
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
            for( size_t j = 1 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
            for( size_t j = 0; j < nBytes; j += depth )
                image.write( &data[j], bpc );
            // invert alpha
            for( size_t j = 3 * bpc; j < nBytes; j += depth )
            {
                if( bpc == 1 )
                {
                    const uint8_t val = 255 - 
                        *reinterpret_cast< const uint8_t* >( &data[j] );
                    image.write( reinterpret_cast<const char*>( &val ), 1 );
                }
                else
                    image.write(&data[j], bpc );
            }
            break;

        default:
            for( size_t i = 0; i < nChannels; ++i )
                for( size_t j = i * bpc; j < nBytes; j += depth )
                    image.write(&data[j], bpc );
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
    if( header.compression != 0)
    {
        EQERROR << "Unsupported compression " << filename << endl;
        image.close();
        return false;
    }

    const size_t nChannels = header.depth;

    if( header.nDimensions != 3 ||
        header.minValue != 0 ||
        header.maxValue != 255 ||
        header.colorMode != 0 ||
        ( buffer == Frame::BUFFER_COLOR && nChannels != 3 && nChannels != 4 ) ||
        ( buffer == Frame::BUFFER_DEPTH && nChannels != 4 ))
    {
        EQERROR << "Unsupported image type " << filename << endl;
        image.close();
        return false;
    }

    switch( buffer )
    {
        case Frame::BUFFER_DEPTH:
            if( header.bytesPerChannel != 1 )
            {
                EQERROR << "Unsupported channel depth " 
                        << static_cast< int >( header.bytesPerChannel ) << endl;
                image.close();
                return false;
            }

            setFormat( Frame::BUFFER_DEPTH, GL_DEPTH_COMPONENT );
            setType(   Frame::BUFFER_DEPTH, GL_UNSIGNED_INT );
            break;

        default:
            EQUNREACHABLE;
        case Frame::BUFFER_COLOR:
            setFormat( Frame::BUFFER_COLOR, (nChannels==4) ? GL_RGBA : GL_RGB );
            switch( header.bytesPerChannel )
            {
                case 1:
                    setType( Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
                    break;
                case 2:
                    setType( Frame::BUFFER_COLOR, GL_HALF_FLOAT );
                    break;
                case 4:
                    setType( Frame::BUFFER_COLOR, GL_FLOAT );
                    break;
                default:
                    EQERROR << "Unsupported channel depth " 
                            << static_cast< int >( header.bytesPerChannel )
                            << std::endl;
                    image.close();
                    return false;
            }
            break;
    }

    const uint8_t bpc     = header.bytesPerChannel;
    const size_t  depth   = nChannels * bpc;
    const size_t  nPixels = header.width * header.height;
    const size_t  nBytes  = nPixels * depth;
    Pixels        pixels;

    pixels.data.format = getFormat( buffer );
    pixels.data.type   = getType( buffer );
    pixels.resize( nBytes );
    char* data = reinterpret_cast< char* >( pixels.data.chunk.data );

    // Each channel is saved separately
    for( size_t i = 0; i < depth; ++i )
        for( size_t j = i * bpc; j < nBytes; j += depth )
            image.read( &data[j], bpc );

    if( image.bad() || image.eof( ))
    {
        EQERROR << "Error during image data input " << filename << endl;
        image.close();
        return false;
    }

    const PixelViewport pvp( 0, 0, header.width, header.height );
    if( pvp != getPixelViewport( ))
        setPixelViewport( pvp );

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
