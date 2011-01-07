
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#include "frameData.h"
#include "log.h"
#include "windowSystem.h"

#include <eq/util/frameBufferObject.h>
#include <eq/util/objectManager.h>

#include <eq/fabric/colorMask.h>
#include <co/node.h>

#include <co/base/global.h>
#include <co/base/memoryMap.h>
#include <co/base/omp.h>
#include <co/base/pluginRegistry.h>

// Internal headers
#include "../collage/base/plugin.h"
#include "../collage/base/compressorInfo.h"
#include "../collage/base/cpuCompressor.h"
#include "util/gpuCompressor.h"

#include <fstream>

#ifdef _WIN32
#  include <malloc.h>
#  define bzero( ptr, size ) memset( ptr, 0, size );
#else
#  include <alloca.h>
#endif

namespace eq
{
#define glewGetContext glObjects->glewGetContext

Image::Image()
        : _type( Frame::TYPE_MEMORY )
{
    reset();
}

Image::~Image(){}

void Image::reset()
{
    _ignoreAlpha = false;
    setPixelViewport( PixelViewport( ));
}

void Image::flush()
{
    _color.flush();
    _depth.flush();
}

Image::Attachment::Attachment()
        : fullCompressor( new co::base::CPUCompressor )
        , lossyCompressor( new co::base::CPUCompressor )
        , fullTransfer( new util::GPUCompressor )
        , lossyTransfer( new util::GPUCompressor )
        , compressor( fullCompressor )
        , transfer ( fullTransfer )
        , quality( 1.f )
        , texture( GL_TEXTURE_RECTANGLE_ARB )
{}

Image::Attachment::~Attachment()
{
    delete fullCompressor;
    delete lossyCompressor;
    delete fullTransfer;
    delete lossyTransfer;
}

void Image::Attachment::flush()
{
    memory.flush();
    texture.flush();
    fullCompressor->reset();
    lossyCompressor->reset();
    fullTransfer->reset();
    lossyTransfer->reset();
}

uint32_t Image::getPixelDataSize( const Frame::Buffer buffer ) const
{
    const Memory& memory = _getMemory( buffer );
    return memory.pvp.getArea() * memory.pixelSize;
}


void Image::_setExternalFormat( const Frame::Buffer buffer,
                                const uint32_t externalFormat,
                                const uint32_t pixelSize, const bool hasAlpha_ )
{
    Memory& memory = _getMemory( buffer );
    if( memory.externalFormat == externalFormat )
        return;

    memory.externalFormat = externalFormat;
    memory.pixelSize = pixelSize;
    memory.hasAlpha = buffer == Frame::BUFFER_DEPTH ? false : hasAlpha_;
    memory.state = Memory::INVALID;
}

void Image::setInternalFormat( const Frame::Buffer buffer,
                               const uint32_t internalFormat ) 
{
    Memory& memory = _getMemory( buffer );
    if( memory.internalFormat == internalFormat )
        return;

    memory.internalFormat = internalFormat;
    allocCompressor( buffer, EQ_COMPRESSOR_INVALID );
    if( internalFormat == 0 )
        return;
}

uint32_t Image::getInternalFormat( const Frame::Buffer buffer ) const
{
    const Memory& memory = _getAttachment( buffer ).memory;
    EQASSERT( memory.internalFormat );
    return memory.internalFormat;
}

std::vector< uint32_t > Image::findCompressors( const Frame::Buffer buffer )
    const
{
    const co::base::PluginRegistry& registry = co::base::Global::getPluginRegistry();
    const co::base::Plugins& plugins = registry.getPlugins();
    const uint32_t tokenType = getExternalFormat( buffer );

    std::vector< uint32_t > names;
    for( co::base::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const co::base::Plugin* plugin = *i;
        const co::base::CompressorInfos& infos = plugin->getInfos();

        for( co::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const co::base::CompressorInfo& info = *j;

            if( info.capabilities & EQ_COMPRESSOR_TRANSFER )
                continue;

            if( info.tokenType == tokenType )
                names.push_back( info.name );
        }
    }

    EQLOG( LOG_PLUGIN )
        << "Found " << names.size() << " compressors for token type 0x"
        << std::hex << tokenType << std::dec << std::endl;
    return names;
}

void Image::findTransferers( const Frame::Buffer buffer,
                             const GLEWContext* glewContext,
                             std::vector< uint32_t >& names )
{ 
    co::base::CompressorInfos infos;
    _findTransferers( buffer, glewContext, infos );
    for( co::base::CompressorInfos::const_iterator i = infos.begin();
         i != infos.end(); ++i )
    {
        names.push_back( i->name );
    }
}

void Image::_findTransferers( const Frame::Buffer buffer,
                             const GLEWContext* glewContext,
                             co::base::CompressorInfos& result )
{ 
    util::GPUCompressor::findTransferers(
        getInternalFormat( buffer ), getExternalFormat( buffer ),
        0 /*caps*/, getQuality( buffer ), _ignoreAlpha, glewContext, result );
}

uint32_t Image::_chooseCompressor( const Frame::Buffer buffer ) const
{
    const uint32_t tokenType = getExternalFormat( buffer );
    EQASSERT( tokenType != EQ_COMPRESSOR_DATATYPE_NONE );
    if( tokenType == EQ_COMPRESSOR_DATATYPE_NONE )
        return EQ_COMPRESSOR_NONE;

    const Attachment& attachment = _getAttachment( buffer );
    const float quality = attachment.quality /
                          attachment.lossyTransfer->getQuality();

    return co::base::CPUCompressor::chooseCompressor( tokenType, quality,
                                                  _ignoreAlpha );
}

bool Image::hasAlpha() const
{
    return hasPixelData( Frame::BUFFER_COLOR ) &&
           _getMemory( Frame::BUFFER_COLOR ).hasAlpha;
}

void Image::setAlphaUsage( const bool enabled )
{
    if( _ignoreAlpha != enabled )
        return;

    _ignoreAlpha = !enabled;
    _color.memory.isCompressed = false;
    _depth.memory.isCompressed = false;
}    

void Image::setQuality( const Frame::Buffer buffer, const float quality )
{
    Attachment& attachment = _getAttachment( buffer );
    if( attachment.quality == quality )
        return;
    
    attachment.quality = quality;
    if( quality == 1.f )
    {
        attachment.compressor = attachment.fullCompressor;
        attachment.transfer = attachment.fullTransfer;
    }
    else
    {
        attachment.lossyCompressor->reset();
        attachment.lossyTransfer->reset();
        attachment.compressor = attachment.lossyCompressor;
        attachment.transfer = attachment.lossyTransfer;
    }
}

float Image::getQuality( const Frame::Buffer buffer ) const
{
    return _getAttachment( buffer ).quality;
}

bool Image::hasTextureData( const Frame::Buffer buffer ) const
{
    return getTexture( buffer ).isValid(); 
}

const util::Texture& Image::getTexture( const Frame::Buffer buffer ) const
{
    return _getAttachment( buffer ).texture;
}

const uint8_t* Image::getPixelPointer( const Frame::Buffer buffer ) const
{
    EQASSERT( hasPixelData( buffer ));
    return reinterpret_cast< const uint8_t* >
          ( _getAttachment( buffer ).memory.pixels );
}

uint8_t* Image::getPixelPointer( const Frame::Buffer buffer )
{
    EQASSERT( hasPixelData( buffer ));
    return  reinterpret_cast< uint8_t* >
          ( _getAttachment( buffer ).memory.pixels );
}

const PixelData& Image::getPixelData( const Frame::Buffer buffer ) const
{
    EQASSERT( hasPixelData( buffer ));
    return _getAttachment( buffer ).memory;
}

void Image::upload( const Frame::Buffer buffer, util::Texture* texture,
                    const Vector2i& position,
                    util::ObjectManager< const void* >* glObjects ) const
{
    EQASSERT( glObjects );

    util::GPUCompressor* uploader = glObjects->obtainEqUploader(
                                        _getCompressorKey( buffer ));
    const PixelData& pixelData = getPixelData( buffer );
    const uint32_t externalFormat = pixelData.externalFormat;
    const uint32_t internalFormat = pixelData.internalFormat;
    const uint64_t flags = EQ_COMPRESSOR_TRANSFER |
                           EQ_COMPRESSOR_DATA_2D |
                           ( texture ? texture->getCompressorTarget() :
                                       EQ_COMPRESSOR_USE_FRAMEBUFFER );

    if( !uploader->isValidUploader( externalFormat, internalFormat, flags ))
        uploader->initUploader( externalFormat, internalFormat, flags );

    PixelViewport pvp = getPixelViewport();
    pvp.x = position.x() + pvp.x; 
    pvp.y = position.y() + pvp.y;
    if( texture )
        texture->init( internalFormat, _pvp.w, _pvp.h );

    uploader->upload( pixelData.pixels, pixelData.pvp, flags, pvp,
                      texture ? texture->getName() : 0 );
}

void Image::readback( const uint32_t buffers, const PixelViewport& pvp,
                      const Zoom& zoom,
                      util::ObjectManager< const void* >* glObjects )
{
    EQASSERT( glObjects );
    EQLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers
                          << std::endl;

    _pvp = pvp;
    _color.memory.state = Memory::INVALID;
    _depth.memory.state = Memory::INVALID;

    if( buffers & Frame::BUFFER_COLOR )
        _readback( Frame::BUFFER_COLOR, zoom, glObjects );

    if( buffers & Frame::BUFFER_DEPTH )
        _readback( Frame::BUFFER_DEPTH, zoom, glObjects );

    _pvp.x = 0;
    _pvp.y = 0;
}

void Image::readback( const Frame::Buffer buffer, const util::Texture* texture,
                      const GLEWContext* glewContext )
{
    Attachment& attachment = _getAttachment( buffer );
    util::GPUCompressor* downloader = attachment.transfer;
    Memory& memory = attachment.memory;    
    const uint32_t inputToken = memory.internalFormat;

    downloader->setGLEWContext( glewContext );

    uint32_t flags = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                     ( texture ? texture->getCompressorTarget() :
                                 EQ_COMPRESSOR_USE_FRAMEBUFFER );

    const bool alpha = _ignoreAlpha && buffer == Frame::BUFFER_COLOR;
    if( !downloader->isValidDownloader( inputToken, alpha, flags ))
        downloader->initDownloader( inputToken, attachment.quality, 
                                    alpha, flags );
            
    // get the pixel type produced by the downloader
    _setExternalFormat( buffer, downloader->getExternalFormat(),
                        downloader->getTokenSize(), downloader->hasAlpha( ));

    if( !memory.hasAlpha )
        flags |= EQ_COMPRESSOR_IGNORE_ALPHA;

    if( texture )
        downloader->download( PixelViewport( 0, 0, texture->getWidth(),
                                             texture->getHeight( )),
                              texture->getName(), flags,
                              memory.pvp, &memory.pixels );
    else
        downloader->download( _pvp, 0 , flags, memory.pvp, &memory.pixels );

    downloader->setGLEWContext( 0 );
    memory.state = Memory::VALID;
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

const void* Image::_getCompressorKey( const Frame::Buffer buffer ) const
{
    const Attachment& attachment = _getAttachment( buffer );
    
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            if( attachment.quality == 1.0f )
                return ( reinterpret_cast< const char* >( this ) + 0 );
            else
                return ( reinterpret_cast< const char* >( this ) + 1 );
        case Frame::BUFFER_DEPTH:
            if( attachment.quality == 1.0f )
                return ( reinterpret_cast< const char* >( this ) + 2 );
            else
                return ( reinterpret_cast< const char* >( this ) + 3 );
        default:
            EQUNIMPLEMENTED;
            return ( reinterpret_cast< const char* >( this ) + 0 );
    }
}

void Image::_readback( const Frame::Buffer buffer, const Zoom& zoom,
                       util::ObjectManager< const void* >* glObjects )
{
    Attachment& attachment = _getAttachment( buffer );
    attachment.memory.isCompressed = false;

    if( _type == Frame::TYPE_TEXTURE )
    {
        EQASSERTINFO( zoom == Zoom::NONE, "Texture readback zoom not "
                      << "implemented, zoom happens during compositing" );
        util::Texture& texture = _getAttachment( buffer ).texture;
        texture.setGLEWContext( glewGetContext( ));
        texture.copyFromFrameBuffer( getInternalFormat( buffer ), _pvp );
        texture.setGLEWContext( 0 );
    }
    else if( zoom == Zoom::NONE ) // normal framebuffer readback
        readback( buffer, 0, glewGetContext( ));
    else // copy to texture, draw zoomed quad into FBO, (read FBO texture)
        _readbackZoom( buffer, zoom, glObjects );
}

void Image::_readbackZoom( const Frame::Buffer buffer, const Zoom& zoom,
                           util::ObjectManager< const void* >* glObjects )
{
    EQASSERT( glObjects );
    EQASSERT( glObjects->supportsEqTexture( ));
    EQASSERT( glObjects->supportsEqFrameBufferObject( ));

    PixelViewport pvp = _pvp;
    pvp.apply( zoom );
    if( !pvp.hasArea( ))
        return;

    // copy frame buffer to texture
    const void* bufferKey = _getBufferKey( buffer );
    util::Texture* texture =
        glObjects->obtainEqTexture( bufferKey, GL_TEXTURE_RECTANGLE_ARB );

    texture->copyFromFrameBuffer( getInternalFormat( buffer ), _pvp );

    // draw zoomed quad into FBO
    //  uses the same FBO for color and depth, with masking.
    const void* fboKey = _getBufferKey( Frame::BUFFER_COLOR );
    util::FrameBufferObject* fbo = glObjects->getEqFrameBufferObject( fboKey );

    if( fbo )
    {
        EQCHECK( fbo->resize( pvp.w, pvp.h ));
    }
    else
    {
        fbo = glObjects->newEqFrameBufferObject( fboKey );
        EQCHECK( fbo->init( pvp.w, pvp.h, getInternalFormat( buffer ), 24, 0 ));
    }
    fbo->bind();
    texture->bind();

    if( buffer == Frame::BUFFER_COLOR )
        glDepthMask( false );
    else
    {
        EQASSERT( buffer == Frame::BUFFER_DEPTH )
        glColorMask( false, false, false, false );
    }

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    texture->applyZoomFilter( FILTER_LINEAR );
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
    // TODO channel->bindFramebuffer()
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, 0 );
    fbo->unbind();

    const util::Texture* zoomedTexture = 0;
    if( buffer == Frame::BUFFER_COLOR )
    {
        glDepthMask( true );
        zoomedTexture = fbo->getColorTextures().front();
    }
    else
    {
        const ColorMask colorMask; // TODO = channel->getDrawBufferMask();
        glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
        zoomedTexture = &fbo->getDepthTexture();
    }

    EQLOG( LOG_ASSEMBLY ) << "Scale " << _pvp << " -> " << pvp << std::endl;
    _pvp = pvp;

    readback( buffer, zoomedTexture, glewGetContext( ));
    EQLOG( LOG_ASSEMBLY ) << "Read texture " << getPixelDataSize( buffer )
                          << std::endl;
}

void Image::setPixelViewport( const PixelViewport& pvp )
{
    _pvp = pvp;
    _color.memory.state = Memory::INVALID;
    _depth.memory.state = Memory::INVALID;
    _color.memory.isCompressed = false;
    _depth.memory.isCompressed = false;
}

void Image::clearPixelData( const Frame::Buffer buffer )
{
    Memory& memory = _getAttachment( buffer ).memory;
    memory.pvp = _pvp;
    const ssize_t size = getPixelDataSize( buffer );
    if( size == 0 )
        return;

    validatePixelData( buffer );

    switch( memory.externalFormat )
    {
      case EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT:
        memset( memory.pixels, 0xFF, size );
        break;

      case EQ_COMPRESSOR_DATATYPE_RGBA:
      case EQ_COMPRESSOR_DATATYPE_BGRA:
      {
        uint8_t* data = reinterpret_cast< uint8_t* >( memory.pixels );
#ifdef Darwin
        const unsigned char pixel[4] = { 0, 0, 0, 255 };
        memset_pattern4( data, &pixel, size );
#else
        bzero( data, size );

#ifdef EQ_USE_OPENMP
#pragma omp parallel for
#endif
        for( ssize_t i = 3; i < size; i+=4 )
            data[i] = 255;
#endif
        break;
      }
      default:
        EQWARN << "Unknown external format " << memory.externalFormat
               << ", initializing to 0" << std::endl;
        bzero( memory.pixels, size );
        break;
    }
}

void Image::validatePixelData( const Frame::Buffer buffer )
{
    Memory& memory = _getAttachment( buffer ).memory;
    memory.useLocalBuffer();
    memory.state = Memory::VALID;
    memory.isCompressed = false;
}

void Image::setPixelData( const Frame::Buffer buffer, const PixelData& pixels )
{
    Memory& memory   = _getMemory( buffer );
    memory.externalFormat = pixels.externalFormat;
    memory.internalFormat = pixels.internalFormat;
    memory.pixelSize = pixels.pixelSize;
    memory.pvp       = pixels.pvp;
    memory.state     = Memory::INVALID;
    memory.isCompressed = false;
    memory.hasAlpha = false;

    co::base::CompressorInfos transferrers;
    _findTransferers( buffer, 0 /*GLEW context*/, transferrers );

    if( transferrers.empty( ))
        EQWARN << "No upload engines found for given pixel data" << std::endl;
    else
    {
        memory.hasAlpha =
            transferrers.front().capabilities & EQ_COMPRESSOR_IGNORE_ALPHA;
#ifndef NDEBUG
        for( co::base::CompressorInfos::const_iterator i = transferrers.begin();
             i != transferrers.end(); ++i )
        {
            EQASSERTINFO( memory.hasAlpha == 
                          bool( i->capabilities & EQ_COMPRESSOR_IGNORE_ALPHA ),
                          "Uploaders don't agree on alpha state of external " <<
                          "format: " << transferrers.front() << " != " << *i );
        }
#endif
    }

    const uint32_t size = getPixelDataSize( buffer );
    EQASSERT( size > 0 );
    if( size == 0 )
        return;

    if( pixels.compressorName <= EQ_COMPRESSOR_NONE )
    {
        validatePixelData( buffer ); // alloc memory for pixels

        if( pixels.pixels )
        {
            memcpy( memory.pixels, pixels.pixels, size );
            memory.state = Memory::VALID;
        }
        else
            // no data in pixels, clear image buffer
            clearPixelData( buffer );

        return;
    }

    EQASSERT( !pixels.compressedData.empty( ));
    Attachment& attachment = _getAttachment( buffer );
    if( !_allocDecompressor( attachment, pixels.compressorName ))
    {
        EQASSERTINFO( false,
                      "Can't allocate decompressor " << pixels.compressorName <<
                      ", mismatched compressor installation?" );
        return;
    }

    const co::base::CompressorInfo& info = attachment.compressor->getInfo();
    if( memory.externalFormat != info.outputTokenType )
    {
        // decompressor output differs from compressor input
        memory.externalFormat = info.outputTokenType;
        memory.pixelSize = info.outputTokenSize;
    }
    validatePixelData( buffer ); // alloc memory for pixels

    uint64_t outDims[4] = { memory.pvp.x, memory.pvp.w,  
                            memory.pvp.y, memory.pvp.h }; 
    const uint64_t nBlocks = pixels.compressedSize.size();

    EQASSERT( nBlocks == pixels.compressedData.size( ));
    attachment.compressor->decompress( &pixels.compressedData.front(),
                                       &pixels.compressedSize.front(),
                                       nBlocks, memory.pixels, outDims,
                                       pixels.compressorFlags );
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

const Image::Attachment& Image::_getAttachment( const Frame::Buffer buffer ) 
    const
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

/** Find and activate a compression engine */
bool Image::allocCompressor( const Frame::Buffer buffer, const uint32_t name )
{
    Attachment& attachment = _getAttachment( buffer );
    if( name <= EQ_COMPRESSOR_NONE )
    {
        attachment.memory.isCompressed = false;
        attachment.compressor->reset();
        return true;
    }

    if( !attachment.compressor->isValid( name ) )
    {
        attachment.memory.isCompressed = false;
        if( !attachment.compressor->initCompressor( name ) )
            return false;

        EQLOG( LOG_PLUGIN ) << "Instantiated compressor of type 0x" << std::hex
                            << name << std::dec << std::endl;
    }
    return true;
}

/** Find and activate a compression engine */
bool Image::allocDownloader( const Frame::Buffer buffer, 
                             const uint32_t name,
                             const GLEWContext* glewContext )
{
    EQASSERT( name > EQ_COMPRESSOR_NONE )
    EQASSERT( glewContext );

    Attachment& attachment = _getAttachment( buffer );
    util::GPUCompressor* downloader = attachment.transfer;

    if( name <= EQ_COMPRESSOR_NONE )
    {
        downloader->initDownloader( name );
        _setExternalFormat( buffer, EQ_COMPRESSOR_DATATYPE_NONE, 0, true );
        return false;
    }

    if( !downloader->isValid( name ))
    {
        downloader->setGLEWContext( glewContext );
        if( !downloader->initDownloader( name ))
        {
            downloader->setGLEWContext( 0 );
            return false;
        }

        downloader->setGLEWContext( 0 );
        attachment.memory.internalFormat = downloader->getInternalFormat();
        _setExternalFormat( buffer, downloader->getExternalFormat(),
                            downloader->getTokenSize(), downloader->hasAlpha());

        EQLOG( LOG_PLUGIN ) << "Instantiated downloader of type 0x" << std::hex
                            << name << std::dec << std::endl;
    }
    return true;
}

/** Find and activate a decompression engine */
bool Image::_allocDecompressor( Attachment& attachment, uint32_t name )
{
    if( !attachment.compressor->isValid( name ) &&
        !attachment.compressor->initDecompressor( name ))
    {
        return false;
    }
    return true;
}

void Image::Memory::flush()
{
    PixelData::reset();
    state = INVALID;
    localBuffer.clear();
    hasAlpha = true;
}

void Image::Memory::useLocalBuffer()
{
    EQASSERT( internalFormat != 0 );
    EQASSERT( externalFormat != 0 );
    EQASSERT( pixelSize > 0 );
    EQASSERT( pvp.hasArea( ));

    localBuffer.resize( pvp.getArea() * pixelSize );
    pixels = localBuffer.getData();
}

const PixelData& Image::compressPixelData( const Frame::Buffer buffer )
{
    EQASSERT( getPixelDataSize( buffer ) > 0 );

    Attachment& attachment = _getAttachment( buffer );
    Memory& memory = attachment.memory;
    if( memory.isCompressed )
        return memory;

    if( attachment.compressor->isValid( attachment.compressor->getName( )))
        memory.compressorName = attachment.compressor->getName();
    else
    {
        memory.compressorName = _chooseCompressor( buffer );
        if( !allocCompressor( buffer, memory.compressorName ) || 
            memory.compressorName == EQ_COMPRESSOR_NONE )
        {
            EQWARN << "No compressor found for token type 0x" << std::hex 
                   << getExternalFormat( buffer ) << std::dec << std::endl;
            memory.compressorName = EQ_COMPRESSOR_NONE;
        }
    }

    EQASSERT( memory.compressorName != 0 );
    if( memory.compressorName == EQ_COMPRESSOR_NONE )
        return memory;

    memory.compressorFlags = EQ_COMPRESSOR_DATA_2D;
    if( _ignoreAlpha && memory.hasAlpha )
    {
        EQASSERT( buffer == Frame::BUFFER_COLOR );
        memory.compressorFlags |= EQ_COMPRESSOR_IGNORE_ALPHA;
    }

    const uint64_t inDims[4] = { memory.pvp.x, memory.pvp.w,
                                 memory.pvp.y, memory.pvp.h }; 
    attachment.compressor->compress( memory.pixels, inDims,
                                     memory.compressorFlags );

    const unsigned numResults = attachment.compressor->getNumResults();
    
    memory.compressedSize.resize( numResults );
    memory.compressedData.resize( numResults );

    for( unsigned i = 0; i < numResults ; ++i )
        attachment.compressor->getResult( i, &memory.compressedData[i], 
                                          &memory.compressedSize[i] );

    memory.isCompressed = true;
    return memory;
}


//---------------------------------------------------------------------------
// File IO
//---------------------------------------------------------------------------

bool Image::writeImages( const std::string& filenameTemplate ) const
{
    return( writeImage( filenameTemplate + "_color.rgb", Frame::BUFFER_COLOR) &&
            writeImage( filenameTemplate + "_depth.rgb", Frame::BUFFER_DEPTH ));
}

#define SWAP_SHORT(v) ( v = (v&0xff) << 8 | (v&0xff00) >> 8 )
#define SWAP_INT(v)   ( v = (v&0xff) << 24 | (v&0xff00) << 8 |      \
                        (v&0xff0000) >> 8 | (v&0xff000000) >> 24)

#ifdef _WIN32
#  pragma pack(1)
#endif
/** @cond IGNORE */
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
#if defined(__i386__) || defined(__amd64__) || defined (__ia64) || \
    defined(__x86_64) || defined(_WIN32)
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
/** @endcond */
#ifndef _WIN32
  __attribute__((packed))
#endif
;

bool Image::writeImage( const std::string& filename,
                        const Frame::Buffer buffer ) const
{
    const Memory& memory = _getMemory( buffer );

    const PixelViewport& pvp = memory.pvp;
    const size_t  nPixels = pvp.w * pvp.h;

    if( nPixels == 0 || memory.state != Memory::VALID )
        return false;

    std::ofstream image( filename.c_str(), std::ios::out | std::ios::binary );
    if( !image.is_open( ))
    {
        EQERROR << "Can't open " << filename << " for writing" << std::endl;
        return false;
    }

    RGBHeader    header;

    header.width  = pvp.w;
    header.height = pvp.h;

    switch( getExternalFormat( buffer ))
    {      
        case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
            header.maxValue = 1023;
        case EQ_COMPRESSOR_DATATYPE_BGRA:
        case EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV:        
        case EQ_COMPRESSOR_DATATYPE_RGBA:
        case EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV:
            header.bytesPerChannel = 1;
            header.depth = 4;
            break;
        case EQ_COMPRESSOR_DATATYPE_BGR:
        case EQ_COMPRESSOR_DATATYPE_RGB:
            header.bytesPerChannel = 1;
            header.depth = 3;
            break;
        case EQ_COMPRESSOR_DATATYPE_BGRA32F:
        case EQ_COMPRESSOR_DATATYPE_RGBA32F:
            header.bytesPerChannel = 4;
            header.depth = 4;
            break;
        case EQ_COMPRESSOR_DATATYPE_BGR32F:
        case EQ_COMPRESSOR_DATATYPE_RGB32F:
            header.bytesPerChannel = 4;
            header.depth = 3;
            break;
        case EQ_COMPRESSOR_DATATYPE_BGRA16F:
        case EQ_COMPRESSOR_DATATYPE_RGBA16F:
            header.bytesPerChannel = 2;
            header.depth = 4;
            break;
        case EQ_COMPRESSOR_DATATYPE_BGR16F:
        case EQ_COMPRESSOR_DATATYPE_RGB16F:
            header.bytesPerChannel = 2;
            header.depth = 3;
            break;
        case EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT:
            header.bytesPerChannel = 4;
            header.depth = 1;
            break;

        default:
            EQERROR << "Unknown image pixel data type" << std::endl;
            return false;
    }

    // if the data picture has a RGB format, we can easy translate it in 
    // a BGR format
    bool invertChannel = false;
    switch( getExternalFormat( buffer ))
    {      
        case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
        case EQ_COMPRESSOR_DATATYPE_RGBA:
        case EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV:
        case EQ_COMPRESSOR_DATATYPE_RGB:
        case EQ_COMPRESSOR_DATATYPE_RGBA32F:
        case EQ_COMPRESSOR_DATATYPE_RGB32F:
        case EQ_COMPRESSOR_DATATYPE_RGBA16F:
        case EQ_COMPRESSOR_DATATYPE_RGB16F:
            invertChannel = true;
    }

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

    if( nChannels == 3 || nChannels == 4 )
    {
        // channel one is R or B
        if ( invertChannel )
        {
            for( size_t j = 0 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
        }
        else
        {
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
        }

        // channel two is G
        for( size_t j = 1 * bpc; j < nBytes; j += depth )
            image.write( &data[j], bpc );

        // channel three is B or G
        if ( invertChannel )
        {
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
        }
        else
        {
            for( size_t j = 0; j < nBytes; j += depth )
                image.write( &data[j], bpc );
        }

        // channel four is Alpha
        if( nChannels == 4 )
        {
            // invert alpha
            for( size_t j = 3 * bpc; j < nBytes; j += depth )
            {
                if( bpc == 1 && header.maxValue == 255 )
                {
                    const uint8_t val = 255 - 
                        *reinterpret_cast< const uint8_t* >( &data[j] );
                    image.write( reinterpret_cast<const char*>( &val ), 1 );
                }
                else
                    image.write( &data[j], bpc );
            }
        }
    }
    else
    {
        for( size_t i = 0; i < nChannels; i += bpc )
           for( size_t j = i * bpc; j < nBytes; j += depth )
              image.write(&data[j], bpc );
    }

    image.close();
    return true;
}

bool Image::readImage( const std::string& filename, const Frame::Buffer buffer )
{
    co::base::MemoryMap image;
    const uint8_t* addr = static_cast< const uint8_t* >( image.map( filename ));

    if( !addr )
    {
        EQINFO << "Can't open " << filename << " for reading" << std::endl;
        return false;
    }

    const size_t size = image.getSize();
    if( size < sizeof( RGBHeader ))
    {
        EQWARN << "Image " << filename << " too small" << std::endl;
        return false;
    }

    RGBHeader header;
    memcpy( &header, addr, sizeof( header ));
    addr += sizeof( header );

    header.convert();

    if( header.magic != 474)
    {
        EQERROR << "Bad magic number " << filename << std::endl;
        return false;
    }
    if( header.width == 0 || header.height == 0 )
    {
        EQERROR << "Zero-sized image " << filename << std::endl;
        return false;
    }
    if( header.compression != 0)
    {
        EQERROR << "Unsupported compression " << filename << std::endl;
        return false;
    }

    const unsigned nChannels = header.depth;

    if( header.nDimensions != 3 ||
        header.minValue != 0 ||
        ( header.maxValue != 255 && header.maxValue != 1023 ) ||
        header.colorMode != 0 ||
        ( buffer == Frame::BUFFER_COLOR && nChannels != 3 && nChannels != 4 ) ||
        ( buffer == Frame::BUFFER_DEPTH && nChannels != 4 ))
    {
        EQERROR << "Unsupported image type " << filename << std::endl;
        return false;
    }

    if(( header.bytesPerChannel != 1 || nChannels == 1 ) &&
         header.maxValue != 255 )
    {
        EQERROR << "Unsupported value range " << header.maxValue << std::endl;
        return false;
    }

    const uint8_t bpc     = header.bytesPerChannel;
    const size_t  depth   = nChannels * bpc;
    const size_t  nPixels = header.width * header.height;
    const size_t  nComponents = nPixels * nChannels;
    const size_t  nBytes  = nComponents * bpc;

    if( size < sizeof( RGBHeader ) + nBytes )
    {
        EQERROR << "Image " << filename << " too small" << std::endl;
        return false;
    }
    EQASSERT( size == sizeof( RGBHeader ) + nBytes );

    switch( buffer )
    {
        case Frame::BUFFER_DEPTH:
            if( header.bytesPerChannel != 1 )
            {
                EQERROR << "Unsupported channel depth " 
                        << static_cast< int >( header.bytesPerChannel )
                        << std::endl;
                return false;
            }
            _setExternalFormat( Frame::BUFFER_DEPTH,
                                EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT, 4,
                                false );
            setInternalFormat( Frame::BUFFER_DEPTH,
                               EQ_COMPRESSOR_DATATYPE_DEPTH );
            break;

        case Frame::BUFFER_COLOR:
            switch( header.bytesPerChannel )
            {
                case 1:
                    if( header.maxValue == 1023 )
                    {
                        EQASSERT( nChannels==4 );
                        _setExternalFormat( Frame::BUFFER_COLOR,
                                            EQ_COMPRESSOR_DATATYPE_RGB10_A2, 4,
                                            true );
                        setInternalFormat( Frame::BUFFER_COLOR,
                                           EQ_COMPRESSOR_DATATYPE_RGB10_A2 );
                    }
                    else
                    {
                        if( nChannels == 4 )
                            _setExternalFormat( Frame::BUFFER_COLOR,
                                                EQ_COMPRESSOR_DATATYPE_RGBA,
                                                4, true );
                        else
                        {
                            EQASSERT( nChannels == 3 );
                            _setExternalFormat( Frame::BUFFER_COLOR,
                                                EQ_COMPRESSOR_DATATYPE_RGB,
                                                nChannels, false );
                        }
                        setInternalFormat( Frame::BUFFER_COLOR,
                                           EQ_COMPRESSOR_DATATYPE_RGBA );
                    }
                    break;

                case 2:
                    if( nChannels == 4 )
                        _setExternalFormat( Frame::BUFFER_COLOR,
                                            EQ_COMPRESSOR_DATATYPE_RGBA16F,
                                            8, true );
                    else
                    {
                        EQASSERT( nChannels == 3 );
                        _setExternalFormat( Frame::BUFFER_COLOR,
                                            EQ_COMPRESSOR_DATATYPE_RGB16F,
                                            nChannels * 2, false );
                    }

                    setInternalFormat( Frame::BUFFER_COLOR,
                                       EQ_COMPRESSOR_DATATYPE_RGBA16F );
                    break;

                case 4:
                    if( nChannels == 4 )
                        _setExternalFormat( Frame::BUFFER_COLOR,
                                            EQ_COMPRESSOR_DATATYPE_RGBA32F,
                                            16, true );
                    else
                    {
                        EQASSERT( nChannels == 3 );
                        _setExternalFormat( Frame::BUFFER_COLOR,
                                            EQ_COMPRESSOR_DATATYPE_RGBA32F,
                                            nChannels *4, false );
                    }
                    setInternalFormat( Frame::BUFFER_COLOR,
                                       EQ_COMPRESSOR_DATATYPE_RGBA32F );
                    break;

                default:
                    EQERROR << "Unsupported channel depth " 
                            << static_cast< int >( header.bytesPerChannel )
                            << std::endl;
                    return false;
            }
            break;

        default:
            EQUNREACHABLE;
    }
    Memory& memory = _getMemory( buffer );
    const PixelViewport pvp( 0, 0, header.width, header.height );
    if( pvp != _pvp )
    {
        setPixelViewport( pvp );
    }
    
    if ( memory.pvp != pvp )
    {
        memory.pvp = pvp;
        memory.state = Memory::INVALID;
    }
    validatePixelData( buffer );

    uint8_t* data = reinterpret_cast< uint8_t* >( memory.pixels );
    EQASSERTINFO( nBytes <= getPixelDataSize( buffer ),
                  nBytes << " > " << getPixelDataSize( buffer ));
    // Each channel is saved separately
    switch( bpc )
    {
        case 1:
            for( size_t i = 0; i < nChannels; ++i )
            {
                for( size_t j = i; j < nComponents; j += nChannels )
                {
                    data[j] = *addr;
                    ++addr;
                }
            }
            break;

        case 2:
            for( size_t i = 0; i < nChannels; ++i )
            {
                for( size_t j = i; j < nComponents; j += nChannels )
                {
                    reinterpret_cast< uint16_t* >( data )[ j ] = 
                        *reinterpret_cast< const uint16_t* >( addr );
                    addr += bpc;
                }
            }
            break;

        case 4:
            for( size_t i = 0; i < nChannels; ++i )
            {
                for( size_t j = i; j < nComponents; j += nChannels )
                {
                    reinterpret_cast< uint32_t* >( data )[ j ] = 
                        *reinterpret_cast< const uint32_t* >( addr );
                    addr += bpc;
                }
            }
            break;

        default:
            for( size_t i = 0; i < depth; i += bpc )
            {
                for( size_t j = i * bpc; j < nBytes; j += depth )
                {
                    memcpy( &data[j], addr, bpc );
                    addr += bpc;
                }
            }
    }
    return true;
}

}
