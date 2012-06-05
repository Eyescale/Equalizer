
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "log.h"
#include "pixelData.h"
#include "windowSystem.h"

#include <eq/util/frameBufferObject.h>
#include <eq/util/objectManager.h>

#include <eq/fabric/colorMask.h>

#include <co/global.h>
#include <co/pluginRegistry.h>
#include <lunchbox/memoryMap.h>
#include <lunchbox/omp.h>

// Internal headers
#include "../../co/plugin.h"
#include "../../co/compressorInfo.h"
#include "../../co/cpuCompressor.h"
#include "../util/gpuCompressor.h"

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

namespace
{
/** @internal Raw image data. */
struct Memory : public PixelData
{
public:
    Memory() : state( INVALID ) {}

    void flush()
    {
        PixelData::reset();
        state = INVALID;
        localBuffer.clear();
        hasAlpha = true;
    }

    void useLocalBuffer()
    {
        LBASSERT( internalFormat != 0 );
        LBASSERT( externalFormat != 0 );
        LBASSERT( pixelSize > 0 );
        LBASSERT( pvp.hasArea( ));

        localBuffer.resize( pvp.getArea() * pixelSize );
        pixels = localBuffer.getData();
    }

    enum State
    {
        INVALID,
        VALID,
        DOWNLOAD // async RB is in progress
    };

    State state;   //!< The current state of the memory

    /** During the call of setPixelData or writeImage, we have to
        manage an internal buffer to copy the data */
    lunchbox::Bufferb localBuffer;

    bool hasAlpha; //!< The uncompressed pixels contain alpha
};

/** @internal The individual parameters for a buffer. */
struct Attachment
{
    co::CPUCompressor* const fullCompressor;
    co::CPUCompressor* const lossyCompressor;

    util::GPUCompressor* const fullTransfer;
    util::GPUCompressor* const lossyTransfer;

    co::CPUCompressor* compressor; //!< current CPU (de)compressor
    util::GPUCompressor* transfer;   //!< current up/download engine

    float quality; //!< the minimum quality

    /** The texture name for this image component (texture images). */
    util::Texture texture;

    /** Current pixel data (memory images). */
    Memory memory;

    Attachment()
            : fullCompressor( new co::CPUCompressor )
            , lossyCompressor( new co::CPUCompressor )
            , fullTransfer( new util::GPUCompressor )
            , lossyTransfer( new util::GPUCompressor )
            , compressor( fullCompressor )
            , transfer ( fullTransfer )
            , quality( 1.f )
            , texture( GL_TEXTURE_RECTANGLE_ARB )
        {}

    ~Attachment()
    {
        delete fullCompressor;
        delete lossyCompressor;
        delete fullTransfer;
        delete lossyTransfer;
    }

    void flush()
    {
        memory.flush();
        texture.flush();
        fullCompressor->reset();
        lossyCompressor->reset();
        fullTransfer->reset();
        lossyTransfer->reset();
    }
};

/** Find and activate a decompression engine */
static bool _allocDecompressor( Attachment& attachment, uint32_t name )
{
    if( !attachment.compressor->isValid( name ) &&
        !attachment.compressor->initDecompressor( name ))
    {
        return false;
    }
    return true;
}
}

namespace detail
{
class Image
{
public:
    Image() : type( eq::Frame::TYPE_MEMORY ), ignoreAlpha( false ) {}

    /** The rectangle of the current pixel data. */
    PixelViewport pvp;

    /** Zoom factor used for compositing. */
    Zoom zoom;

    /** The storage type for the pixel data. */
    eq::Frame::Type type;

    Attachment color;
    Attachment depth;

    /** Alpha channel significance. */
    bool ignoreAlpha;

    Attachment& getAttachment( const eq::Frame::Buffer buffer )
    {
        switch( buffer )
        {
          case eq::Frame::BUFFER_COLOR:
              return color;
          case eq::Frame::BUFFER_DEPTH:
              return depth;
          default:
              LBUNIMPLEMENTED;
        }
        return color;
    }

    const Attachment& getAttachment( const eq::Frame::Buffer buffer ) const
    {
        switch( buffer )
        {
          case eq::Frame::BUFFER_COLOR:
              return color;
          case eq::Frame::BUFFER_DEPTH:
              return depth;
          default:
              LBUNIMPLEMENTED;
        }
        return color;
    }

    Memory& getMemory( const eq::Frame::Buffer buffer )
        { return getAttachment( buffer ).memory; }
    const Memory& getMemory( const eq::Frame::Buffer buffer ) const
        { return getAttachment( buffer ).memory; }

};
}

Image::Image()
        : _impl( new detail::Image )
{
    reset();
}

Image::~Image()
{
    delete _impl;
}

void Image::reset()
{
    _impl->ignoreAlpha = false;
    setPixelViewport( PixelViewport( ));
}

void Image::flush()
{
    _impl->color.flush();
    _impl->depth.flush();
}

uint32_t Image::getPixelDataSize( const Frame::Buffer buffer ) const
{
    const Memory& memory = _impl->getMemory( buffer );
    return memory.pvp.getArea() * memory.pixelSize;
}


void Image::_setExternalFormat( const Frame::Buffer buffer,
                                const uint32_t externalFormat,
                                const uint32_t pixelSize, const bool hasAlpha_ )
{
    Memory& memory = _impl->getMemory( buffer );
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
    Memory& memory = _impl->getMemory( buffer );
    if( memory.internalFormat == internalFormat )
        return;

    memory.internalFormat = internalFormat;
    allocCompressor( buffer, EQ_COMPRESSOR_INVALID );
    if( internalFormat == 0 )
        return;
}

uint32_t Image::getInternalFormat( const Frame::Buffer buffer ) const
{
    const Memory& memory = _impl->getMemory( buffer );
    LBASSERT( memory.internalFormat );
    return memory.internalFormat;
}

std::vector< uint32_t > Image::findCompressors( const Frame::Buffer buffer )
    const
{
    const co::PluginRegistry& registry = co::Global::getPluginRegistry();
    const co::Plugins& plugins = registry.getPlugins();
    const uint32_t tokenType = getExternalFormat( buffer );

    std::vector< uint32_t > names;
    for( co::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const co::Plugin* plugin = *i;
        const co::CompressorInfos& infos = plugin->getInfos();

        for( co::CompressorInfosCIter j = infos.begin(); j != infos.end(); ++j )
        {
            const co::CompressorInfo& info = *j;

            if( info.capabilities & EQ_COMPRESSOR_TRANSFER )
                continue;

            if( info.tokenType == tokenType )
                names.push_back( info.name );
        }
    }

    LBLOG( LOG_PLUGIN )
        << "Found " << names.size() << " compressors for token type 0x"
        << std::hex << tokenType << std::dec << std::endl;
    return names;
}

void Image::findTransferers( const Frame::Buffer buffer,
                             const GLEWContext* glewContext,
                             std::vector< uint32_t >& names )
{
    co::CompressorInfos infos;
    _findTransferers( buffer, glewContext, infos );
    for( co::CompressorInfosCIter i = infos.begin(); i != infos.end(); ++i )
        names.push_back( i->name );
}

void Image::_findTransferers( const Frame::Buffer buffer,
                              const GLEWContext* glewContext,
                              co::CompressorInfos& result )
{
    util::GPUCompressor::findTransferers(
        getInternalFormat( buffer ), getExternalFormat( buffer ),
        0 /*caps*/, getQuality( buffer ), _impl->ignoreAlpha, glewContext, result );
}

uint32_t Image::_chooseCompressor( const Frame::Buffer buffer ) const
{
    const uint32_t tokenType = getExternalFormat( buffer );
    LBASSERT( tokenType != EQ_COMPRESSOR_DATATYPE_NONE );
    if( tokenType == EQ_COMPRESSOR_DATATYPE_NONE )
        return EQ_COMPRESSOR_NONE;

    const Attachment& attachment = _impl->getAttachment( buffer );
    const float quality = attachment.quality /
                          attachment.lossyTransfer->getQuality();

    return co::CPUCompressor::chooseCompressor(tokenType, quality,_impl->ignoreAlpha);
}

bool Image::hasAlpha() const
{
    return hasPixelData( Frame::BUFFER_COLOR ) &&
           _impl->getMemory( Frame::BUFFER_COLOR ).hasAlpha;
}

void Image::setAlphaUsage( const bool enabled )
{
    if( _impl->ignoreAlpha != enabled )
        return;

    _impl->ignoreAlpha = !enabled;
    _impl->color.memory.isCompressed = false;
    _impl->depth.memory.isCompressed = false;
}

void Image::setQuality( const Frame::Buffer buffer, const float quality )
{
    Attachment& attachment = _impl->getAttachment( buffer );
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
    return _impl->getAttachment( buffer ).quality;
}

bool Image::hasTextureData( const Frame::Buffer buffer ) const
{
    return getTexture( buffer ).isValid();
}

const util::Texture& Image::getTexture( const Frame::Buffer buffer ) const
{
    return _impl->getAttachment( buffer ).texture;
}

const uint8_t* Image::getPixelPointer( const Frame::Buffer buffer ) const
{
    LBASSERT( hasPixelData( buffer ));
    return reinterpret_cast< const uint8_t* >( _impl->getMemory( buffer ).pixels );
}

uint8_t* Image::getPixelPointer( const Frame::Buffer buffer )
{
    LBASSERT( hasPixelData( buffer ));
    return  reinterpret_cast< uint8_t* >( _impl->getMemory( buffer ).pixels );
}

const PixelData& Image::getPixelData( const Frame::Buffer buffer ) const
{
    LBASSERT( hasPixelData( buffer ));
    return _impl->getMemory( buffer );
}

void Image::upload( const Frame::Buffer buffer, util::Texture* texture,
                    const Vector2i& position, ObjectManager* glObjects ) const
{
    LBASSERT( glObjects );

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
        texture->init( internalFormat, _impl->pvp.w, _impl->pvp.h );

    uploader->upload( pixelData.pixels, pixelData.pvp, flags, pvp,
                      texture ? texture->getName() : 0 );
}

//---------------------------------------------------------------------------
// asynchronous readback
//---------------------------------------------------------------------------
#ifndef EQ_2_0_API
bool Image::readback( const uint32_t buffers, const PixelViewport& pvp,
                      const Zoom& zoom, ObjectManager* glObjects )
{
    if( startReadback( buffers, pvp, zoom, glObjects ))
        finishReadback( zoom, glewGetContext( ));
    return true;
}
#endif

bool Image::startReadback( const uint32_t buffers, const PixelViewport& pvp,
                           const Zoom& zoom, ObjectManager* glObjects )
{
    LBASSERT( glObjects );
    LBLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers
                          << std::endl;

    _impl->pvp = pvp;
    _impl->color.memory.state = Memory::INVALID;
    _impl->depth.memory.state = Memory::INVALID;

    bool needFinish = (buffers & Frame::BUFFER_COLOR) &&
                         _startReadback( Frame::BUFFER_COLOR, zoom, glObjects );
    if( (buffers & Frame::BUFFER_DEPTH) &&
        _startReadback( Frame::BUFFER_DEPTH, zoom, glObjects ))
    {
        needFinish = true;
    }

    _impl->pvp.x = 0;
    _impl->pvp.y = 0;
    return needFinish;
}

bool Image::_startReadback( const Frame::Buffer buffer, const Zoom& zoom,
                            ObjectManager* glObjects )
{
    Attachment& attachment = _impl->getAttachment( buffer );
    attachment.memory.isCompressed = false;

    if( _impl->type == Frame::TYPE_TEXTURE )
    {
        LBASSERTINFO( zoom == Zoom::NONE, "Texture readback zoom not " <<
                      "implemented, zoom happens during compositing" );
        util::Texture& texture = attachment.texture;
        texture.setGLEWContext( glewGetContext( ));
        texture.copyFromFrameBuffer( getInternalFormat( buffer ), _impl->pvp );
        texture.setGLEWContext( 0 );
        return false;
    }

    if( zoom == Zoom::NONE ) // normal framebuffer readback
        return startReadback( buffer, 0, glewGetContext( ));

    // else copy to texture, draw zoomed quad into FBO, (read FBO texture)
    return _readbackZoom( buffer, zoom, glObjects );
}

bool Image::startReadback( const Frame::Buffer buffer,
                           const util::Texture* texture,
                           const GLEWContext* glewContext )
{
    Attachment& attachment = _impl->getAttachment( buffer );
    util::GPUCompressor* downloader = attachment.transfer;
    Memory& memory = attachment.memory;
    const uint32_t inputToken = memory.internalFormat;

    downloader->setGLEWContext( glewContext );

    uint32_t flags = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                     ( texture ? texture->getCompressorTarget() :
                                 EQ_COMPRESSOR_USE_FRAMEBUFFER );

    const bool alpha = _impl->ignoreAlpha && buffer == Frame::BUFFER_COLOR;
    if( !downloader->isValidDownloader( inputToken, alpha, flags ) &&
        !downloader->initDownloader( inputToken, attachment.quality, alpha,
                                     flags ))
    {
        LBWARN << "Download plugin initialization failed" << std::endl;
        return false;
    }

    // get the pixel type produced by the downloader
    _setExternalFormat( buffer, downloader->getExternalFormat(),
                        downloader->getTokenSize(), downloader->hasAlpha( ));
    attachment.memory.state = Memory::DOWNLOAD;

    if( !memory.hasAlpha )
        flags |= EQ_COMPRESSOR_IGNORE_ALPHA;

    const bool needFinish = texture ?
        downloader->startDownload( PixelViewport( 0, 0, texture->getWidth(),
                                                  texture->getHeight( )),
                                   texture->getName(), flags,
                                   memory.pvp, &memory.pixels ) :
        downloader->startDownload( _impl->pvp, 0, flags, memory.pvp, &memory.pixels );

    downloader->setGLEWContext( 0 );

    if( !needFinish )
        attachment.memory.state = Memory::VALID;
    return needFinish;
}

void Image::finishReadback( const Zoom& zoom, const GLEWContext* glewContext )
{
    LBASSERT( glewContext );
    LBLOG( LOG_ASSEMBLY ) << "finishReadback, zoom " << zoom
                          << std::endl;

    _finishReadback( Frame::BUFFER_COLOR, zoom, glewContext );
    _finishReadback( Frame::BUFFER_DEPTH, zoom, glewContext );

#ifndef NDEBUG
    if( getenv( "EQ_DUMP_IMAGES" ))
    {
        static a_int32_t counter;
        std::ostringstream stringstream;

        stringstream << "Image_" << std::setfill( '0' ) << std::setw(5)
                     << ++counter;
        writeImages( stringstream.str( ));
    }
#endif
}

void Image::_finishReadback( const Frame::Buffer buffer, const Zoom& zoom,
                             const GLEWContext* glewContext )
{
    if( _impl->type == Frame::TYPE_TEXTURE )
        return;

    Attachment& attachment = _impl->getAttachment( buffer );
    util::GPUCompressor* downloader = attachment.transfer;
    Memory& memory = attachment.memory;
    const uint32_t inputToken = memory.internalFormat;

    if( memory.state != Memory::DOWNLOAD )
        return;

    downloader->setGLEWContext( glewContext );

    uint32_t flags = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                     ( zoom == Zoom::NONE ? EQ_COMPRESSOR_USE_FRAMEBUFFER :
                                            EQ_COMPRESSOR_USE_TEXTURE_RECT );

    const bool alpha = _impl->ignoreAlpha && buffer == Frame::BUFFER_COLOR;
    if( !downloader->isValidDownloader( inputToken, alpha, flags ))
    {
        LBWARN << "Download plugin initialization failed" << std::endl;
        attachment.memory.state = Memory::INVALID;
        return;
    }

    if( !memory.hasAlpha )
        flags |= EQ_COMPRESSOR_IGNORE_ALPHA;

    if( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
        downloader->finishDownload( _impl->pvp, flags, memory.pvp, &memory.pixels );
    else
    {
        PixelViewport pvp = _impl->pvp;
        pvp.apply( zoom );
        pvp.x = 0;
        pvp.y = 0;
        downloader->finishDownload( pvp, flags, memory.pvp, &memory.pixels );
    }

    downloader->setGLEWContext( 0 );
    memory.state = Memory::VALID;
}

bool Image::_readbackZoom( const Frame::Buffer buffer, const Zoom& zoom,
                           ObjectManager* glObjects )
{
    LBASSERT( glObjects );
    LBASSERT( glObjects->supportsEqTexture( ));
    LBASSERT( glObjects->supportsEqFrameBufferObject( ));

    PixelViewport pvp = _impl->pvp;
    pvp.apply( zoom );
    if( !pvp.hasArea( ))
        return false;

    // copy frame buffer to texture
    const void* bufferKey = _getBufferKey( buffer );
    util::Texture* texture =
        glObjects->obtainEqTexture( bufferKey, GL_TEXTURE_RECTANGLE_ARB );

    texture->copyFromFrameBuffer( getInternalFormat( buffer ), _impl->pvp );

    // draw zoomed quad into FBO
    //  uses the same FBO for color and depth, with masking.
    const void* fboKey = _getBufferKey( Frame::BUFFER_COLOR );
    util::FrameBufferObject* fbo = glObjects->getEqFrameBufferObject( fboKey );

    if( fbo )
    {
        LBCHECK( fbo->resize( pvp.w, pvp.h ));
    }
    else
    {
        fbo = glObjects->newEqFrameBufferObject( fboKey );
        LBCHECK( fbo->init( pvp.w, pvp.h, getInternalFormat( buffer ), 24, 0 ));
    }
    fbo->bind();
    texture->bind();

    if( buffer == Frame::BUFFER_COLOR )
        glDepthMask( false );
    else
    {
        LBASSERT( buffer == Frame::BUFFER_DEPTH )
        glColorMask( false, false, false, false );
    }

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    texture->applyZoomFilter( FILTER_LINEAR );
    glColor3f( 1.0f, 1.0f, 1.0f );

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( 0, 0, 0.0f );

        glTexCoord2f( static_cast< float >( _impl->pvp.w ), 0.0f );
        glVertex3f( static_cast< float >( pvp.w ), 0, 0.0f );

        glTexCoord2f( static_cast< float >( _impl->pvp.w ),
                      static_cast< float >( _impl->pvp.h ));
        glVertex3f( static_cast< float >( pvp.w ),
                    static_cast< float >( pvp.h ), 0.0f );

        glTexCoord2f( 0.0f, static_cast< float >( _impl->pvp.h ));
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

    LBLOG( LOG_ASSEMBLY ) << "Scale " << _impl->pvp << " -> " << pvp << std::endl;

// BUG:
// TODO: this is a bug in case of color and depth buffers read-back, as _impl->pvp
//       will be incorrect for the depth buffer!
//
//       This should be done separately for color an depth buffers!
    _impl->pvp = pvp;

    LBLOG( LOG_ASSEMBLY ) << "Read texture " << getPixelDataSize( buffer )
                          << std::endl;
    return startReadback( buffer, zoomedTexture, glewGetContext( ));
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
            LBUNIMPLEMENTED;
            return ( reinterpret_cast< const char* >( this ) + 2 );
    }
}

const void* Image::_getCompressorKey( const Frame::Buffer buffer ) const
{
    const Attachment& attachment = _impl->getAttachment( buffer );

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
            LBUNIMPLEMENTED;
            return ( reinterpret_cast< const char* >( this ) + 0 );
    }
}

void Image::setPixelViewport( const PixelViewport& pvp )
{
    _impl->pvp = pvp;
    _impl->color.memory.state = Memory::INVALID;
    _impl->depth.memory.state = Memory::INVALID;
    _impl->color.memory.isCompressed = false;
    _impl->depth.memory.isCompressed = false;
}

void Image::clearPixelData( const Frame::Buffer buffer )
{
    Memory& memory = _impl->getAttachment( buffer ).memory;
    memory.pvp = _impl->pvp;
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

#ifdef CO_USE_OPENMP
#pragma omp parallel for
#endif
        for( ssize_t i = 3; i < size; i+=4 )
            data[i] = 255;
#endif
        break;
      }
      default:
        LBWARN << "Unknown external format " << memory.externalFormat
               << ", initializing to 0" << std::endl;
        bzero( memory.pixels, size );
        break;
    }
}

void Image::validatePixelData( const Frame::Buffer buffer )
{
    Memory& memory = _impl->getAttachment( buffer ).memory;
    memory.useLocalBuffer();
    memory.state = Memory::VALID;
    memory.isCompressed = false;
}

void Image::setPixelData( const Frame::Buffer buffer, const PixelData& pixels )
{
    Memory& memory = _impl->getMemory( buffer );
    memory.externalFormat = pixels.externalFormat;
    memory.internalFormat = pixels.internalFormat;
    memory.pixelSize = pixels.pixelSize;
    memory.pvp       = pixels.pvp;
    memory.state     = Memory::INVALID;
    memory.isCompressed = false;
    memory.hasAlpha = false;

    co::CompressorInfos transferrers;
    _findTransferers( buffer, 0 /*GLEW context*/, transferrers );

    if( transferrers.empty( ))
        LBWARN << "No upload engines found for given pixel data" << std::endl;
    else
    {
        memory.hasAlpha =
            transferrers.front().capabilities & EQ_COMPRESSOR_IGNORE_ALPHA;
#ifndef NDEBUG
        for( co::CompressorInfosCIter i = transferrers.begin();
             i != transferrers.end(); ++i )
        {
            LBASSERTINFO( memory.hasAlpha ==
                          bool( i->capabilities & EQ_COMPRESSOR_IGNORE_ALPHA ),
                          "Uploaders don't agree on alpha state of external " <<
                          "format: " << transferrers.front() << " != " << *i );
        }
#endif
    }

    const uint32_t size = getPixelDataSize( buffer );
    LBASSERT( size > 0 );
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

    LBASSERT( !pixels.compressedData.empty( ));
    LBASSERT( pixels.compressorName != EQ_COMPRESSOR_AUTO );

    Attachment& attachment = _impl->getAttachment( buffer );
    if( !_allocDecompressor( attachment, pixels.compressorName ))
    {
        LBASSERTINFO( false,
                      "Can't allocate decompressor " << pixels.compressorName <<
                      ", mismatched compressor plugin installation?" );
        return;
    }

    const co::CompressorInfo& info = attachment.compressor->getInfo();
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

    LBASSERT( nBlocks == pixels.compressedData.size( ));
    attachment.compressor->decompress( &pixels.compressedData.front(),
                                       &pixels.compressedSize.front(),
                                       nBlocks, memory.pixels, outDims,
                                       pixels.compressorFlags );
}

/** Find and activate a compression engine */
bool Image::allocCompressor( const Frame::Buffer buffer, const uint32_t name )
{
    Attachment& attachment = _impl->getAttachment( buffer );
    if( name <= EQ_COMPRESSOR_NONE )
    {
        attachment.memory.isCompressed = false;
        attachment.compressor->reset();
        return true;
    }

    if( !attachment.compressor->isValid( name ) )
    {
        attachment.memory.isCompressed = false;
        if( !attachment.compressor->co::Compressor::initCompressor( name ))
            return false;

        LBLOG( LOG_PLUGIN ) << "Instantiated compressor of type 0x" << std::hex
                            << name << std::dec << std::endl;
    }
    return true;
}

/** Find and activate a compression engine */
bool Image::allocDownloader( const Frame::Buffer buffer,
                             const uint32_t name,
                             const GLEWContext* glewContext )
{
    LBASSERT( name > EQ_COMPRESSOR_NONE )
    LBASSERT( glewContext );

    Attachment& attachment = _impl->getAttachment( buffer );
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

        LBLOG( LOG_PLUGIN ) << "Instantiated downloader of type 0x" << std::hex
                            << name << std::dec << std::endl;
    }
    return true;
}

uint32_t Image::getDownloaderName( const Frame::Buffer buffer ) const
{
    const Attachment& attachment = _impl->getAttachment( buffer );
    const uint32_t name = attachment.transfer->getName();
    if( attachment.transfer->isValid( name ))
        return name;
    return EQ_COMPRESSOR_INVALID;
}

void Image::useCompressor( const Frame::Buffer buffer, const uint32_t name )
{
    _impl->getMemory( buffer ).compressorName = name;
}

const PixelData& Image::compressPixelData( const Frame::Buffer buffer )
{
    LBASSERT( getPixelDataSize( buffer ) > 0 );

    Attachment& attachment = _impl->getAttachment( buffer );
    Memory& memory = attachment.memory;
    if( memory.isCompressed || memory.compressorName == EQ_COMPRESSOR_NONE )
    {
        LBASSERT( memory.compressorName != EQ_COMPRESSOR_AUTO );
        return memory;
    }

    const co::CPUCompressor* compressor = attachment.compressor;

    if( compressor->isValid( attachment.compressor->getName( )) &&
        compressor->getInfo().tokenType == getExternalFormat( buffer ) &&
        memory.compressorName != EQ_COMPRESSOR_AUTO )
    {
        memory.compressorName = compressor->getName();
    }
    else
    {
        if( memory.compressorName == EQ_COMPRESSOR_AUTO )
            memory.compressorName = _chooseCompressor( buffer );

        if( !allocCompressor( buffer, memory.compressorName ) ||
            memory.compressorName == EQ_COMPRESSOR_NONE )
        {
            LBWARN << "No compressor found for token type 0x" << std::hex
                   << getExternalFormat( buffer ) << std::dec << std::endl;
            memory.compressorName = EQ_COMPRESSOR_NONE;
        }
    }

    LBASSERT( memory.compressorName != EQ_COMPRESSOR_AUTO );
    LBASSERT( memory.compressorName != EQ_COMPRESSOR_INVALID );
    if( memory.compressorName == EQ_COMPRESSOR_NONE )
        return memory;

    memory.compressorFlags = EQ_COMPRESSOR_DATA_2D;
    if( _impl->ignoreAlpha && memory.hasAlpha )
    {
        LBASSERT( buffer == Frame::BUFFER_COLOR );
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
    return( writeImage( filenameTemplate + "_impl->color.rgb", Frame::BUFFER_COLOR) &&
            writeImage( filenameTemplate + "_impl->depth.rgb", Frame::BUFFER_DEPTH ));
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
    const Memory& memory = _impl->getMemory( buffer );

    const PixelViewport& pvp = memory.pvp;
    const size_t nPixels = pvp.w * pvp.h;

    if( nPixels == 0 || memory.state != Memory::VALID )
        return false;

    std::ofstream image( filename.c_str(), std::ios::out | std::ios::binary );
    if( !image.is_open( ))
    {
        LBERROR << "Can't open " << filename << " for writing" << std::endl;
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
            LBERROR << "Unknown image pixel data type" << std::endl;
            return false;
    }

    // Swap red & blue where needed
    bool swapRB = false;
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
            swapRB = true;
    }

    if( header.depth == 1 ) // depth
    {
        LBASSERT( (header.bytesPerChannel % 4) == 0 );
        header.depth = 4;
        header.bytesPerChannel /= 4;
    }
    LBASSERT( header.bytesPerChannel > 0 );
    if( header.bytesPerChannel > 2 )
        LBWARN << static_cast< int >( header.bytesPerChannel )
               << " bytes per channel not supported by RGB spec" << std::endl;

    const uint8_t bpc = header.bytesPerChannel;
    const uint16_t nChannels = header.depth;

    strncpy( header.filename, filename.c_str(), 80 );

    header.convert();
    image.write( reinterpret_cast<const char *>( &header ), sizeof( header ));
    header.convert();

    // Each channel is saved separately
    const size_t depth  = nChannels * bpc;
    const size_t nBytes = nPixels * depth;
    const char* data = reinterpret_cast<const char*>( getPixelPointer( buffer));

    if( nChannels == 3 || nChannels == 4 )
    {
        // channel one is R or B
        if ( swapRB )
            for( size_t j = 0 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
        else
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );

        // channel two is G
        for( size_t j = 1 * bpc; j < nBytes; j += depth )
            image.write( &data[j], bpc );

        // channel three is B or G
        if ( swapRB )
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
        else
            for( size_t j = 0; j < nBytes; j += depth )
                image.write( &data[j], bpc );

        // channel four is Alpha
        if( nChannels == 4 )
            for( size_t j = 3 * bpc; j < nBytes; j += depth )
                image.write( &data[j], bpc );
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
    lunchbox::MemoryMap image;
    const uint8_t* addr = static_cast< const uint8_t* >( image.map( filename ));

    if( !addr )
    {
        LBINFO << "Can't open " << filename << " for reading" << std::endl;
        return false;
    }

    const size_t size = image.getSize();
    if( size < sizeof( RGBHeader ))
    {
        LBWARN << "Image " << filename << " too small" << std::endl;
        return false;
    }

    RGBHeader header;
    memcpy( &header, addr, sizeof( header ));
    addr += sizeof( header );

    header.convert();

    if( header.magic != 474)
    {
        LBERROR << "Bad magic number " << filename << std::endl;
        return false;
    }
    if( header.width == 0 || header.height == 0 )
    {
        LBERROR << "Zero-sized image " << filename << std::endl;
        return false;
    }
    if( header.compression != 0)
    {
        LBERROR << "Unsupported compression " << filename << std::endl;
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
        LBERROR << "Unsupported image type " << filename << std::endl;
        return false;
    }

    if(( header.bytesPerChannel != 1 || nChannels == 1 ) &&
         header.maxValue != 255 )
    {
        LBERROR << "Unsupported value range " << header.maxValue << std::endl;
        return false;
    }

    const uint8_t bpc     = header.bytesPerChannel;
    const size_t  depth   = nChannels * bpc;
    const size_t  nPixels = header.width * header.height;
    const size_t  nComponents = nPixels * nChannels;
    const size_t  nBytes  = nComponents * bpc;

    if( size < sizeof( RGBHeader ) + nBytes )
    {
        LBERROR << "Image " << filename << " too small" << std::endl;
        return false;
    }
    LBASSERT( size == sizeof( RGBHeader ) + nBytes );

    switch( buffer )
    {
        case Frame::BUFFER_DEPTH:
            if( header.bytesPerChannel != 1 )
            {
                LBERROR << "Unsupported channel depth "
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
                        LBASSERT( nChannels==4 );
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
                            LBASSERT( nChannels == 3 );
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
                        LBASSERT( nChannels == 3 );
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
                        LBASSERT( nChannels == 3 );
                        _setExternalFormat( Frame::BUFFER_COLOR,
                                            EQ_COMPRESSOR_DATATYPE_RGBA32F,
                                            nChannels *4, false );
                    }
                    setInternalFormat( Frame::BUFFER_COLOR,
                                       EQ_COMPRESSOR_DATATYPE_RGBA32F );
                    break;

                default:
                    LBERROR << "Unsupported channel depth "
                            << static_cast< int >( header.bytesPerChannel )
                            << std::endl;
                    return false;
            }
            break;

        default:
            LBUNREACHABLE;
    }
    Memory& memory = _impl->getMemory( buffer );
    const PixelViewport pvp( 0, 0, header.width, header.height );
    if( pvp != _impl->pvp )
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
    LBASSERTINFO( nBytes <= getPixelDataSize( buffer ),
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

uint32_t Image::getExternalFormat( const Frame::Buffer buffer ) const
{
    return _impl->getMemory( buffer ).externalFormat;
}

uint32_t Image::getPixelSize( const Frame::Buffer buffer ) const
{
    return _impl->getMemory( buffer ).pixelSize;
}

void Image::setStorageType( const Frame::Type type )
{
    _impl->type = type;
}

Frame::Type Image::getStorageType() const
{
    return _impl->type;
}

const PixelViewport& Image::getPixelViewport() const
{
    return _impl->pvp;
}

void Image::setZoom( const Zoom& zoom )
{
    _impl->zoom = zoom;
}

const Zoom& Image::getZoom() const
{
    return _impl->zoom;
}

bool Image::hasPixelData( const Frame::Buffer buffer ) const
{
    return _impl->getMemory( buffer ).state == Memory::VALID;
}

bool Image::hasAsyncReadback( const Frame::Buffer buffer ) const
{
    return _impl->getMemory(buffer).state == Memory::DOWNLOAD;
}

bool Image::hasAsyncReadback() const
{
    return hasAsyncReadback( Frame::BUFFER_COLOR ) ||
           hasAsyncReadback( Frame::BUFFER_DEPTH );
}

bool Image::getAlphaUsage() const
{
    return !_impl->ignoreAlpha;
}

void Image::setOffset( int32_t x, int32_t y )
{
    _impl->pvp.x = x;
    _impl->pvp.y = y;
}

}
