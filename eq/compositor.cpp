
/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include <lunchbox/perThread.h>

#include "channel.h"
#include "channelStatistics.h"
#include "client.h"
#include "compositor.h"
#include "config.h"
#include "exception.h"
#include "frameData.h"
#include "gl.h"
#include "image.h"
#include "log.h"
#include "pixelData.h"
#include "server.h"
#include "window.h"
#include "windowSystem.h"

#include <eq/util/accum.h>
#include <eq/util/objectManager.h>
#include <eq/util/shader.h>

#include <co/global.h>
#include <lunchbox/debug.h>
#include <lunchbox/monitor.h>
#include <lunchbox/os.h>
#include <pression/plugins/compressor.h>

using lunchbox::Monitor;

namespace eq
{

#define glewGetContext channel->glewGetContext

namespace
{
// use to address one shader and program per shared context set
static const char seed = 42;
static const char* shaderDBKey = &seed;
static const char* colorDBKey  = shaderDBKey + 1;
static const char* depthDBKey  = shaderDBKey + 2;

// Image used for CPU-based assembly
static lunchbox::PerThread< Image > _resultImage;

static bool _useCPUAssembly( const Frames& frames, Channel* channel,
                             const bool blendAlpha = false )
{
    // It doesn't make sense to use CPU-assembly for only one frame
    if( frames.size() < 2 )
        return false;

    // Test that at least two input frames have color and depth buffers or that
    // alpha-blended assembly is used with multiple RGBA buffers. We assume then
    // that we will have at least one image per frame so most likely it's worth
    // to wait for the images and to do a CPU-based assembly.
    // Also test early for unsupport decomposition modes
    const uint32_t desiredBuffers = blendAlpha ? Frame::BUFFER_COLOR :
                                    Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH;
    size_t nFrames = 0;
    for( Frames::const_iterator i = frames.begin(); i != frames.end(); ++i )
    {
        const Frame* frame = *i;
        if( frame->getPixel() != Pixel::ALL ||
            frame->getSubPixel() != SubPixel::ALL ||
            frame->getZoom() != Zoom::NONE ) // Not supported by CPU compositor
        {
            return false;
        }

        if( frame->getBuffers() == desiredBuffers )
            ++nFrames;
    }
    if( nFrames < 2 )
        return false;

    // Now wait for all images to be ready and test if our assumption was
    // correct, that there are enough images to make a CPU-based assembly
    // worthwhile and all other preconditions for our CPU-based assembly code
    // are true.
    size_t   nImages        = 0;
    uint32_t colorInternalFormat = 0;
    uint32_t colorExternalFormat = 0;
    uint32_t depthInternalFormat = 0;
    uint32_t depthExternalFormat = 0;
    const uint32_t timeout = channel->getConfig()->getTimeout();

    for( FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        const Frame* frame = *i;
        {
            ChannelStatistics event( Statistic::CHANNEL_FRAME_WAIT_READY,
                                     channel );
            frame->waitReady( timeout );
        }

        if( frame->getFrameData()->getZoom() != Zoom::NONE )
            return false;

        const Images& images = frame->getImages();
        for( Images::const_iterator j = images.begin();
             j != images.end(); ++j )
        {
            const Image* image = *j;

            const bool hasColor = image->hasPixelData( Frame::BUFFER_COLOR );
            const bool hasDepth = image->hasPixelData( Frame::BUFFER_DEPTH );

            if( // Not an alpha-blending compositing
                ( !blendAlpha || !hasColor || !image->hasAlpha( )) &&
                // and not a depth-sorting compositing
                ( !hasColor || !hasDepth ))
            {
                return false;
            }

            if( colorInternalFormat == 0 && colorExternalFormat == 0 )
            {
                colorInternalFormat =
                    image->getInternalFormat( Frame::BUFFER_COLOR );
                colorExternalFormat =
                    image->getExternalFormat( Frame::BUFFER_COLOR );

                switch( colorExternalFormat )
                {
                    case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
                    case EQ_COMPRESSOR_DATATYPE_BGR10_A2:
                        if( !hasDepth )
                            // blending of RGB10A2 not implemented
                            return false;
                        break;

                    case EQ_COMPRESSOR_DATATYPE_RGBA:
                    case EQ_COMPRESSOR_DATATYPE_BGRA:
                        break;

                    default:
                        return false;
                }

            }
            else if( colorInternalFormat !=
                     image->getInternalFormat( Frame::BUFFER_COLOR ) ||
                     colorExternalFormat !=
                     image->getExternalFormat( Frame::BUFFER_COLOR ))
            {
                return false;
            }
            if( hasDepth )
            {
                if( depthInternalFormat == 0 && depthExternalFormat == 0 )
                {
                    depthInternalFormat =
                        image->getInternalFormat( Frame::BUFFER_DEPTH );
                    depthExternalFormat =
                        image->getExternalFormat( Frame::BUFFER_DEPTH );

                    if( depthExternalFormat !=
                        EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT )
                    {
                        return false;
                    }
                }
                else if( depthInternalFormat !=
                         image->getInternalFormat(Frame::BUFFER_DEPTH ) ||
                         depthExternalFormat !=
                         image->getExternalFormat(  Frame::BUFFER_DEPTH ))
                {
                    return false;
                }
            }
            ++nImages;
        }
    }
    return (nImages > 1);
}
}

uint32_t Compositor::assembleFrames( const Frames& frames,
                                     Channel* channel, util::Accum* accum )
{
    if( frames.empty( ))
        return 0;

    if( _useCPUAssembly( frames, channel ))
        return assembleFramesCPU( frames, channel );

    // else
    return assembleFramesUnsorted( frames, channel, accum );
}

util::Accum* Compositor::_obtainAccum( Channel* channel )
{
    const PixelViewport& pvp = channel->getPixelViewport();

    LBASSERT( pvp.isValid( ));

    util::ObjectManager& objects = channel->getObjectManager();
    util::Accum* accum = objects.getEqAccum( channel );
    if( !accum )
    {
        accum = objects.newEqAccum( channel );
        if( !accum->init( pvp, channel->getWindow()->getColorFormat( )))
        {
            LBERROR << "Accumulation initialization failed." << std::endl;
        }
    }
    else
        accum->resize( pvp.w, pvp.h );

    accum->clear();
    return accum;
}

uint32_t Compositor::blendFrames( const Frames& frames, Channel* channel,
                                  util::Accum* accum )
{
    if( frames.empty( ))
        return 0;

    if( _isSubPixelDecomposition( frames ))
    {
        const bool coreProfile = channel->getWindow()->getIAttribute(
                    WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
        if( coreProfile )
        {
            LBERROR << "No support for sub pixel assembly for OpenGL core"
                       "profile, skipping assemble" << std::endl;
            return 0;
        }

        if( !accum )
        {
            accum = _obtainAccum( channel );
            accum->clear();

            const SubPixel& subpixel = frames.back()->getSubPixel();
            accum->setTotalSteps( subpixel.size );
        }

        uint32_t count = 0;
        Frames framesLeft = frames;
        while( !framesLeft.empty( ))
        {
            Frames current = _extractOneSubPixel( framesLeft );
            const uint32_t subCount = blendFrames( current, channel, accum );
            LBASSERT( subCount < 2 );

            if( subCount > 0 )
                accum->accum();
            count += subCount;
        }
        if( count > 0 )
            accum->display();
        return count;
    }

    glEnable( GL_BLEND );
    LBASSERT( GLEW_EXT_blend_func_separate );
    glBlendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );

    uint32_t count = 0;
    if( _useCPUAssembly( frames, channel, true ))
        count |= assembleFramesCPU( frames, channel, true );
    else
    {
        const uint32_t timeout = channel->getConfig()->getTimeout();
        for( Frames::const_iterator i = frames.begin();
             i != frames.end(); ++i )
        {
            Frame* frame = *i;
            {
                ChannelStatistics event( Statistic::CHANNEL_FRAME_WAIT_READY,
                                         channel );
                frame->waitReady( timeout );
            }

            if( !frame->getImages().empty( ))
            {
                count = 1;
                assembleFrame( frame, channel );
            }
        }
    }

    glDisable( GL_BLEND );
    return count;
}

bool Compositor::_isSubPixelDecomposition( const Frames& frames )
{
    if( frames.empty( ))
        return false;

    Frames::const_iterator i = frames.begin();
    Frame* frame = *i;
    const SubPixel& subpixel = frame->getSubPixel();

    for( ++i; i != frames.end(); ++i)
    {
        frame = *i;
        if( subpixel != frame->getSubPixel( ))
            return true;
    }

    return false;
}

const Frames Compositor::_extractOneSubPixel( Frames& frames )
{
    Frames current;

    const SubPixel& subpixel = frames.back()->getSubPixel();
    current.push_back( frames.back( ));
    frames.pop_back();

    for( Frames::iterator i = frames.begin(); i != frames.end(); )
    {
        Frame* frame = *i;

        if( frame->getSubPixel() == subpixel )
        {
            current.push_back( frame );
            i = frames.erase( i );
        }
        else
            ++i;
    }

    return current;
}

uint32_t Compositor::assembleFramesUnsorted( const Frames& frames,
                                             Channel* channel,
                                             util::Accum* accum )
{
    if( frames.empty( ))
        return 0;

    LBVERB << "Unsorted GPU assembly" << std::endl;
    if( _isSubPixelDecomposition( frames ))
    {
        const bool coreProfile = channel->getWindow()->getIAttribute(
                    WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
        if( coreProfile )
        {
            LBERROR << "No support for sub pixel assembly for OpenGL core"
                       "profile, skipping assemble" << std::endl;
            return 0;
        }

        uint32_t count = 0;

        if( !accum )
        {
            accum = _obtainAccum( channel );
            accum->clear();

            const SubPixel& subpixel = frames.back()->getSubPixel();
            accum->setTotalSteps( subpixel.size );
        }

        Frames framesLeft = frames;
        while( !framesLeft.empty( ))
        {
            // get the frames with the same subpixel compound
            Frames current = _extractOneSubPixel( framesLeft );

            // use assembleFrames to potentially benefit from CPU assembly
            const uint32_t subCount = assembleFrames( current, channel, accum );
            LBASSERT( subCount < 2 )
            if( subCount > 0 )
                accum->accum();
            count += subCount;
        }
        if( count > 1 )
            accum->display();
        return count;
    }

    // This is an optimized assembly version. The frames are not assembled in
    // the saved order, but in the order they become available, which is faster
    // because less time is spent waiting on frame availability.
    //
    // The ready frames are counted in a monitor. Whenever a frame becomes
    // available, it increments the monitor which causes this code to wake up
    // and assemble it.

    uint32_t count = 0;

    // wait and assemble frames
    WaitHandle* handle = startWaitFrames( frames, channel );
    for( Frame* frame = waitFrame( handle ); frame; frame = waitFrame( handle ))
    {
        if( frame->getImages().empty( ))
            continue;

        count = 1;
        assembleFrame( frame, channel );
    }

    return count;
}

class Compositor::WaitHandle
{
public:
    WaitHandle( const Frames& frames, Channel* ch )
            : left( frames ), channel( ch ), processed( 0 ) {}
    ~WaitHandle()
        {
            // de-register the monitor on eventual left-overs on error/exception
            for( FramesCIter i = left.begin(); i != left.end(); ++i )
                (*i)->removeListener( monitor );
            left.clear();
        }

    lunchbox::Monitor< uint32_t > monitor;
    Frames left;
    Channel* const channel;
    uint32_t processed;
};

Compositor::WaitHandle* Compositor::startWaitFrames( const Frames& frames,
                                                     Channel* channel )
{
    WaitHandle* handle = new WaitHandle( frames, channel );
    for( FramesCIter i = frames.begin(); i != frames.end(); ++i )
        (*i)->addListener( handle->monitor );

    return handle;
}

Frame* Compositor::waitFrame( WaitHandle* handle )
{
    if( handle->left.empty( ))
    {
        delete handle;
        return 0;
    }

    ChannelStatistics event( Statistic::CHANNEL_FRAME_WAIT_READY,
                             handle->channel );
    Config* config = handle->channel->getConfig();
    const uint32_t timeout = config->getTimeout();

    ++handle->processed;
    if( timeout == LB_TIMEOUT_INDEFINITE )
        handle->monitor.waitGE( handle->processed );
    else
    {
        const int64_t time = config->getTime() + timeout;
        const int64_t aliveTimeout = co::Global::getKeepaliveTimeout();

        while( !handle->monitor.timedWaitGE( handle->processed, aliveTimeout ))
        {
            // pings timed out nodes
            const bool pinged = config->getLocalNode()->pingIdleNodes();

            if( config->getTime() >= time || !pinged )
            {
                delete handle;
                throw Exception( Exception::TIMEOUT_INPUTFRAME );
            }
        }
    }

    for( FramesIter i = handle->left.begin(); i != handle->left.end(); ++i )
    {
        Frame* frame = *i;
        if( !frame->isReady( ))
            continue;

        frame->removeListener( handle->monitor );
        handle->left.erase( i );
        return frame;
    }

    LBASSERTINFO( false, "Unreachable code" );
    delete handle;
    return 0;
}

uint32_t Compositor::assembleFramesCPU( const Frames& frames, Channel* channel,
                                        const bool blendAlpha )
{
    if( frames.empty( ))
        return 0;

    LBVERB << "Sorted CPU assembly" << std::endl;
    // Assembles images from DB and 2D compounds using the CPU and then
    // assembles the result image. Does not yet support Pixel or Eye
    // compounds.

    const Image* result = mergeFramesCPU( frames, blendAlpha,
                                          channel->getConfig()->getTimeout( ));
    if( !result )
        return 0;

    // assemble result on dest channel
    ImageOp operation;
    operation.channel = channel;
    operation.buffers = Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH;
    assembleImage( result, operation );

#if 0
    static uint32_t counter = 0;
    std::ostringstream stringstream;
    stringstream << "Image_" << ++counter;
    result->writeImages( stringstream.str( ));
#endif

    return 1;
}

const Image* Compositor::mergeFramesCPU( const Frames& frames,
                                         const bool blendAlpha,
                                         const uint32_t timeout )
{
    LBVERB << "Sorted CPU assembly" << std::endl;

    // Collect input image information and check preconditions
    PixelViewport destPVP;
    uint32_t colorInternalFormat    = 0;
    uint32_t colorExternalFormat    = 0;
    uint32_t colorPixelSize         = 0;
    uint32_t depthInternalFormat    = 0;
    uint32_t depthExternalFormat    = 0;
    uint32_t depthPixelSize         = 0;

    if( !_collectOutputData( frames, destPVP,
                             colorInternalFormat, colorPixelSize,
                             colorExternalFormat,
                             depthInternalFormat, depthPixelSize,
                             depthExternalFormat, timeout ))
    {
        return 0;
    }

    // prepare output image
    if( !_resultImage )
        _resultImage = new Image;
    Image* result = _resultImage.get();

    // pre-condition check for current _merge implementations
    LBASSERT( colorInternalFormat != 0 );

    result->setPixelViewport( destPVP );

    PixelData colorPixels;
    colorPixels.internalFormat = colorInternalFormat;
    colorPixels.externalFormat = colorExternalFormat;
    colorPixels.pixelSize      = colorPixelSize;
    colorPixels.pvp            = destPVP;
    result->setPixelData( Frame::BUFFER_COLOR, colorPixels );

    void* destDepth = 0;
    if( depthInternalFormat != 0 ) // at least one depth assembly
    {
        LBASSERT( depthExternalFormat ==
                  EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT );
        PixelData depthPixels;
        depthPixels.internalFormat = depthInternalFormat;
        depthPixels.externalFormat = depthExternalFormat;
        depthPixels.pixelSize      = depthPixelSize;
        depthPixels.pvp            = destPVP;
        result->setPixelData( Frame::BUFFER_DEPTH, depthPixels );
        destDepth = result->getPixelPointer( Frame::BUFFER_DEPTH );
    }

    // assembly
    _mergeFrames( frames, blendAlpha,
                  result->getPixelPointer( Frame::BUFFER_COLOR ),
                  destDepth, destPVP );
    return result;
}

bool Compositor::_collectOutputData(
         const Frames& frames, PixelViewport& destPVP,
         uint32_t& colorInternalFormat, uint32_t& colorPixelSize,
         uint32_t& colorExternalFormat,
         uint32_t& depthInternalFormat, uint32_t& depthPixelSize,
         uint32_t& depthExternalFormat, const uint32_t timeout )
{
    for( Frames::const_iterator i = frames.begin(); i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->waitReady( timeout );

        LBASSERTINFO( frame->getPixel() == Pixel::ALL &&
                      frame->getSubPixel() == SubPixel::ALL &&
                      frame->getFrameData()->getZoom() == Zoom::NONE &&
                      frame->getZoom() == Zoom::NONE,
                      "CPU-based compositing not implemented for given frames");
        if( frame->getPixel() != Pixel::ALL )
            return false;

        const Images& images = frame->getImages();
        for( Images::const_iterator j = images.begin(); j != images.end(); ++j )
        {
            const Image* image = *j;
            LBASSERT( image->getStorageType() == Frame::TYPE_MEMORY );
            if( image->getStorageType() != Frame::TYPE_MEMORY )
                return false;

            if( !image->hasPixelData( Frame::BUFFER_COLOR ))
                continue;

            destPVP.merge( image->getPixelViewport() + frame->getOffset( ));

            _collectOutputData( image->getPixelData( Frame::BUFFER_COLOR ),
                                colorInternalFormat, colorPixelSize,
                                colorExternalFormat );

            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
            {
                _collectOutputData( image->getPixelData( Frame::BUFFER_DEPTH ),
                                    depthInternalFormat,
                                    depthPixelSize, depthExternalFormat );
            }
        }
    }

    if( !destPVP.hasArea( ))
    {
        LBWARN << "Nothing to assemble: " << destPVP << std::endl;
        return false;
    }

    return true;
}

void Compositor::_collectOutputData( const PixelData& pixelData,
                                     uint32_t& internalFormat,
                                     uint32_t& pixelSize,
                                     uint32_t& externalFormat )
{
    LBASSERT( internalFormat == GL_NONE ||
              internalFormat == pixelData.internalFormat );
    LBASSERT( externalFormat == GL_NONE ||
              externalFormat == pixelData.externalFormat );
    LBASSERT( pixelSize == GL_NONE || pixelSize == pixelData.pixelSize );
    internalFormat    = pixelData.internalFormat;
    pixelSize         = pixelData.pixelSize;
    externalFormat    = pixelData.externalFormat;
}

bool Compositor::mergeFramesCPU( const Frames& frames,
                                 const bool blendAlpha, void* colorBuffer,
                                 const uint32_t colorBufferSize,
                                 void* depthBuffer,
                                 const uint32_t depthBufferSize,
                                 PixelViewport& outPVP,
                                 const uint32_t timeout )
{
    LBASSERT( colorBuffer );
    LBVERB << "Sorted CPU assembly" << std::endl;

    // Collect input image information and check preconditions
    uint32_t colorInternalFormat    = 0;
    uint32_t colorExternalFormat    = 0;
    uint32_t colorPixelSize         = 0;
    uint32_t depthInternalFormat    = 0;
    uint32_t depthExternalFormat    = 0;
    uint32_t depthPixelSize         = 0;
    outPVP.invalidate();

    if( !_collectOutputData( frames, outPVP, colorInternalFormat,
                             colorPixelSize, colorExternalFormat,
                             depthInternalFormat,
                             depthPixelSize, depthExternalFormat, timeout ))
        return false;

    // pre-condition check for current _merge implementations
    LBASSERT( colorInternalFormat != 0 );

    // check output buffers
    const uint32_t area = outPVP.getArea();
    uint32_t nChannels = 0;

    switch ( colorInternalFormat )
    {
        case EQ_COMPRESSOR_DATATYPE_RGBA:
        case EQ_COMPRESSOR_DATATYPE_RGBA16F:
        case EQ_COMPRESSOR_DATATYPE_RGBA32F:
        case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
            nChannels = 4;
            break;
        case EQ_COMPRESSOR_DATATYPE_RGB:
        case EQ_COMPRESSOR_DATATYPE_RGB16F:
        case EQ_COMPRESSOR_DATATYPE_RGB32F:
            nChannels = 3;
            break;
        default:
            LBASSERT( false );
    }

    if( colorBufferSize < area * nChannels )
    {
        LBWARN << "Color output buffer to small" << std::endl;
        return false;
    }

    if( depthInternalFormat != 0 ) // at least one depth assembly
    {
        LBASSERT( depthBuffer );
        LBASSERT( depthInternalFormat == GL_DEPTH_COMPONENT );
        LBASSERT( depthExternalFormat ==
                  EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT );

        if( !depthBuffer )
        {
            LBWARN << "No depth output buffer provided" << std::endl;
            return false;
        }

        if( depthBufferSize < area * 4 )
        {
            LBWARN << "Depth output buffer to small" << std::endl;
            return false;
        }
    }

    // assembly
    _mergeFrames( frames, blendAlpha, colorBuffer, depthBuffer, outPVP );
    return true;
}

void Compositor::_mergeFrames( const Frames& frames, const bool blendAlpha,
                               void* colorBuffer, void* depthBuffer,
                               const PixelViewport& destPVP )
{
    for( Frames::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        const Frame* frame = *i;
        const Images& images = frame->getImages();
        for( Images::const_iterator j = images.begin(); j != images.end(); ++j )
        {
            const Image* image = *j;

            if( !image->hasPixelData( Frame::BUFFER_COLOR ))
                continue;

            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
                _mergeDBImage( colorBuffer, depthBuffer, destPVP,
                               image, frame->getOffset( ));
            else if( blendAlpha && image->hasAlpha( ))
                _mergeBlendImage( colorBuffer, destPVP,
                                  image, frame->getOffset( ));
            else
                _merge2DImage( colorBuffer, depthBuffer, destPVP,
                               image, frame->getOffset());
        }
    }
}

void Compositor::_mergeDBImage( void* destColor, void* destDepth,
                                const PixelViewport& destPVP,
                                const Image* image,
                                const Vector2i& offset )
{
    LBASSERT( destColor && destDepth );

    LBVERB << "CPU-DB assembly" << std::endl;

    uint32_t* destC = reinterpret_cast< uint32_t* >( destColor );
    uint32_t* destD = reinterpret_cast< uint32_t* >( destDepth );

    const PixelViewport&  pvp    = image->getPixelViewport();

    const int32_t         destX  = offset.x() + pvp.x - destPVP.x;
    const int32_t         destY  = offset.y() + pvp.y - destPVP.y;

    const uint32_t* color = reinterpret_cast< const uint32_t* >
        ( image->getPixelPointer( Frame::BUFFER_COLOR ));
    const uint32_t* depth = reinterpret_cast< const uint32_t* >
        ( image->getPixelPointer( Frame::BUFFER_DEPTH ));

#pragma omp parallel for
    for( int32_t y = 0; y < pvp.h; ++y )
    {
        const uint32_t skip =  (destY + y) * destPVP.w + destX;
        uint32_t* destColorIt = destC + skip;
        uint32_t* destDepthIt = destD + skip;
        const uint32_t* colorIt = color + y * pvp.w;
        const uint32_t* depthIt = depth + y * pvp.w;

        for( int32_t x = 0; x < pvp.w; ++x )
        {
            if( *destDepthIt > *depthIt )
            {
                *destColorIt = *colorIt;
                *destDepthIt = *depthIt;
            }

            ++destColorIt;
            ++destDepthIt;
            ++colorIt;
            ++depthIt;
        }
    }
}

void Compositor::_merge2DImage( void* destColor, void* destDepth,
                                const eq::PixelViewport& destPVP,
                                const Image* image,
                                const Vector2i& offset )
{
    // This is mostly copy&paste code from _mergeDBImage :-/
    LBVERB << "CPU-2D assembly" << std::endl;

    uint8_t* destC = reinterpret_cast< uint8_t* >( destColor );
    uint8_t* destD = reinterpret_cast< uint8_t* >( destDepth );

    const PixelViewport&  pvp    = image->getPixelViewport();
    const int32_t         destX  = offset.x() + pvp.x - destPVP.x;
    const int32_t         destY  = offset.y() + pvp.y - destPVP.y;

    LBASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));

    const uint8_t*   color = image->getPixelPointer( Frame::BUFFER_COLOR );
    const size_t pixelSize = image->getPixelSize( Frame::BUFFER_COLOR );
    const size_t rowLength = pvp.w * pixelSize;

#pragma omp parallel for
    for( int32_t y = 0; y < pvp.h; ++y )
    {
        const size_t skip = ( (destY + y) * destPVP.w + destX ) * pixelSize;
        memcpy( destC + skip, color + y * pvp.w * pixelSize, rowLength);
        // clear depth, for depth-assembly into existing FB
        if( destD )
            lunchbox::setZero( destD + skip, rowLength );
    }
}


void Compositor::_mergeBlendImage( void* dest, const eq::PixelViewport& destPVP,
                                   const Image* image,
                                   const Vector2i& offset )
{
    LBVERB << "CPU-Blend assembly" << std::endl;

    int32_t* destColor = reinterpret_cast< int32_t* >( dest );

    const PixelViewport&  pvp    = image->getPixelViewport();
    const int32_t         destX  = offset.x() + pvp.x - destPVP.x;
    const int32_t         destY  = offset.y() + pvp.y - destPVP.y;

    LBASSERT( image->getPixelSize( Frame::BUFFER_COLOR ) == 4 );
    LBASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
    LBASSERT( image->hasAlpha( ));

    const int32_t* color = reinterpret_cast< const int32_t* >
                               ( image->getPixelPointer( Frame::BUFFER_COLOR ));

    // Blending of two slices, none of which is on final image (i.e. result
    // could be blended on to something else) should be performed with:
    // glBlendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA )
    // which means:
    // dstColor = 1*srcColor + srcAlpha*dstColor
    // dstAlpha = 0*srcAlpha + srcAlpha*dstAlpha
    // because we accumulate light which is go through (= 1-Alpha) and we
    // already have colors as Alpha*Color

    int32_t* destColorStart = destColor + destY*destPVP.w + destX;
    const uint32_t step = sizeof( int32_t );

#pragma omp parallel for
    for( int32_t y = 0; y < pvp.h; ++y )
    {
        const unsigned char* src =
            reinterpret_cast< const uint8_t* >( color + pvp.w * y );
        unsigned char*       dst =
            reinterpret_cast< uint8_t* >( destColorStart + destPVP.w * y );

        for( int32_t x = 0; x < pvp.w; ++x )
        {
            dst[0] = LB_MIN( src[0] + (src[3]*dst[0] >> 8), 255 );
            dst[1] = LB_MIN( src[1] + (src[3]*dst[1] >> 8), 255 );
            dst[2] = LB_MIN( src[2] + (src[3]*dst[2] >> 8), 255 );
            dst[3] =                   src[3]*dst[3] >> 8;

            src += step;
            dst += step;
        }
    }
}

void Compositor::assembleFrame( const Frame* frame, Channel* channel )
{
    const Images& images = frame->getImages();
    if( images.empty( ))
        LBINFO << "No images to assemble" << std::endl;

    ImageOp operation;
    operation.channel = channel;
    operation.buffers = frame->getBuffers();
    operation.offset  = frame->getOffset();
    operation.pixel   = frame->getPixel();
    operation.zoom    = frame->getZoom();
    operation.zoom.apply( frame->getFrameData()->getZoom( ));

    for( Images::const_iterator i = images.begin(); i != images.end(); ++i )
    {
        const Image* image = *i;
        ImageOp op = operation;
        op.zoom.apply( image->getZoom() );
        op.zoomFilter = (operation.zoom == Zoom::NONE) ?
                                        FILTER_NEAREST : frame->getZoomFilter();
        assembleImage( image, op );
    }
}

void Compositor::assembleImage( const Image* image, const ImageOp& op )
{
    const bool coreProfile = op.channel->getWindow()->getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( coreProfile && op.pixel != Pixel::ALL )
    {
        LBERROR << "No support for pixel assembly for OpenGL core profile,"
                   "skipping assemble" << std::endl;
        return;
    }

    ImageOp operation = op;
    operation.buffers = Frame::BUFFER_NONE;

    const Frame::Buffer buffer[] = { Frame::BUFFER_COLOR, Frame::BUFFER_DEPTH };
    for( unsigned i = 0; i<2; ++i )
    {
        if( (op.buffers & buffer[i]) &&
            ( image->hasPixelData( buffer[i] ) ||
              image->hasTextureData( buffer[i] )) )
        {
            operation.buffers |= buffer[i];
        }
    }

    if( operation.buffers == Frame::BUFFER_NONE )
    {
        LBWARN << "No image attachment buffers to assemble" << std::endl;
        return;
    }

    setupStencilBuffer( image, operation );

    if( operation.buffers == Frame::BUFFER_COLOR )
        assembleImage2D( image, operation );
    else if( operation.buffers == ( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH ))
        assembleImageDB( image, operation );
    else
        LBWARN << "Don't know how to assemble using buffers "
               << operation.buffers << std::endl;

    clearStencilBuffer( operation );
}

void Compositor::setupStencilBuffer( const Image* image, const ImageOp& op )
{
    if( op.pixel == Pixel::ALL )
        return;

    // mark stencil buffer where pixel shall not pass
    // TODO: OPT!
    EQ_GL_CALL( glClear( GL_STENCIL_BUFFER_BIT ));
    EQ_GL_CALL( glEnable( GL_STENCIL_TEST ));
    EQ_GL_CALL( glEnable( GL_DEPTH_TEST ));

    EQ_GL_CALL( glStencilFunc( GL_ALWAYS, 1, 1 ));
    EQ_GL_CALL( glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE ));

    EQ_GL_CALL( glLineWidth( 1.0f ));
    EQ_GL_CALL( glDepthMask( false ));
    EQ_GL_CALL( glColorMask( false, false, false, false ));

    const PixelViewport& pvp = image->getPixelViewport();

    EQ_GL_CALL( glPixelZoom( float( op.pixel.w ), float( op.pixel.h )));

    if( op.pixel.w > 1 )
    {
        const float width  = float( pvp.w * op.pixel.w );
        const float step   = float( op.pixel.w );

        const float startX = float( op.offset.x() + pvp.x ) + 0.5f -
                             float( op.pixel.w );
        const float endX   = startX + width + op.pixel.w + step;
        const float startY = float( op.offset.y() + pvp.y + op.pixel.y );
        const float endY   = float( startY + pvp.h*op.pixel.h );

        glBegin( GL_QUADS );
        for( float x = startX + op.pixel.x + 1.0f ; x < endX; x += step)
        {
            glVertex3f( x-step, startY, 0.0f );
            glVertex3f( x-1.0f, startY, 0.0f );
            glVertex3f( x-1.0f, endY, 0.0f );
            glVertex3f( x-step, endY, 0.0f );
        }
        glEnd();
    }
    if( op.pixel.h > 1 )
    {
        const float height = float( pvp.h * op.pixel.h );
        const float step   = float( op.pixel.h );
        const float startX = float( op.offset.x() + pvp.x + op.pixel.x );
        const float endX   = float( startX + pvp.w * op.pixel.w );
        const float startY = float( op.offset.y() + pvp.y ) + 0.5f -
                             float( op.pixel.h );
        const float endY   = startY + height + op.pixel.h + step;

        glBegin( GL_QUADS );
        for( float y = startY + op.pixel.y; y < endY; y += step)
        {
            glVertex3f( startX, y-step, 0.0f );
            glVertex3f( endX,   y-step, 0.0f );
            glVertex3f( endX,   y-1.0f, 0.0f );
            glVertex3f( startX, y-1.0f, 0.0f );
        }
        glEnd();
    }

    EQ_GL_CALL( glDisable( GL_DEPTH_TEST ));
    EQ_GL_CALL( glStencilFunc( GL_EQUAL, 0, 1 ));
    EQ_GL_CALL( glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP ));

    const ColorMask& colorMask = op.channel->getDrawBufferMask();
    EQ_GL_CALL( glColorMask( colorMask.red, colorMask.green, colorMask.blue,
                             true ));
    EQ_GL_CALL( glDepthMask( true ));
}

void Compositor::clearStencilBuffer( const ImageOp& op )
{
    if( op.pixel == Pixel::ALL )
        return;

    EQ_GL_CALL( glPixelZoom( 1.f, 1.f ));
    EQ_GL_CALL( glDisable( GL_STENCIL_TEST ));
}

void Compositor::assembleImage2D( const Image* image, const ImageOp& op )
{
    // cppcheck-suppress unreadVariable
    Channel* channel = op.channel;

    if( GLEW_VERSION_3_3 )
        _drawPixelsGLSL( image, op, Frame::BUFFER_COLOR );
    else
        _drawPixelsFF( image, op, Frame::BUFFER_COLOR );
    declareRegion( image, op );
#if 0
    static lunchbox::a_int32_t counter;
    std::ostringstream stringstream;
    stringstream << "Image_" << ++counter;
    image->writeImages( stringstream.str( ));
#endif
}

void Compositor::_drawPixelsFF( const Image* image, const ImageOp& op,
                                const Frame::Buffer which )
{
    const PixelViewport& pvp = image->getPixelViewport();
    LBLOG( LOG_ASSEMBLY ) << "_drawPixelsFF " << pvp << " offset " << op.offset
                          << std::endl;

    if( !_setupDrawPixels( image, op, which ))
        return;

    const Vector4f& coords = _getCoords( op, pvp );

    EQ_GL_CALL( glDisable( GL_LIGHTING ));
    EQ_GL_CALL( glEnable( GL_TEXTURE_RECTANGLE_ARB ));

    EQ_GL_CALL( glColor3f( 1.0f, 1.0f, 1.0f ));

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( coords[0], coords[2], 0.0f );

        glTexCoord2f( float( pvp.w ), 0.0f );
        glVertex3f( coords[1], coords[2], 0.0f );

        glTexCoord2f( float( pvp.w ), float( pvp.h ));
        glVertex3f( coords[1], coords[3], 0.0f );

        glTexCoord2f( 0.0f, float( pvp.h ));
        glVertex3f( coords[0], coords[3], 0.0f );
    glEnd();

    // restore state
    EQ_GL_CALL( glDisable( GL_TEXTURE_RECTANGLE_ARB ));

    if ( which == Frame::BUFFER_COLOR )
        EQ_GL_CALL( glDepthMask( true ))
    else
    {
        const ColorMask& colorMask = op.channel->getDrawBufferMask();
        EQ_GL_CALL( glColorMask( colorMask.red, colorMask.green, colorMask.blue,
                                 true ));
    }
}

void Compositor::_drawPixelsGLSL( const Image* image, const ImageOp& op,
                                  const Frame::Buffer which )
{
    const PixelViewport& pvp = image->getPixelViewport();
    LBLOG( LOG_ASSEMBLY ) << "_drawPixelsGLSL " << pvp << " offset "
                          << op.offset << std::endl;

    if( !_setupDrawPixels( image, op, which ))
        return;

    _drawTexturedQuad( op.channel, op, pvp, false );

    // restore state
    if ( which == Frame::BUFFER_COLOR )
        EQ_GL_CALL( glDepthMask( true ))
    else
    {
        const ColorMask& colorMask = op.channel->getDrawBufferMask();
        EQ_GL_CALL( glColorMask( colorMask.red, colorMask.green, colorMask.blue,
                                 true ));
    }
}

bool Compositor::_setupDrawPixels( const Image* image, const ImageOp& op,
                                   const Frame::Buffer which )
{
    Channel* channel = op.channel; // needed for glewGetContext
    const PixelViewport& pvp = image->getPixelViewport();
    const util::Texture* texture = 0;
    if( image->getStorageType() == Frame::TYPE_MEMORY )
    {
        LBASSERT( image->hasPixelData( which ));
        util::ObjectManager& objects = channel->getObjectManager();

        const bool coreProfile = channel->getWindow()->getIAttribute(
                    WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
        if( op.zoom == Zoom::NONE && !coreProfile )
        {
            image->upload( which, 0, op.offset, objects );
            return false;
        }
        util::Texture* ncTexture = objects.obtainEqTexture(
            which == Frame::BUFFER_COLOR ? colorDBKey : depthDBKey,
            GL_TEXTURE_RECTANGLE_ARB );
        texture = ncTexture;

        const Vector2i offset( -pvp.x, -pvp.y ); // will be applied with quad
        image->upload( which, ncTexture, offset, objects );
    }
    else // texture image
    {
        LBASSERT( image->hasTextureData( which ));
        texture = &image->getTexture( which );
    }

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE0 ));
    texture->bind();
    texture->applyZoomFilter( op.zoomFilter );
    texture->applyWrap();

    if ( which == Frame::BUFFER_COLOR )
        EQ_GL_CALL( glDepthMask( false ))
    else
    {
        LBASSERT( which == Frame::BUFFER_DEPTH )
        EQ_GL_CALL( glColorMask( false, false, false, false ));
    }
    return true;
}

Vector4f Compositor::_getCoords( const ImageOp& op, const PixelViewport& pvp )
{
    return Vector4f(
        op.offset.x() + pvp.x * op.pixel.w + op.pixel.x,
        op.offset.x() + pvp.getXEnd() * op.pixel.w*op.zoom.x() + op.pixel.x,
        op.offset.y() + pvp.y * op.pixel.h + op.pixel.y,
        op.offset.y() + pvp.getYEnd() * op.pixel.h*op.zoom.y() + op.pixel.y );
}

template< typename T >
void Compositor::_drawTexturedQuad( const T* key, const ImageOp& op,
                                    const PixelViewport& pvp,
                                    const bool withDepth )
{
    Channel* channel = op.channel; // needed for glewGetContext
    util::ObjectManager& om = channel->getObjectManager();
    GLuint program = om.getProgram( key );
    GLuint vertexArray = om.getVertexArray( key );
    GLuint vertexBuffer = om.getBuffer( key );
    GLuint uvBuffer = om.getBuffer( key + 1 );
    if( program == util::ObjectManager::INVALID )
    {
        vertexBuffer = om.newBuffer( key );
        uvBuffer = om.newBuffer( key + 1 );
        vertexArray = om.newVertexArray( key );
        program = om.newProgram( key );

        const char* vertexShaderGLSL = {
            "#version 330 core\n"
            "layout(location = 0) in vec3 vert;\n"
            "layout(location = 1) in vec2 vertTexCoord;\n"
            "uniform mat4 proj;\n"
            "out vec2 fragTexCoord;\n"
            "void main() {\n"
            "    fragTexCoord = vertTexCoord;\n"
            "    gl_Position = proj * vec4(vert, 1);\n"
            "}\n"
        };

        const char* fragmentShaderGLSL = { withDepth ?
            "#version 330 core\n"
            "#extension GL_ARB_texture_rectangle : enable\n"
            "uniform sampler2DRect color;\n"
            "uniform sampler2DRect depth;\n"
            "in vec2 fragTexCoord;\n"
            "out vec4 finalColor;\n"
            "void main() {\n"
            "    finalColor = texture2DRect(color, fragTexCoord);\n"
            "    gl_FragDepth = texture2DRect(depth, fragTexCoord).x;\n"
            "}\n"
                : // no depth
            "#version 330 core\n"
            "#extension GL_ARB_texture_rectangle : enable\n"
            "uniform sampler2DRect color;\n"
            "in vec2 fragTexCoord;\n"
            "out vec4 finalColor;\n"
            "void main() {\n"
            "    finalColor = texture2DRect(color, fragTexCoord);\n"
            "}\n"
        };

        LBCHECK( util::shader::linkProgram( glewGetContext(), program,
                                            vertexShaderGLSL,
                                            fragmentShaderGLSL ));

        EQ_GL_CALL( glUseProgram( program ));

        GLint colorParam = glGetUniformLocation( program, "color" );
        EQ_GL_CALL( glUniform1i( colorParam, 0 ));
        if( withDepth )
        {
            const GLint depthParam = glGetUniformLocation( program, "depth" );
            EQ_GL_CALL( glUniform1i( depthParam, 1 ));
        }
    }

    const Vector4f& coords = _getCoords( op, pvp );
    const GLfloat vertices[] = {
        coords[0], coords[2], 0.0f,
        coords[1], coords[2], 0.0f,
        coords[0], coords[3], 0.0f,
        coords[1], coords[3], 0.0f
    };

    const GLfloat uvs[] = {
        0.0f, 0.0f,
        float( pvp.w ), 0.0f,
        0.0f, float( pvp.h ),
        float( pvp.w ), float( pvp.h )
    };

    eq::Frustumf frustum;
    frustum.left() = channel->getPixelViewport().x;
    frustum.right() = channel->getPixelViewport().getXEnd();
    frustum.bottom() = channel->getPixelViewport().y;
    frustum.top() = channel->getPixelViewport().getYEnd();
    frustum.far_plane() = 1.0f;
    frustum.near_plane() = -1.0f;
    const eq::Matrix4f& proj = frustum.compute_ortho_matrix();

    if( withDepth )
        EQ_GL_CALL( glEnable( GL_DEPTH_TEST ));

    EQ_GL_CALL( glBindVertexArray( vertexArray ));
    EQ_GL_CALL( glUseProgram( program ));

    const GLuint projection = glGetUniformLocation( program, "proj" );
    EQ_GL_CALL( glUniformMatrix4fv( projection, 1, GL_FALSE, &proj[0] ));

    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer ));
    EQ_GL_CALL( glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                              GL_DYNAMIC_DRAW ));
    EQ_GL_CALL( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 ));

    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, uvBuffer ));
    EQ_GL_CALL( glBufferData( GL_ARRAY_BUFFER, sizeof(uvs), uvs,
                              GL_DYNAMIC_DRAW ));
    EQ_GL_CALL( glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 ));
    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, 0 ));

    EQ_GL_CALL( glEnableVertexAttribArray( 0 ));
    EQ_GL_CALL( glEnableVertexAttribArray( 1 ));
    EQ_GL_CALL( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ));
    EQ_GL_CALL( glDisableVertexAttribArray( 0 ));
    EQ_GL_CALL( glDisableVertexAttribArray( 1 ));

    EQ_GL_CALL( glBindVertexArray( 0 ));
    EQ_GL_CALL( glUseProgram( 0 ));

    if( withDepth )
        EQ_GL_CALL( glDisable( GL_DEPTH_TEST ));
}

void Compositor::assembleImageDB( const Image* image, const ImageOp& op )
{
    // cppcheck-suppress unreadVariable
    Channel* channel = op.channel;

    if( GLEW_VERSION_3_3 )
        assembleImageDB_GLSL( image, op );
    else
        assembleImageDB_FF( image, op );
}

void Compositor::assembleImageDB_FF( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    LBLOG( LOG_ASSEMBLY ) << "assembleImageDB_FF " << pvp
                          << std::endl;

    // Z-Based sort-last assembly
    EQ_GL_CALL( glRasterPos2i( op.offset.x() + pvp.x, op.offset.y() + pvp.y ));
    EQ_GL_CALL( glEnable( GL_STENCIL_TEST ));

    // test who is in front and mark in stencil buffer
    EQ_GL_CALL( glEnable( GL_DEPTH_TEST ));

    const bool pixelComposite = ( op.pixel != Pixel::ALL );
    if( pixelComposite )
    {   // keep already marked stencil values
        EQ_GL_CALL( glStencilFunc( GL_EQUAL, 1, 1 ));
        EQ_GL_CALL( glStencilOp( GL_KEEP, GL_ZERO, GL_REPLACE ));
    }
    else
    {
        EQ_GL_CALL( glStencilFunc( GL_ALWAYS, 1, 1 ));
        EQ_GL_CALL( glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE ));
    }

    _drawPixelsFF( image, op, Frame::BUFFER_DEPTH );

    EQ_GL_CALL( glDisable( GL_DEPTH_TEST ));

    // draw front-most, visible pixels using stencil mask
    EQ_GL_CALL( glStencilFunc( GL_EQUAL, 1, 1 ));
    EQ_GL_CALL( glStencilOp( GL_KEEP, GL_ZERO, GL_ZERO ));

    _drawPixelsFF( image, op, Frame::BUFFER_COLOR );

    EQ_GL_CALL( glDisable( GL_STENCIL_TEST ));
    declareRegion( image, op );
}

void Compositor::assembleImageDB_GLSL( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();
    LBLOG( LOG_ASSEMBLY ) << "assembleImageDB_GLSL " << pvp << std::endl;

    Channel* channel = op.channel; // needed for glewGetContext
    util::ObjectManager& om = channel->getObjectManager();
    const bool useImageTexture = image->getStorageType() == Frame::TYPE_TEXTURE;

    const util::Texture* textureColor = 0;
    const util::Texture* textureDepth = 0;
    if ( useImageTexture )
    {
        textureColor = &image->getTexture( Frame::BUFFER_COLOR );
        textureDepth = &image->getTexture( Frame::BUFFER_DEPTH );
    }
    else
    {
        util::Texture* ncTextureColor = om.obtainEqTexture( colorDBKey,
                                                     GL_TEXTURE_RECTANGLE_ARB );
        util::Texture* ncTextureDepth = om.obtainEqTexture( depthDBKey,
                                                     GL_TEXTURE_RECTANGLE_ARB );
        const Vector2i offset( -pvp.x, -pvp.y ); // will be applied with quad

        image->upload( Frame::BUFFER_COLOR, ncTextureColor, offset, om );
        image->upload( Frame::BUFFER_DEPTH, ncTextureDepth, offset, om );

        textureColor = ncTextureColor;
        textureDepth = ncTextureDepth;
    }

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE1 ));
    textureDepth->bind();
    textureDepth->applyZoomFilter( op.zoomFilter );

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE0 ));
    textureColor->bind();
    textureColor->applyZoomFilter( op.zoomFilter );

    _drawTexturedQuad( shaderDBKey, op, pvp, true );

    declareRegion( image, op );
}

void Compositor::declareRegion( const Image* image, const ImageOp& op )
{
    if( !op.channel )
        return;

    const eq::PixelViewport area = image->getPixelViewport() + op.offset;
    op.channel->declareRegion( area );
}

#undef glewGetContext
#define glewGetContext() glCtx

void Compositor::setupAssemblyState( const PixelViewport& pvp,
                                     const GLEWContext* glCtx )
{
    EQ_GL_ERROR( "before setupAssemblyState" );
    glPushAttrib( GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT | GL_LINE_BIT |
        GL_PIXEL_MODE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT );

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );
    if( GLEW_VERSION_1_2 )
        glDisable( GL_TEXTURE_3D );
    glDisable( GL_FOG );
    glDisable( GL_CLIP_PLANE0 );
    glDisable( GL_CLIP_PLANE1 );
    glDisable( GL_CLIP_PLANE2 );
    glDisable( GL_CLIP_PLANE3 );
    glDisable( GL_CLIP_PLANE4 );
    glDisable( GL_CLIP_PLANE5 );

    glPolygonMode( GL_FRONT, GL_FILL );
    if( GLEW_VERSION_1_3 )
        glActiveTexture( GL_TEXTURE0 );

    glMatrixMode( GL_TEXTURE );
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    if( pvp.hasArea( ))
        glOrtho( pvp.x, pvp.getXEnd(), pvp.y, pvp.getYEnd(), -1.0f, 1.0f );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    EQ_GL_ERROR( "after  setupAssemblyState" );
}

void Compositor::resetAssemblyState()
{
    EQ_GL_CALL( glMatrixMode( GL_TEXTURE ) );
    EQ_GL_CALL( glPopMatrix() );
    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ) );
    EQ_GL_CALL( glPopMatrix() );
    EQ_GL_CALL( glMatrixMode( GL_MODELVIEW ));
    EQ_GL_CALL( glPopMatrix());
    EQ_GL_CALL( glPopAttrib());
}

}
