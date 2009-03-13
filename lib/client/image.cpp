
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "image.h"

#include "frame.h"
#include "frameBufferObject.h"
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

typedef Image::PixelData::Chunk Chunk;
}

size_t Image::PixelData::Chunk::headerSize = 16;

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
    _colorPixels.flush();
    _depthPixels.flush();
    _compressedColorPixels.flush();
    _compressedDepthPixels.flush();
    _colorTexture.flush();
    _depthTexture.flush();
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
    EQASSERT( _getPixels( buffer ).data.chunks.size() == 1 );

    return _getPixels( buffer ).data.chunks[0]->data;
}

uint8_t* Image::getPixelPointer( const Frame::Buffer buffer )
{
    EQASSERT( hasPixelData( buffer ));
    EQASSERT( _getPixels( buffer ).data.chunks.size() == 1 );

    return _getPixels( buffer ).data.chunks[0]->data;
}

const Image::PixelData& Image::getPixelData( const Frame::Buffer buffer ) const
{
    EQASSERT(hasPixelData(buffer));
    return _getPixels( buffer ).data;
}

void Image::startReadback( const uint32_t buffers, const PixelViewport& pvp,
                           const Zoom& zoom, Window::ObjectManager* glObjects )
{
    EQ_GL_ERROR( "before startReadback" );
    EQASSERT( glObjects );
    EQASSERTINFO( !_glObjects, "Another readback in progress?" );
    EQLOG( LOG_ASSEMBLY ) << "startReadback " << pvp << ", buffers " << buffers
                          << endl;

    _glObjects = glObjects;
    _pvp       = pvp;

    _colorPixels.state = Pixels::INVALID;
    _depthPixels.state = Pixels::INVALID;

    if( buffers & Frame::BUFFER_COLOR )
        _startReadback( Frame::BUFFER_COLOR, zoom );

    if( buffers & Frame::BUFFER_DEPTH )
        _startReadback( Frame::BUFFER_DEPTH, zoom );


    _pvp.apply( zoom );
    _pvp.x = 0;
    _pvp.y = 0;
    EQ_GL_ERROR( "after startReadback" );
}

void Image::syncReadback()
{
    EQ_GL_ERROR( "before syncReadback" );
    _syncReadback( Frame::BUFFER_COLOR );
    _syncReadback( Frame::BUFFER_DEPTH );
    _glObjects = 0;
    EQ_GL_ERROR( "after syncReadback" );
}

void Image::Pixels::resize( uint32_t size )
{
    EQASSERT( data.chunks.size() == 1 );

    if( maxSize >= size )
    {
        data.chunks[0]->size = size;
        return;
    }

    // round to next 8-byte alignment (compress uses 8-byte tokens)
    if( size%8 )
        size += 8 - (size%8);

    if( data.chunks[0] )
        free( data.chunks[0] );
    
    data.chunks[0] = reinterpret_cast< Chunk* >( malloc( size + 
                                                         Chunk::headerSize ));
    data.chunks[0]->size = size;
    maxSize = size;
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
                      getType( buffer ), pixels.data.chunks[0]->data );
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
            return _colorTexture;
            
        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _depthTexture;
    }
}   

Texture& Image::_getTexture( const Frame::Buffer buffer )
{
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            return _colorTexture;
            
        default:
            EQASSERTINFO( buffer == Frame::BUFFER_DEPTH, buffer );
            return _depthTexture;
    }


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

    PixelViewport      pvp = _pvp;
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

        glTexCoord2f( _pvp.w, 0.0f );
        glVertex3f( pvp.w, 0, 0.0f );

        glTexCoord2f( _pvp.w, _pvp.h );
        glVertex3f( pvp.w, pvp.h, 0.0f );

        glTexCoord2f( 0.0f, _pvp.h );
        glVertex3f( 0, pvp.h, 0.0f );
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

    EQ_GL_CALL( glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo ));
    const void* data = glMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY );
    EQ_GL_ERROR( "glMapBuffer" );
    EQASSERT( data );

    memcpy( pixels.data.chunks[0]->data, data, size );

    glUnmapBuffer( GL_PIXEL_PACK_BUFFER );
    glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

    pixels.state = Pixels::VALID;
}

void Image::_syncReadbackZoom( const Frame::Buffer buffer )
{
    Pixels& pixels = _getPixels( buffer );
    const size_t size = getPixelDataSize( buffer );
    pixels.resize( size );

    const void*  bufferKey = _getBufferKey( buffer );
    FrameBufferObject* fbo = _glObjects->getEqFrameBufferObject( bufferKey );
    EQASSERT( fbo != 0 );
    
    switch( buffer )
    {
        case Frame::BUFFER_COLOR:
            fbo->getColorTextures()[0]->download( pixels.data.chunks[0]->data,
                                                  getFormat( buffer ), 
                                                  getType( buffer ));
            break;

        default:
            EQUNIMPLEMENTED;
        case Frame::BUFFER_DEPTH:
            fbo->getDepthTexture().download( pixels.data.chunks[0]->data,
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
    _colorPixels.state = Pixels::INVALID;
    _depthPixels.state = Pixels::INVALID;
    _compressedColorPixels.valid = false;
    _compressedDepthPixels.valid = false;
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
#ifdef LEOPARD
        void*       data = pixels.data.chunks[0]->data;
        const float one  = 1.0f;
        memset_pattern4( data, &one, size );
#else
        EQASSERT( (size % 4) == 0 );
        const size_t nWords = (size >> 2);
        float*       data   = reinterpret_cast< float* >(
                                  pixels.data.chunks[0]->data );
        for( size_t i =0; i < nWords; ++i )
            data[i] = 1.0f;
#endif

    }
    else
    {
        if( getDepth( Frame::BUFFER_COLOR ) == 4 )
        {
            uint8_t* data = pixels.data.chunks[0]->data;
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
            bzero( pixels.data.chunks[0]->data, size );
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
    memcpy( pixels.data.chunks[0]->data, data, size );
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

    if( !pixels.compressed )
    {
        EQASSERT( pixels.chunks.size() == 1 );
        EQASSERT( size == pixels.chunks[0]->size );

        setPixelData( buffer, pixels.chunks[0]->data );
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
        uint8_t* out = outPixels.data.chunks[0]->data;
        for( ssize_t i = 0; i < nChunks; ++i )
        {
            outTable[i] = reinterpret_cast< uint64_t* >( out );

            const uint64_t* in  = 
                reinterpret_cast< const uint64_t* >( pixels.chunks[i]->data );
            const uint64_t nWords = in[0];
            out += nWords * sizeof( uint64_t );
        }
        EQASSERTINFO( size >= (uint32_t)(out-outPixels.data.chunks[0]->data-7 ),
                      "Pixel data size does not match expected image size: "
                      << size << " ? " 
                      << (uint32_t)( out - outPixels.data.chunks[0]->data ));
    }

    // decompress each block
    // On OS X the loop is sometimes slower when parallelized. Investigate this!
#pragma omp parallel for
    for( ssize_t i = 0; i < nChunks; ++i )
    {
        uint64_t* in  = reinterpret_cast< uint64_t* >( pixels.chunks[i]->data );
        uint64_t* out = outTable[i];

        uint32_t       outPos = 0;
        const uint64_t endPos = in[0];
        uint32_t       inPos  = 1;

        EQVERB << "In chunk " << i << " size " << pixels.chunks[i]->size 
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

    outPixels.state = Pixels::VALID;
}

void Image::CompressedPixels::flush()
{
    data.flush();
    chunkMaxSizes.clear();
    valid = false;
}

void Image::Pixels::flush()
{
    maxSize = 0;
    pboSize = 0;
    state = INVALID;
    data.flush();
    data.chunks.push_back( 0 );
}

void Image::PixelData::flush()
{
    while( !chunks.empty( ))
    {
        if( chunks.back( ))
            free( chunks.back( ));
        chunks.pop_back();
    }
    format = GL_FALSE;
    type   = GL_FALSE;
    compressed = false;
}

Image::PixelData::~PixelData()
{
    flush();
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
                  compressedPixels.chunkMaxSizes.size( ));

        if( compressedPixels.data.chunks.size() <= static_cast< size_t >( i ))
        {
            Chunk* chunk = reinterpret_cast< Chunk* >( 
                               malloc( maxChunkSize + Chunk::headerSize ));
            compressedPixels.data.chunks.push_back( chunk );
            compressedPixels.chunkMaxSizes.push_back( maxChunkSize );
        }
        else
        {
            if( compressedPixels.chunkMaxSizes[i] < maxChunkSize )
            {
                if( compressedPixels.data.chunks[i] )
                    free( compressedPixels.data.chunks[i] );

                compressedPixels.data.chunks[i] = reinterpret_cast< Chunk* >( 
                                    malloc( maxChunkSize + Chunk::headerSize ));
                compressedPixels.chunkMaxSizes[i] = maxChunkSize;
            }
        }
        compressedPixels.data.chunks[i]->size = 0;
    }


    const float width = static_cast< float >( nWords ) /
                        static_cast< float >( nChunks );

#pragma omp parallel for
    for ( ssize_t i = 0; i < nChunks; ++i )
    {
        const uint32_t startIndex = static_cast< uint32_t >( i * width );
        const uint32_t endIndex   = static_cast< uint32_t >( (i+1) * width );
        uint64_t*      out        = reinterpret_cast< uint64_t* >(
                                        compressedPixels.data.chunks[i]->data );

        compressedPixels.data.chunks[i]->size =
            _compressPixelData( &data[ startIndex ], endIndex-startIndex, out );

        EQVERB << "In chunk " << i << " size "
               << (endIndex-startIndex) * sizeof( uint64_t ) << " @ " 
               << (void*)&data[ startIndex ] << endl
               << "Out chunk " << i << " size "
               << compressedPixels.data.chunks[i]->size << " @ " << (void*)out
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
            if( depth > 4)
            {
                EQASSERT( (depth % 4) == 0 );
                stringstream << "_" << d / 4;
            }
            writeImage( filenameTemplate + stringstream.str() + ".rgb",
                        buffer, d );
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
                        const Frame::Buffer buffer, 
                        const uint32_t shift ) const
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

    const size_t depth   = getDepth( buffer );
    RGBHeader    header;

    header.width  = _pvp.w;
    header.height = _pvp.h;
    header.depth  = depth > 4 ? 4 : depth;
    strncpy( header.filename, filename.c_str(), 80 );

    header.convert();
    image.write( reinterpret_cast<const char *>( &header ), sizeof( header ));

    // Each channel is saved separately
    const size_t   nBytes = nPixels * depth;
    const uint8_t* data   = getPixelPointer( buffer ) + shift;

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
    if( header.compression != 0)
    {
        EQERROR << "Unsupported compression " << filename << endl;
        image.close();
        return false;
    }

    const size_t depth = header.depth;

    if( header.bytesPerChannel != 1 ||
        header.nDimensions != 3 ||
        header.minValue != 0 ||
        header.maxValue != 255 ||
        header.colorMode != 0 ||
        ( buffer == Frame::BUFFER_COLOR && depth != 3 && depth != 4 ) ||
        ( buffer == Frame::BUFFER_DEPTH && depth != 4 ))
    {
        EQERROR << "Unsupported image type " << filename << endl;
        image.close();
        return false;
    }

    switch( buffer )
    {
        case Frame::BUFFER_DEPTH:
            setFormat( Frame::BUFFER_DEPTH, GL_DEPTH_COMPONENT );
            setType(   Frame::BUFFER_DEPTH, GL_FLOAT );
            break;

        default:
            EQUNREACHABLE;
        case Frame::BUFFER_COLOR:
            setFormat( Frame::BUFFER_COLOR, (depth==4) ? GL_RGBA : GL_RGB );
            setType(   Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
            break;
    }

    const size_t     nPixels = header.width * header.height;
    const size_t     nBytes  = nPixels * depth;
    Pixels           pixels;

    pixels.resize( nBytes );
    uint8_t* data = pixels.data.chunks[0]->data;

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
