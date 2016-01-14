
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Enrique G. Paredes <egparedes@ifi.uzh.ch>
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

#include "gl.h"
#include "half.h"
#include "log.h"
#include "pixelData.h"
#include "windowSystem.h"

#include <eq/util/frameBufferObject.h>
#include <eq/util/objectManager.h>
#include <eq/fabric/renderContext.h>

#include <co/global.h>

#include <lunchbox/buffer.h>
#include <lunchbox/memoryMap.h>
#include <lunchbox/omp.h>
#include <pression/compressor.h>
#include <pression/decompressor.h>
#include <pression/downloader.h>
#include <pression/pluginRegistry.h>
#include <pression/uploader.h>

#include <boost/filesystem.hpp>
#include <fstream>

#ifdef _WIN32
#  include <malloc.h>
#else
#  include <alloca.h>
#endif

#include "transferFinder.h"

#ifdef EQUALIZER_USE_OPENSCENEGRAPH
#  include <osgDB/WriteFile>
#endif

namespace eq
{
namespace
{
/** @internal Raw image data. */
struct Memory : public PixelData
{
public:
    Memory()
        : state( INVALID )
        , hasAlpha( true )
    {}

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
     * manage an internal buffer to copy the data. Otherwise the downloader
     * allocates the memory. */
    lunchbox::Bufferb localBuffer;

    bool hasAlpha; //!< The uncompressed pixels contain alpha
};

enum ActivePlugin
{
    PLUGIN_FULL,
    PLUGIN_LOSSY,
    PLUGIN_ALL
};

/** @internal The individual parameters for a buffer. */
struct Attachment
{
    ActivePlugin active;
    pression::Compressor compressor[ PLUGIN_ALL ];
    pression::Decompressor decompressor[ PLUGIN_ALL ];
    pression::Downloader downloader[ PLUGIN_ALL ];

    float quality; //!< the minimum quality

    /** The texture name for this image component (texture images). */
    util::Texture texture;

    /** Current pixel data (memory images). */
    Memory memory;

    Zoom zoom; //!< zoom factor of pending readback

    Attachment()
        : active( PLUGIN_FULL )
        , quality( 1.f )
        , texture( GL_TEXTURE_RECTANGLE_ARB )
        {}

    ~Attachment()
    {
        LBASSERT( !compressor[ PLUGIN_FULL ].isGood( ));
        LBASSERT( !compressor[ PLUGIN_LOSSY ].isGood( ));
        LBASSERT( !decompressor[ PLUGIN_FULL ].isGood( ));
        LBASSERT( !decompressor[ PLUGIN_LOSSY ].isGood( ));
        LBASSERT( !downloader[ PLUGIN_FULL ].isGood( ));
        LBASSERT( !downloader[ PLUGIN_LOSSY ].isGood( ));
    }

    void flush()
    {
        memory.flush();
        texture.flush();
        resetPlugins();
    }

    void resetPlugins()
    {
        compressor[ PLUGIN_FULL ].clear();
        compressor[ PLUGIN_LOSSY ].clear();
        decompressor[ PLUGIN_FULL ].clear();
        decompressor[ PLUGIN_LOSSY ].clear();
        downloader[ PLUGIN_FULL ].clear();
        downloader[ PLUGIN_LOSSY ].clear();
    }
};
}

namespace detail
{
class Image
{
public:
    Image()
        : type( eq::Frame::TYPE_MEMORY )
        , ignoreAlpha( false )
        , hasPremultipliedAlpha( false )
    {}

    /** The rectangle of the current pixel data. */
    PixelViewport pvp;

    /** The render context producing the image. */
    RenderContext context;

    /** Zoom factor used for compositing. */
    Zoom zoom;

    /** The storage type for the pixel data. */
    eq::Frame::Type type;

    Attachment color;
    Attachment depth;

    /** Alpha channel significance. */
    bool ignoreAlpha;

    bool hasPremultipliedAlpha;

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

    EqCompressorInfos findTransferers( const eq::Frame::Buffer buffer,
                                       const GLEWContext* gl ) const
    {
        const Attachment& attachment = getAttachment( buffer );
        const Memory& memory = attachment.memory;
        TransferFinder finder( memory.internalFormat, memory.externalFormat, 0,
                               attachment.quality, ignoreAlpha, gl );
        co::Global::getPluginRegistry().accept( finder );
        return finder.result;
    }
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
    _impl->hasPremultipliedAlpha = false;
    setPixelViewport( PixelViewport( ));
    setContext( RenderContext( ));
}

void Image::flush()
{
    _impl->color.flush();
    _impl->depth.flush();
}

void Image::resetPlugins()
{
    _impl->color.resetPlugins();
    _impl->depth.resetPlugins();
}

void Image::deleteGLObjects( util::ObjectManager& om )
{
    const char* key = reinterpret_cast< const char* >( this );
    for( size_t i=0; i < 4; ++i )
    {
        om.deleteEqUploader( key + i );
        om.deleteEqTexture( key + i );
    }
}

const void* Image::_getBufferKey( const Frame::Buffer buffer ) const
{
    switch( buffer )
    {
        // Check also deleteGLObjects!
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
        // Check also deleteGLObjects!
        case Frame::BUFFER_COLOR:
            if( attachment.quality == 1.0f )
                return ( reinterpret_cast< const char* >( this ) + 0 );
            return ( reinterpret_cast< const char* >( this ) + 1 );
        case Frame::BUFFER_DEPTH:
            if( attachment.quality == 1.0f )
                return ( reinterpret_cast< const char* >( this ) + 2 );
            return ( reinterpret_cast< const char* >( this ) + 3 );
        default:
            LBUNIMPLEMENTED;
            return ( reinterpret_cast< const char* >( this ) + 0 );
    }
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

namespace
{
class CompressorFinder : public pression::ConstPluginVisitor
{
public:
    explicit CompressorFinder( const uint32_t token ) : token_( token ) {}

    virtual fabric::VisitorResult visit( const pression::Plugin&,
                                         const EqCompressorInfo& info )
    {
        if( info.capabilities & EQ_COMPRESSOR_TRANSFER )
            return fabric::TRAVERSE_CONTINUE;

        if( info.tokenType == token_ )
            result.push_back( info.name );
        return fabric::TRAVERSE_CONTINUE;
    }

    std::vector< uint32_t > result;

private:
    const uint32_t token_;
};
}

std::vector< uint32_t > Image::findCompressors( const Frame::Buffer buffer )
    const
{
    const pression::PluginRegistry& registry = co::Global::getPluginRegistry();
    CompressorFinder finder( getExternalFormat( buffer ));
    registry.accept( finder );

    LBLOG( LOG_PLUGIN )
        << "Found " << finder.result.size() << " compressors for token type 0x"
        << std::hex << getExternalFormat( buffer ) << std::dec << std::endl;
    return finder.result;
}

std::vector< uint32_t > Image::findTransferers( const Frame::Buffer buffer,
                                                const GLEWContext* gl ) const
{
    std::vector< uint32_t > result;
    const EqCompressorInfos& infos = _impl->findTransferers( buffer, gl );
    for( EqCompressorInfosCIter i = infos.begin(); i != infos.end(); ++i )
        result.push_back( i->name );
    return result;
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
    _impl->color.memory.compressedData = pression::CompressorResult();
    _impl->depth.memory.compressedData = pression::CompressorResult();
}

void Image::setQuality( const Frame::Buffer buffer, const float quality )
{
    Attachment& attachment = _impl->getAttachment( buffer );
    if( attachment.quality == quality )
        return;

    attachment.quality = quality;
    if( quality >= 1.f )
        attachment.active = PLUGIN_FULL;
    else
    {
        attachment.active = PLUGIN_LOSSY;
        attachment.compressor[ PLUGIN_LOSSY ].clear();
        attachment.decompressor[ PLUGIN_LOSSY ].clear();
        attachment.downloader[ PLUGIN_LOSSY ].clear();
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

bool Image::upload( const Frame::Buffer buffer, util::Texture* texture,
                    const Vector2i& position, util::ObjectManager& om ) const
{
    // freed by deleteGLObjects, e.g., called from Pipe::flushFrames()
    pression::Uploader* uploader = om.obtainEqUploader(
                                        _getCompressorKey( buffer ));
    const PixelData& pixelData = getPixelData( buffer );
    const uint32_t externalFormat = pixelData.externalFormat;
    const uint32_t internalFormat = pixelData.internalFormat;
    const uint64_t flags = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                           ( texture ? texture->getCompressorTarget() :
                                       EQ_COMPRESSOR_USE_FRAMEBUFFER );
    const GLEWContext* const gl = om.glewGetContext();

    if( !uploader->supports( externalFormat, internalFormat, flags, gl ))
        uploader->setup( co::Global::getPluginRegistry(), externalFormat,
                         internalFormat, flags, gl );

    if( !uploader->isGood( gl ))
    {
        LBWARN << "No upload plugin for " << std::hex << externalFormat
               << " -> " << internalFormat << std::dec << " upload" <<std::endl;
        return false;
    }

    PixelViewport pvp = getPixelViewport();
    pvp.x = position.x() + pvp.x;
    pvp.y = position.y() + pvp.y;
    if( texture )
        texture->init( internalFormat, _impl->pvp.w, _impl->pvp.h );

    uint64_t inDims[4], outDims[4];
    pixelData.pvp.convertToPlugin( inDims );
    pvp.convertToPlugin( outDims );
    uploader->upload( pixelData.pixels, inDims, flags, outDims,
                      texture ? texture->getName() : 0, gl );
    return true;
}

//---------------------------------------------------------------------------
// asynchronous readback
//---------------------------------------------------------------------------
// TODO: 2.0 API: rename to readback and return Future
bool Image::startReadback( const uint32_t buffers, const PixelViewport& pvp,
                           const RenderContext& context, const Zoom& zoom,
                           util::ObjectManager& glObjects )
{
    LBLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers
                          << std::endl;

    _impl->pvp = pvp;
    _impl->context = context;
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
                            util::ObjectManager& glObjects )
{
    Attachment& attachment = _impl->getAttachment( buffer );
    attachment.memory.compressedData = pression::CompressorResult();

    if( _impl->type == Frame::TYPE_TEXTURE )
    {
        LBASSERTINFO( zoom == Zoom::NONE, "Texture readback zoom not " <<
                      "implemented, zoom happens during compositing" );
        util::Texture& texture = attachment.texture;
        texture.setGLEWContext( glObjects.glewGetContext( ));
        texture.copyFromFrameBuffer( getInternalFormat( buffer ), _impl->pvp );
        texture.setGLEWContext( 0 );
        return false;
    }

    attachment.zoom = zoom;
    if( zoom == Zoom::NONE ) // normal framebuffer readback
        return startReadback( buffer, 0, glObjects.glewGetContext( ));

    // else copy to texture, draw zoomed quad into FBO, (read FBO texture)
    return _readbackZoom( buffer, glObjects );
}

bool Image::startReadback( const Frame::Buffer buffer,
                           const util::Texture* texture, const GLEWContext* gl )
{
    Attachment& attachment = _impl->getAttachment( buffer );
    pression::Downloader& downloader = attachment.downloader[attachment.active];
    Memory& memory = attachment.memory;
    const uint32_t inputToken = memory.internalFormat;

    uint32_t flags = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                     ( texture ? texture->getCompressorTarget() :
                                 EQ_COMPRESSOR_USE_FRAMEBUFFER );
    const bool noAlpha = _impl->ignoreAlpha && buffer == Frame::BUFFER_COLOR;

    if( !downloader.supports( inputToken, noAlpha, flags ))
        downloader.setup( co::Global::getPluginRegistry(), inputToken,
                          attachment.quality, noAlpha, flags, gl );

    if( !downloader.isGood( ))
    {
        LBWARN << "Download plugin initialization failed using input 0x"
               << std::hex << inputToken << std::dec << std::endl;
        return false;
    }

    // get the pixel type produced by the downloader
    const EqCompressorInfo& info = downloader.getInfo();
    const bool alpha = (info.capabilities & EQ_COMPRESSOR_IGNORE_ALPHA) == 0;
    _setExternalFormat( buffer, info.outputTokenType, info.outputTokenSize,
                        alpha );
    attachment.memory.state = Memory::DOWNLOAD;

    if( !memory.hasAlpha )
        flags |= EQ_COMPRESSOR_IGNORE_ALPHA;

    uint64_t outDims[4] = {0};
    if( texture )
    {
        LBASSERT( texture->isValid( ));
        const uint64_t inDims[4] = { 0ull, uint64_t( texture->getWidth( )),
                                     0ull, uint64_t( texture->getHeight( )) };
        if( downloader.start( &memory.pixels, inDims, flags, outDims,
                              texture->getName(), gl ))
        {
            return true;
        }
    }
    else
    {
        uint64_t inDims[4];
        _impl->pvp.convertToPlugin( inDims );
        if( downloader.start( &memory.pixels, inDims, flags, outDims, 0, gl ))
            return true;
    }

    memory.pvp.convertFromPlugin( outDims );
    attachment.memory.state = Memory::VALID;
    return false;
}

void Image::finishReadback( const GLEWContext* context )
{
    LBASSERT( context );
    LBLOG( LOG_ASSEMBLY ) << "finishReadback" << std::endl;

    _finishReadback( Frame::BUFFER_COLOR, context );
    _finishReadback( Frame::BUFFER_DEPTH, context );

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

void Image::_finishReadback( const Frame::Buffer buffer,
                             const GLEWContext* context )
{
    if( _impl->type == Frame::TYPE_TEXTURE )
        return;

    Attachment& attachment = _impl->getAttachment( buffer );
    Memory& memory = attachment.memory;
    if( memory.state != Memory::DOWNLOAD )
        return;

    pression::Downloader& downloader = attachment.downloader[attachment.active];
    const uint32_t inputToken = memory.internalFormat;
    const bool alpha = _impl->ignoreAlpha && buffer == Frame::BUFFER_COLOR;
    uint32_t flags = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
        ( attachment.zoom == Zoom::NONE ? EQ_COMPRESSOR_USE_FRAMEBUFFER :
                                          EQ_COMPRESSOR_USE_TEXTURE_RECT );

    if( !downloader.supports( inputToken, alpha, flags ))
    {
        LBWARN << "Download plugin initialization failed" << std::endl;
        attachment.memory.state = Memory::INVALID;
        return;
    }

    if( memory.hasAlpha && buffer == Frame::BUFFER_COLOR )
        _impl->hasPremultipliedAlpha = true;

    flags |= ( memory.hasAlpha ? 0 : EQ_COMPRESSOR_IGNORE_ALPHA );

    uint64_t outDims[4] = {0};
    uint64_t inDims[4];
    PixelViewport pvp = _impl->pvp;
    if( attachment.zoom != Zoom::NONE )
    {
        pvp.apply( attachment.zoom );
        pvp.x = 0;
        pvp.y = 0;
    }
    _impl->pvp.convertToPlugin( inDims );

    downloader.finish( &memory.pixels, inDims, flags, outDims, context );
    memory.pvp.convertFromPlugin( outDims );
    memory.state = Memory::VALID;
}

bool Image::_readbackZoom( const Frame::Buffer buffer, util::ObjectManager& om )
{
    LBASSERT( om.supportsEqTexture( ));
    LBASSERT( om.supportsEqFrameBufferObject( ));

    const Attachment& attachment = _impl->getAttachment( buffer );
    PixelViewport pvp = _impl->pvp;
    pvp.apply( attachment.zoom );
    if( !pvp.hasArea( ))
        return false;

    // copy frame buffer to texture
    const uint32_t inputToken = attachment.memory.internalFormat;
    const void* bufferKey = _getBufferKey( buffer );
    util::Texture* texture = om.obtainEqTexture( bufferKey,
                                                 GL_TEXTURE_RECTANGLE_ARB );
    texture->copyFromFrameBuffer( inputToken, _impl->pvp );

    // draw zoomed quad into FBO
    //  uses the same FBO for color and depth, with masking.
    const void* fboKey = _getBufferKey( Frame::BUFFER_COLOR );
    util::FrameBufferObject* fbo = om.getEqFrameBufferObject( fboKey );

    if( fbo )
    {
        LBCHECK( fbo->resize( pvp.w, pvp.h ));
    }
    else
    {
        fbo = om.newEqFrameBufferObject( fboKey );
        LBCHECK( fbo->init( pvp.w, pvp.h, inputToken, 24, 0 ));
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
    LBASSERT( zoomedTexture->isValid( ));
    LBLOG( LOG_ASSEMBLY ) << "Scale " << _impl->pvp << " -> " << pvp << std::endl;

    // BUG TODO: this is a bug in case of color and depth buffers read-back, as
    // _impl->pvp will be incorrect for the depth buffer!
    //
    // This should be done separately for color an depth buffers!
    _impl->pvp = pvp;

    LBLOG( LOG_ASSEMBLY ) << "Read texture " << getPixelDataSize( buffer )
                          << std::endl;
    return startReadback( buffer, zoomedTexture, om.glewGetContext( ));
}

void Image::setPixelViewport( const PixelViewport& pvp )
{
    _impl->pvp = pvp;
    _impl->color.memory.state = Memory::INVALID;
    _impl->depth.memory.state = Memory::INVALID;
    _impl->color.memory.compressedData = pression::CompressorResult();
    _impl->depth.memory.compressedData = pression::CompressorResult();
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
        lunchbox::setZero( data, size );
#pragma omp parallel for
        for( ssize_t i = 3; i < size; i+=4 )
            data[i] = 255;
#endif
        break;
      }
      default:
        LBWARN << "Unknown external format " << memory.externalFormat
               << ", initializing to 0" << std::endl;
        lunchbox::setZero( memory.pixels, size );
        break;
    }
}

void Image::validatePixelData( const Frame::Buffer buffer )
{
    Memory& memory = _impl->getAttachment( buffer ).memory;
    memory.useLocalBuffer();
    memory.state = Memory::VALID;
    memory.compressedData = pression::CompressorResult();
}

void Image::setPixelData( const Frame::Buffer buffer, const PixelData& pixels )
{
    Memory& memory = _impl->getMemory( buffer );
    memory.externalFormat = pixels.externalFormat;
    memory.internalFormat = pixels.internalFormat;
    memory.pixelSize = pixels.pixelSize;
    memory.pvp       = pixels.pvp;
    memory.state     = Memory::INVALID;
    memory.compressedData = pression::CompressorResult();
    memory.hasAlpha = false;

    const EqCompressorInfos& transferrers = _impl->findTransferers( buffer,
                                                           0 /*GLEW context*/ );
    if( transferrers.empty( ))
        LBWARN << "No upload engines found for given pixel data" << std::endl;
    else
    {
        memory.hasAlpha =
            transferrers.front().capabilities & EQ_COMPRESSOR_IGNORE_ALPHA;
#ifndef NDEBUG
        for( EqCompressorInfosCIter i = transferrers.begin();
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

    if( pixels.compressedData.compressor <= EQ_COMPRESSOR_NONE )
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

    LBASSERT( !pixels.compressedData.chunks.empty( ));
    LBASSERT( pixels.compressedData.compressor != EQ_COMPRESSOR_AUTO );

    Attachment& attachment = _impl->getAttachment( buffer );
    if( !attachment.decompressor->setup( co::Global::getPluginRegistry(),
                                         pixels.compressedData.compressor ))
    {
        LBASSERTINFO( false,
                      "Can't allocate decompressor " <<
                      pixels.compressedData.compressor <<
                      ", mismatched compression plugin installation?" );
        return;
    }

    const EqCompressorInfo& info = attachment.decompressor->getInfo();
    LBASSERTINFO( info.name == pixels.compressedData.compressor, info );

    if( memory.externalFormat != info.outputTokenType )
    {
        // decompressor output differs from compressor input
        memory.externalFormat = info.outputTokenType;
        memory.pixelSize = info.outputTokenSize;
    }
    validatePixelData( buffer ); // alloc memory for pixels

    uint64_t outDims[4];
    memory.pvp.convertToPlugin( outDims );

    attachment.decompressor->decompress( pixels.compressedData, memory.pixels,
                                         outDims, pixels.compressorFlags );
}

/** Find and activate a compression engine */
bool Image::allocCompressor( const Frame::Buffer buffer, const uint32_t name )
{
    Attachment& attachment = _impl->getAttachment( buffer );
    pression::Compressor& compressor = attachment.compressor[attachment.active];
    if( name <= EQ_COMPRESSOR_NONE )
    {
        attachment.memory.compressedData = pression::CompressorResult();
        compressor.clear();
        return true;
    }

    if( compressor.uses( name ))
        return true;

    attachment.memory.compressedData = pression::CompressorResult();
    compressor.setup( co::Global::getPluginRegistry(), name );
    LBLOG( LOG_PLUGIN ) << "Instantiated compressor of type 0x" << std::hex
                        << name << std::dec << std::endl;
    return compressor.isGood();
}

/** Find and activate a compression engine */
bool Image::allocDownloader( const Frame::Buffer buffer, const uint32_t name,
                             const GLEWContext* gl )
{
    LBASSERT( name > EQ_COMPRESSOR_NONE )
    LBASSERT( gl );

    Attachment& attachment = _impl->getAttachment( buffer );
    pression::Downloader& downloader = attachment.downloader[attachment.active];

    if( name <= EQ_COMPRESSOR_NONE )
    {
        downloader.clear();
        _setExternalFormat( buffer, EQ_COMPRESSOR_DATATYPE_NONE, 0, true );
        return false;
    }


    if( downloader.uses( name ))
        return true;

    if( !downloader.setup( co::Global::getPluginRegistry(), name, gl ))
        return false;

    const EqCompressorInfo& info = downloader.getInfo();
    attachment.memory.internalFormat = info.tokenType;
    _setExternalFormat( buffer, info.outputTokenType, info.outputTokenSize,
                        !(info.capabilities & EQ_COMPRESSOR_IGNORE_ALPHA) );
    return true;
}

uint32_t Image::getDownloaderName( const Frame::Buffer buffer ) const
{
    const Attachment& attachment = _impl->getAttachment( buffer );
    const pression::Downloader& downloader =
        attachment.downloader[attachment.active];
    if( downloader.isGood( ))
        return downloader.getInfo().name;
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
    if( memory.compressedData.isCompressed() ||
        memory.compressorName == EQ_COMPRESSOR_NONE )
    {
        LBASSERT( memory.compressorName != EQ_COMPRESSOR_AUTO );
        return memory;
    }

    pression::Compressor& compressor = attachment.compressor[attachment.active];

    if( !compressor.isGood() ||
        compressor.getInfo().tokenType != getExternalFormat( buffer ) ||
        memory.compressorName == EQ_COMPRESSOR_AUTO )
    {
        if( memory.compressorName == EQ_COMPRESSOR_AUTO )
        {
            const uint32_t tokenType = getExternalFormat( buffer );
            const float downloadQuality =
                attachment.downloader[ attachment.active ].getInfo().quality;
            const float quality = attachment.quality / downloadQuality;

            compressor.setup( co::Global::getPluginRegistry(), tokenType,
                               quality, _impl->ignoreAlpha );
        }
        else
            compressor.setup( co::Global::getPluginRegistry(),
                              memory.compressorName );

        if( !compressor.isGood( ))
        {
            LBWARN << "No compressor found for token type 0x" << std::hex
                   << getExternalFormat( buffer ) << std::dec << std::endl;
            compressor.clear();
        }
    }

    memory.compressedData.compressor = compressor.getInfo().name;
    LBASSERT( memory.compressedData.compressor != EQ_COMPRESSOR_AUTO );
    LBASSERT( memory.compressedData.compressor != EQ_COMPRESSOR_INVALID );
    if( memory.compressedData.compressor == EQ_COMPRESSOR_NONE )
        return memory;

    memory.compressorFlags = EQ_COMPRESSOR_DATA_2D;
    if( _impl->ignoreAlpha && memory.hasAlpha )
    {
        LBASSERT( buffer == Frame::BUFFER_COLOR );
        memory.compressorFlags |= EQ_COMPRESSOR_IGNORE_ALPHA;
    }

    uint64_t inDims[4];
    memory.pvp.convertToPlugin( inDims );
    compressor.compress( memory.pixels, inDims, memory.compressorFlags );
    memory.compressedData = compressor.getResult();
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

namespace
{
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

void put32f( std::ostream& os, const char* ptr )
{
    // cppcheck-suppress invalidPointerCast
    const float& value = *reinterpret_cast< const float* >( ptr );
    const uint8_t byte = uint8_t( value * 255.f );
    os.write( (const char*)&byte, 1 );
}
void put16f( std::ostream& os, const char* ptr )
{
    const uint16_t& value = *reinterpret_cast< const uint16_t* >(ptr);
    const float f = half_to_float( value );
    put32f( os, (const char*)&f );
}
}

bool Image::writeImage( const std::string& filename,
                        const Frame::Buffer buffer ) const
{
    const Memory& memory = _impl->getMemory( buffer );

    const PixelViewport& pvp = memory.pvp;
    const size_t nPixels = pvp.w * pvp.h;

    if( nPixels == 0 || memory.state != Memory::VALID )
        return false;

    const unsigned char* data =
         reinterpret_cast<const unsigned char*>( getPixelPointer( buffer ));

    unsigned char* convertedData = nullptr;

    // glReadPixels with alpha has ARGB premultiplied format: post-divide alpha
    if( _impl->hasPremultipliedAlpha &&
        getExternalFormat( buffer ) == EQ_COMPRESSOR_DATATYPE_BGRA )
    {
        convertedData = new unsigned char[nPixels*4];

        const uint32_t* bgraData = reinterpret_cast< const uint32_t* >( data );
        uint32_t* bgraConverted = reinterpret_cast< uint32_t* >( convertedData);
        for( size_t i = 0; i < nPixels; ++i, ++bgraConverted, ++bgraData )
        {
            *bgraConverted = *bgraData;
            uint32_t& pixel = *bgraConverted;
            const uint32_t alpha = pixel >> 24;
            if( alpha != 0 )
            {
                const uint32_t red = (pixel >> 16) & 0xff;
                const uint32_t green = (pixel >> 8) & 0xff;
                const uint32_t blue = pixel & 0xff;
                *bgraConverted = (( alpha << 24 ) |
                                 (((255 * red) / alpha ) << 16 ) |
                                 (((255 * green) / alpha )  << 8 ) |
                                 ((255 * blue) / alpha ));
            }
        }
    }

    const bool retVal = _writeImage( filename, buffer,
                                     convertedData ? convertedData : data );
    delete [] convertedData;
    return retVal;
}

bool Image::_writeImage( const std::string& filename,
                         const Frame::Buffer buffer,
                         const unsigned char* data_ ) const
{
    const Memory& memory = _impl->getMemory( buffer );
    const PixelViewport& pvp = memory.pvp;
    const size_t nPixels = pvp.w * pvp.h;

    RGBHeader header;
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

    if( header.depth == 1 ) // depth
    {
        LBASSERT( (header.bytesPerChannel % 4) == 0 );
        header.depth = 4;
        header.bytesPerChannel /= 4;
    }
    LBASSERT( header.bytesPerChannel > 0 );

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

    const uint8_t bpc = header.bytesPerChannel;
    const uint16_t nChannels = header.depth;
    const size_t depth = nChannels * bpc;

    const boost::filesystem::path path( filename );
#ifdef EQUALIZER_USE_OPENSCENEGRAPH
    if( path.extension() != ".rgb" )
    {
        osg::ref_ptr<osg::Image> osgImage = new osg::Image();
        osgImage->setImage( pvp.w, pvp.h, depth, getExternalFormat( buffer ),
                            swapRB ? GL_RGBA : GL_BGRA, GL_UNSIGNED_BYTE,
                            const_cast< unsigned char* >( data_ ),
                            osg::Image::NO_DELETE );
        return osgDB::writeImageFile( *osgImage, filename );
    }
#endif

    std::ofstream image( filename.c_str(), std::ios::out | std::ios::binary );
    if( !image.is_open( ))
    {
        LBERROR << "Can't open " << filename << " for writing" << std::endl;
        return false;
    }

    const size_t nBytes = nPixels * depth;
    if( header.bytesPerChannel > 2 )
        LBWARN << static_cast< int >( header.bytesPerChannel )
               << " bytes per channel not supported by RGB spec" << std::endl;

    strncpy( header.filename, filename.c_str(), 80 );
    header.convert();
    image.write( reinterpret_cast<const char *>( &header ), sizeof( header ));
    header.convert();

    const char* data = reinterpret_cast< const char* >( data_ );

    // Each channel is saved separately
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

    if( header.bytesPerChannel == 1 )
        return true;
    // else also write 8bpp version

    const std::string smallFilename = path.parent_path().string() + "/s_" +
#if BOOST_FILESYSTEM_VERSION == 3
                                      path.filename().string();
#else
                                      path.filename();
#endif
    image.open( smallFilename.c_str(), std::ios::out | std::ios::binary );
    if( !image.is_open( ))
    {
        LBERROR << "Can't open " << smallFilename << " for writing" <<std::endl;
        return false;
    }

    header.bytesPerChannel = 1;
    header.maxValue = 255;
    header.convert();
    image.write( reinterpret_cast<const char *>( &header ), sizeof( header ));
    header.convert();

    LBASSERTINFO( bpc == 2 || bpc == 4, bpc );
    const bool twoBPC = bpc == 2;

    if( nChannels == 3 || nChannels == 4 )
    {
        // channel one is R or B
        if ( swapRB )
            for( size_t j = 0 * bpc; j < nBytes; j += depth )
                twoBPC ? put16f( image, &data[j] ) : put32f( image, &data[j] );
        else
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                twoBPC ? put16f( image, &data[j] ) : put32f( image, &data[j] );

        // channel two is G
        for( size_t j = 1 * bpc; j < nBytes; j += depth )
                twoBPC ? put16f( image, &data[j] ) : put32f( image, &data[j] );

        // channel three is B or G
        if ( swapRB )
            for( size_t j = 2 * bpc; j < nBytes; j += depth )
                twoBPC ? put16f( image, &data[j] ) : put32f( image, &data[j] );
        else
            for( size_t j = 0; j < nBytes; j += depth )
                twoBPC ? put16f( image, &data[j] ) : put32f( image, &data[j] );

         // channel four is Alpha
        if( nChannels == 4 )
            for( size_t j = 3 * bpc; j < nBytes; j += depth )
                twoBPC ? put16f( image, &data[j] ) : put32f( image, &data[j] );
    }
    else
    {
        for( size_t i = 0; i < nChannels; i += bpc )
           for( size_t j = i * bpc; j < nBytes; j += depth )
               twoBPC ? put16f( image, &data[j] ) : put32f( image, &data[j] );
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
        LBERROR << "Can't open " << filename << " for reading" << std::endl;
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

    const uint8_t bpc = header.bytesPerChannel;
    const size_t depth = nChannels * bpc;
    const size_t nPixels = header.width * header.height;
    const size_t nComponents = nPixels * nChannels;
    const size_t nBytes = nComponents * bpc;

    if( size < sizeof( RGBHeader ) + nBytes )
    {
        LBERROR << "Image " << filename << " too small" << std::endl;
        return false;
    }
    LBASSERTINFO( size == sizeof( RGBHeader ) + nBytes,
                  "delta " << size - sizeof( RGBHeader ) - nBytes );

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
            for( size_t j = i; j < nComponents; j += nChannels )
            {
                data[j] = *addr;
                ++addr;
            }
        break;

    case 2:
        for( size_t i = 0; i < nChannels; ++i )
            for( size_t j = i; j < nComponents; j += nChannels )
            {
                reinterpret_cast< uint16_t* >( data )[ j ] =
                    *reinterpret_cast< const uint16_t* >( addr );
                addr += bpc;
            }
        break;

    case 4:
        for( size_t i = 0; i < nChannels; ++i )
            for( size_t j = i; j < nComponents; j += nChannels )
            {
                reinterpret_cast< uint32_t* >( data )[ j ] =
                    *reinterpret_cast< const uint32_t* >( addr );
                addr += bpc;
            }
        break;

    default:
        for( size_t i = 0; i < depth; i += bpc )
            for( size_t j = i * bpc; j < nBytes; j += depth )
            {
                memcpy( &data[j], addr, bpc );
                addr += bpc;
            }
        break;
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

void Image::setContext( const RenderContext& context )
{
    _impl->context = context;
}

const RenderContext& Image::getContext() const
{
    return _impl->context;
}

bool Image::hasPixelData( const Frame::Buffer buffer ) const
{
    return _impl->getMemory( buffer ).state == Memory::VALID;
}

bool Image::hasAsyncReadback( const Frame::Buffer buffer ) const
{
    return _impl->getMemory( buffer ).state == Memory::DOWNLOAD;
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
