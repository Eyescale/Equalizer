
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <pthread.h>
#include <eq/base/perThread.h>

#include "accum.h"
#include "channel.h"
#include "channelStatistics.h"
#include "client.h"
#include "compositor.h"
#include "image.h"
#include "log.h"
#include "server.h"
#include "windowSystem.h"

#include <eq/base/debug.h>

#include <eq/base/executionListener.h>
#include <eq/base/monitor.h>

#ifdef EQ_USE_PARACOMP
#  include <pcapi.h>
#endif

#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#endif

using eq::base::Monitor;
using namespace std;

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
static const char* colorKey    = shaderDBKey + 3;
static const char* depthKey    = shaderDBKey + 4;

class ResultImage : public Image
{
public:
    virtual ~ResultImage() {}

    void notifyPerThreadDelete() { delete this; }
};


// Image used for CPU-based assembly
static base::PerThread< ResultImage > _resultImage;


static bool _useCPUAssembly( const FrameVector& frames, Channel* channel, 
                             const bool blendAlpha = false )
{
    // It doesn't make sense to use CPU-assembly for only one frame
    if( frames.size() < 2 )
        return false;

    // Test that at least two input frames have color and depth buffers or that
    // alpha-blended assembly is used with multiple RGBA buffers. We assume then
    // that we will have at least one image per frame so most likely it's worth
    // to wait for the images and to do a CPU-based assembly.
    const uint32_t desiredBuffers = blendAlpha ? Frame::BUFFER_COLOR :
                                    Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH;
    size_t nFrames = 0;
    for( FrameVector::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const Frame* frame = *i;
        if( frame->getPixel() != Pixel::ALL ||
            frame->getSubPixel() != SubPixel::ALL ) // Not yet supported
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
    // worthwhile and all other preconditions for our current CPU-based assembly
    // code are true.
    size_t   nImages     = 0;
    uint32_t colorFormat = 0;
    uint32_t colorType   = 0;
    uint32_t depthFormat = 0;
    uint32_t depthType   = 0;

    for( FrameVector::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const Frame* frame = *i;
        {
            ChannelStatistics event( Statistic::CHANNEL_WAIT_FRAME, channel );
            frame->waitReady();
        }

        const vector< Image* >& images = frame->getImages();        
        for( vector< Image* >::const_iterator j = images.begin(); 
             j != images.end(); ++j )
        {
            const Image* image = *j;

            const bool hasColor = image->hasPixelData( Frame::BUFFER_COLOR );
            const bool hasDepth = image->hasPixelData( Frame::BUFFER_DEPTH );
            
            if(( blendAlpha && hasColor && image->hasAlpha( )) ||
               ( hasColor && hasDepth ))
            {
                if( colorFormat == 0 )
                {
                    colorFormat = image->getFormat( Frame::BUFFER_COLOR );
                    colorType   = image->getType(   Frame::BUFFER_COLOR );

                    if (( colorType == GL_HALF_FLOAT ) || 
                        ( colorType == GL_FLOAT ))
                        return false;
                }

                if( colorFormat != image->getFormat( Frame::BUFFER_COLOR ) ||
                    colorType   != image->getType(   Frame::BUFFER_COLOR ))

                    return false;

                if( image->hasPixelData( Frame::BUFFER_DEPTH ))
                {
                    if( depthFormat == 0 )
                    {
                        depthFormat = image->getFormat( Frame::BUFFER_DEPTH );
                        depthType   = image->getType(   Frame::BUFFER_DEPTH );
                    }

                    if( depthFormat != image->getFormat(Frame::BUFFER_DEPTH ) ||
                        depthType   != image->getType(  Frame::BUFFER_DEPTH ))

                        return false;
                }

                ++nImages;
            }
        }

        if( nImages > 1 ) // early-out to reduce wait time
            return true;
    }

    return false;
}
}

void Compositor::assembleFrames( const FrameVector& frames,
                                 Channel* channel, Accum* accum )
{
    if( frames.empty( ))
        return;

    if( _useCPUAssembly( frames, channel ))
        assembleFramesCPU( frames, channel );
    else
        assembleFramesUnsorted( frames, channel, accum );
}

Accum* Compositor::_obtainAccum( Channel* channel )
{
    const PixelViewport& pvp = channel->getPixelViewport();

    EQASSERT( pvp.isValid( ));

    Window::ObjectManager* objects = channel->getObjectManager();
    Accum* accum = objects->getEqAccum( channel );
    if( !accum )
    {
        accum = objects->newEqAccum( channel );
        if( !accum->init( pvp, channel->getWindow()->getColorFormat( )))
        {
            EQERROR << "Accumulation initialization failed." << std::endl;
        }
    }
    else
        accum->resize( pvp.w, pvp.h );

    accum->clear();
    return accum;
}

uint32_t Compositor::assembleFramesSorted( const FrameVector& frames,
                                       Channel* channel, Accum* accum,
                                       const bool blendAlpha )
{
    uint32_t count = 0;
    if( frames.empty( ))
        return count;

    if( _isSubPixelDecomposition( frames ))
    {
        if( !accum )
        {
            accum = _obtainAccum( channel );
            accum->clear();

            const SubPixel& subpixel = frames.back()->getSubPixel();
            accum->setTotalSteps( subpixel.size );
        }

        FrameVector framesLeft = frames;
        while( !framesLeft.empty( ))
        {
            FrameVector current = _extractOneSubPixel( framesLeft );
            count = assembleFramesSorted( current, channel, accum );
            if( count > 0 )
                accum->accum();
        }
        if( count > 0 )
            accum->display();
        return count;
    }

    if( blendAlpha )
    {
        glEnable( GL_BLEND );
        EQASSERT( GLEW_EXT_blend_func_separate );
        glBlendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );
    }

    if( _useCPUAssembly( frames, channel, blendAlpha ))
        assembleFramesCPU( frames, channel, blendAlpha );
    else
    {
        for( FrameVector::const_iterator i = frames.begin();
             i != frames.end(); ++i )
        {
            Frame* frame = *i;
            {
                ChannelStatistics event( Statistic::CHANNEL_WAIT_FRAME,
                                         channel );
                frame->waitReady( );
            }

            if( !frame->getImages().empty( ))
            {
                ++count;
                assembleFrame( frame, channel );
            }
        }
    }

    if( blendAlpha )
        glDisable( GL_BLEND );

    return count;
}

bool Compositor::_isSubPixelDecomposition( const FrameVector& frames )
{
    if( frames.empty( ))
        return false;
        
    FrameVector::const_iterator i = frames.begin();
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

const FrameVector Compositor::_extractOneSubPixel( FrameVector& frames )
{
    FrameVector current;

    const SubPixel& subpixel = frames.back()->getSubPixel();
    current.push_back( frames.back( ));
    frames.pop_back();

    for( FrameVector::iterator i = frames.begin();
                i != frames.end(); )
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

uint32_t Compositor::assembleFramesUnsorted( const FrameVector& frames,
                                         Channel* channel, Accum* accum )
{
    uint32_t count = 0;

    if( frames.empty( ))
        return count;

    if( _isSubPixelDecomposition( frames ))
    {
        if( !accum )
        {
            accum = _obtainAccum( channel );
            accum->clear();

            const SubPixel& subpixel = frames.back()->getSubPixel();
            accum->setTotalSteps( subpixel.size );
        }

        FrameVector framesLeft = frames;
    	while( !framesLeft.empty( ))
    	{
    	    // get the frames with the same subpixel compound
    	    FrameVector current = _extractOneSubPixel( framesLeft );
    	    count = assembleFramesUnsorted( current, channel, accum );
    	    if( count > 0 )
    	        accum->accum();
        }
    	if( count > 0 )
    	    accum->display();
        return count;
    }

    EQVERB << "Unsorted GPU assembly" << endl;
    // This is an optimized assembly version. The frames are not assembled in
    // the saved order, but in the order they become available, which is faster
    // because less time is spent waiting on frame availability.
    //
    // The ready frames are counted in a monitor. Whenever a frame becomes
    // available, it increments the monitor which causes this code to wake up
    // and assemble it.

    // register monitor with all input frames
    Monitor<uint32_t> monitor;
    for( FrameVector::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->addListener( monitor );
    }

    uint32_t    nUsedFrames  = 0;
    FrameVector unusedFrames = frames;

    // wait and assemble frames
    while( !unusedFrames.empty( ))
    {
        {
            ChannelStatistics event( Statistic::CHANNEL_WAIT_FRAME, channel );
            monitor.waitGE( ++nUsedFrames );
        }

        for( FrameVector::iterator i = unusedFrames.begin();
             i != unusedFrames.end(); ++i )
        {
            Frame* frame = *i;
            if( !frame->isReady( ))
                continue;

			if( !frame->getImages().empty( ))
			{
			    ++count;
	            assembleFrame( frame, channel );
			}
    
            unusedFrames.erase( i );
            break;
        }
    }

    // de-register the monitor
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end();
         ++i )
    {
        Frame* frame = *i;
        // syncAssembleFrame( frame );
        frame->removeListener( monitor );
    }

    return count;
}

void Compositor::assembleFramesCPU( const FrameVector& frames,
                                    Channel* channel, const bool blendAlpha )
{
    if( frames.empty( ))
        return;

    EQVERB << "Sorted CPU assembly" << endl;
    // Assembles images from DB and 2D compounds using the CPU and then
    // assembles the result image. Does not yet support Pixel or Eye
    // compounds.

    const Image* result = mergeFramesCPU( frames, blendAlpha );
    if( !result )
        return;

    // assemble result on dest channel
    ImageOp operation;
    operation.channel = channel;
    operation.buffers = Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH;
    assembleImage( result, operation );

#if 0
    static uint32_t counter = 0;
    ostringstream stringstream;
    stringstream << "Image_" << ++counter;
    result->writeImages( stringstream.str( ));
#endif
}

const Image* Compositor::mergeFramesCPU( const FrameVector& frames,
                                         const bool blendAlpha )
{
    EQVERB << "Sorted CPU assembly" << endl;

    // Collect input image information and check preconditions
    PixelViewport destPVP;
    uint32_t colorFormat = GL_NONE;
    uint32_t colorType   = GL_NONE;
    uint32_t depthFormat = GL_NONE;
    uint32_t depthType   = GL_NONE;

    if( !_collectOutputData( frames, destPVP, 
                             colorFormat, colorType, depthFormat, depthType ))
    {
        return 0;
    }

    // prepare output image
    Image* result = _resultImage.get();
    if( !result )
    {
        _resultImage = new ResultImage;
        result       = _resultImage.get();
    }

    // pre-condition check for current _merge implementations
    EQASSERT( colorFormat != GL_NONE );
    EQASSERT( colorType   != GL_NONE );

    result->setFormat( Frame::BUFFER_COLOR, colorFormat );
    result->setType(   Frame::BUFFER_COLOR, colorType );
    result->setPixelViewport( destPVP );
    result->clearPixelData( Frame::BUFFER_COLOR );

    void* destDepth = 0;
    if( depthFormat != GL_NONE ) // at least one depth assembly
    {
        EQASSERT( depthFormat == GL_DEPTH_COMPONENT );
        EQASSERT( depthType   == GL_UNSIGNED_INT );

        result->setFormat( Frame::BUFFER_DEPTH, depthFormat );
        result->setType(   Frame::BUFFER_DEPTH, depthType );
        result->clearPixelData( Frame::BUFFER_DEPTH );
        destDepth = result->getPixelPointer( Frame::BUFFER_DEPTH );
    }

    // assembly
    _mergeFrames( frames, blendAlpha, 
                  result->getPixelPointer( Frame::BUFFER_COLOR ),
                  destDepth, destPVP );
    return result;
}

bool Compositor::_collectOutputData( const FrameVector& frames,
                                     PixelViewport& destPVP, 
                                     uint32_t& colorFormat, uint32_t& colorType,
                                     uint32_t& depthFormat, uint32_t& depthType)
{
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->waitReady();

        EQASSERTINFO( frame->getPixel() == Pixel::ALL,
                      "CPU-based pixel recomposition not implemented" );
        if( frame->getPixel() != Pixel::ALL )
            return false;

        const vector< Image* >& images = frame->getImages();        
        for( vector< Image* >::const_iterator j = images.begin(); 
             j != images.end(); ++j )
        {
            const Image* image = *j;
            EQASSERT( image->getStorageType() == Frame::TYPE_MEMORY );
            if( image->getStorageType() != Frame::TYPE_MEMORY )
                return false;

            if( !image->hasPixelData( Frame::BUFFER_COLOR ))
                continue;

            destPVP.merge( image->getPixelViewport() + frame->getOffset( ));

            EQASSERT( colorFormat == GL_NONE ||
                      colorFormat == image->getFormat( Frame::BUFFER_COLOR ));
            EQASSERT( colorType == GL_NONE ||
                      colorType == image->getType( Frame::BUFFER_COLOR ));

            colorFormat = image->getFormat( Frame::BUFFER_COLOR );
            colorType   = image->getType( Frame::BUFFER_COLOR );

            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
            {
                EQASSERT( depthFormat == GL_NONE ||
                          depthFormat == image->getFormat(Frame::BUFFER_DEPTH));
                EQASSERT( depthType == GL_NONE ||
                          depthType == image->getType( Frame::BUFFER_DEPTH ));

                depthFormat = image->getFormat( Frame::BUFFER_DEPTH );
                depthType   = image->getType( Frame::BUFFER_DEPTH );
            }
        }
    }

    if( !destPVP.hasArea( ))
    {
        EQWARN << "Nothing to assemble: " << destPVP << endl;
        return false;
    }

    return true;
}

bool Compositor::mergeFramesCPU( const FrameVector& frames,
                                 const bool blendAlpha, void* colorBuffer,
                                 const uint32_t colorBufferSize,
                                 void* depthBuffer,
                                 const uint32_t depthBufferSize,
                                 PixelViewport& outPVP )
{
    EQASSERT( colorBuffer );
    EQVERB << "Sorted CPU assembly" << endl;
    
    // Collect input image information and check preconditions
    uint32_t colorFormat = GL_NONE;
    uint32_t colorType   = GL_NONE;
    uint32_t depthFormat = GL_NONE;
    uint32_t depthType   = GL_NONE;
    outPVP.invalidate();

    if( !_collectOutputData( frames, outPVP, 
                             colorFormat, colorType, depthFormat, depthType ))
    {
        return false;
    }

    // pre-condition check for current _merge implementations
    EQASSERT( colorFormat != GL_NONE );
    EQASSERT( colorType   != GL_NONE );

    // check output buffers
    const uint32_t area = outPVP.getArea();
    const uint32_t colorPixelDepth = (colorFormat == GL_RGB) ? 3 : 4;

    if( colorBufferSize < area * colorPixelDepth )
    {
        EQWARN << "Color output buffer to small" << endl;
        return false;
    }

    if( depthFormat != GL_NONE ) // at least one depth assembly
    {
        EQASSERT( depthBuffer );
        EQASSERT( depthFormat == GL_DEPTH_COMPONENT );
        EQASSERT( depthType   == GL_UNSIGNED_INT );

        if( !depthBuffer )
        {
            EQWARN << "No depth output buffer provided" << endl;
            return false;
        }

        if( depthBufferSize < area * 4 )
        {
            EQWARN << "Depth output buffer to small" << endl;
            return false;
        }
    }

    // assembly
    _mergeFrames( frames, blendAlpha, colorBuffer, depthBuffer, outPVP );
    return true;
}

void Compositor::_mergeFrames( const FrameVector& frames,
                               const bool blendAlpha, 
                               void* colorBuffer, void* depthBuffer,
                               const PixelViewport& destPVP )
{
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        const Frame* frame = *i;
        const ImageVector& images = frame->getImages();        
        for( ImageVector::const_iterator j = images.begin();
             j != images.end(); ++j )
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
    EQASSERT( destColor && destDepth );

    EQVERB << "CPU-DB assembly" << endl;

    uint32_t* destC = reinterpret_cast< uint32_t* >( destColor );
    uint32_t* destD = reinterpret_cast< uint32_t* >( destDepth );

    const PixelViewport&  pvp    = image->getPixelViewport();

#ifdef EQ_USE_PARACOMP_DEPTH
    if( pvp == destPVP && offset == eq::Vector2i::ZERO )
    {
        // Use Paracomp to composite
        if( _mergeImage_PC( PC_COMP_DEPTH, destColor, destDepth, image ))
            return;

        EQWARN << "Paracomp compositing failed, using fallback" << endl;
    }
#endif

    const int32_t         destX  = offset.x() + pvp.x - destPVP.x;
    const int32_t         destY  = offset.y() + pvp.y - destPVP.y;

    const uint32_t* color = reinterpret_cast< const uint32_t* >
        ( image->getPixelPointer( Frame::BUFFER_COLOR ));
    const uint32_t* depth = reinterpret_cast< const uint32_t* >
        ( image->getPixelPointer( Frame::BUFFER_DEPTH ));

#ifdef EQ_USE_OPENMP
#  pragma omp parallel for
#endif
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
    EQVERB << "CPU-2D assembly" << endl;

    uint8_t* destC = reinterpret_cast< uint8_t* >( destColor );
    uint8_t* destD = reinterpret_cast< uint8_t* >( destDepth );

    const PixelViewport&  pvp    = image->getPixelViewport();
    const int32_t         destX  = offset.x() + pvp.x - destPVP.x;
    const int32_t         destY  = offset.y() + pvp.y - destPVP.y;

    EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));

    const uint8_t*   color = image->getPixelPointer( Frame::BUFFER_COLOR );
    const size_t pixelSize = image->getDepth( Frame::BUFFER_COLOR );
    const size_t rowLength = pvp.w * pixelSize;

#ifdef EQ_USE_OPENMP
#  pragma omp parallel for
#endif
    for( int32_t y = 0; y < pvp.h; ++y )
    {
        const size_t skip = ( (destY + y) * destPVP.w + destX ) * pixelSize;
        memcpy( destC + skip, color + y * pvp.w * pixelSize, rowLength);
        if( destD ) // clear depth, for depth-assembly into existing FB
            bzero( destD + skip, rowLength );
    }
}


void Compositor::_mergeBlendImage( void* dest, const eq::PixelViewport& destPVP,
                                   const Image* image,
                                   const Vector2i& offset )
{
    EQVERB << "CPU-Blend assembly"<< endl;

    int32_t* destColor = reinterpret_cast< int32_t* >( dest );

    const PixelViewport&  pvp    = image->getPixelViewport();
    const int32_t         destX  = offset.x() + pvp.x - destPVP.x;
    const int32_t         destY  = offset.y() + pvp.y - destPVP.y;

    EQASSERT( image->getDepth( Frame::BUFFER_COLOR ) == 4 );
    EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
    EQASSERT( image->hasAlpha( ));
    
#ifdef EQ_USE_PARACOMP_BLEND
    if( pvp == destPVP && offset == eq::Vector2i::ZERO )
    { 
        // Use Paracomp to composite
        if( !_mergeImage_PC( PC_COMP_ALPHA_SORT2_HP, dest, 0, image ))
            EQWARN << "Paracomp compositing failed, using fallback" << endl;
        else
            return; // Go to next input image
    }
#endif

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

#ifdef EQ_USE_OPENMP
#  pragma omp parallel for
#endif
    for( int32_t y = 0; y < pvp.h; ++y )
    {
        const unsigned char* src =
            reinterpret_cast< const uint8_t* >( color + pvp.w * y );
        unsigned char*       dst =
            reinterpret_cast< uint8_t* >( destColorStart + destPVP.w * y );

        for( int32_t x = 0; x < pvp.w; ++x )
        {
            dst[0] = EQ_MIN( src[0] + (src[3]*dst[0] >> 8), 255 );
            dst[1] = EQ_MIN( src[1] + (src[3]*dst[1] >> 8), 255 );
            dst[2] = EQ_MIN( src[2] + (src[3]*dst[2] >> 8), 255 );
            dst[3] =                   src[3]*dst[3] >> 8;

            src += step;
            dst += step;
        }
    }
}

#ifdef EQ_USE_PARACOMP
namespace
{
static unsigned glToPCFormat( const unsigned glFormat, const unsigned glType )
{
    switch( glFormat )
    {
        case GL_RGBA:
        case GL_RGBA8:
            if( glType == GL_UNSIGNED_BYTE )
                return PC_PF_RGBA8;
            break;

        case GL_BGRA:
            if( glType == GL_UNSIGNED_BYTE )
                return PC_PF_BGRA8;
            break;

        case GL_BGR:
            if( glType == GL_UNSIGNED_BYTE )
                return PC_PF_BGR8;
            break;

        case GL_DEPTH_COMPONENT:
            if( glType == GL_FLOAT )
                return PC_PF_Z32F;
            break;
    }

    return 0;
}
}
#endif

bool Compositor::_mergeImage_PC( int operation, void* destColor, 
                                 void* destDepth, const Image* source )
{
#ifdef EQ_USE_PARACOMP
    const unsigned colorFormat = 
        glToPCFormat( source->getFormat( Frame::BUFFER_COLOR ),
                      source->getType(   Frame::BUFFER_COLOR ));

    if( colorFormat == 0 )
    {
        EQWARN << "Format or type of image not supported by Paracomp" << endl;
        return false;
    }

    PCchannel input[2];
    PCchannel output[2];
    
    input[0].pixelFormat  = colorFormat;
    input[0].size         = source->getDepth( Frame::BUFFER_COLOR );
    output[0].pixelFormat = colorFormat;
    output[0].size        = source->getDepth( Frame::BUFFER_COLOR );

    const PixelViewport& pvp = source->getPixelViewport();
    EQASSERT( pvp == source->getPixelViewport( ));

    input[0].xOffset   = 0;
    input[0].yOffset   = 0;
    input[0].width     = pvp.w;
    input[0].height    = pvp.h;
    input[0].rowLength = pvp.w * input[0].size;
    input[0].address   = source->getPixelPointer( Frame::BUFFER_COLOR );

    output[0].xOffset   = 0;
    output[0].yOffset   = 0;
    output[0].width     = pvp.w;
    output[0].height    = pvp.h;
    output[0].rowLength = pvp.w * output[0].size;
    output[0].address   = destColor;
    
    const bool useDepth = ( operation == PC_COMP_DEPTH );
    if( useDepth )
    {
        const unsigned depthFormat = 
            glToPCFormat( source->getFormat( Frame::BUFFER_DEPTH ),
                          source->getType(   Frame::BUFFER_DEPTH ));

        if( depthFormat == 0 )
        {
            EQWARN << "Format or type of image not supported by Paracomp" 
                   << endl;
            return false;
        }

        input[1].pixelFormat  = depthFormat;
        input[1].size         = source->getDepth( Frame::BUFFER_DEPTH );
        output[1].pixelFormat = depthFormat;
        output[1].size        = source->getDepth( Frame::BUFFER_DEPTH );

        input[1].xOffset   = 0;
        input[1].yOffset   = 0;
        input[1].width     = pvp.w;
        input[1].height    = pvp.h;
        input[1].rowLength = pvp.w * input[1].size;
        input[1].address   = source->getPixelPointer( Frame::BUFFER_DEPTH );

        output[1].xOffset   = 0;
        output[1].yOffset   = 0;
        output[1].width     = pvp.w;
        output[1].height    = pvp.h;
        output[1].rowLength = pvp.w * output[1].size;
        output[1].address   = destDepth;
    }


    PCchannel* inputImages[2]     = { output, input };
    PCchannel* outputImage[1]     = { output };
    PCuint     nInputChannels[2]  = { 1, 1 };
    PCuint     nOutputChannels[1] = { 1 };

    if( useDepth )
    {
        nInputChannels[ 0 ]  = 2;
        nInputChannels[ 1 ]  = 2;
        nOutputChannels[ 0 ] = 2;
    }

    const PCerr error = pcCompositeEXT( operation, 
                                        2, nInputChannels, inputImages,
                                        1, nOutputChannels, outputImage );
    if( error != PC_NO_ERROR )
    {
        EQWARN << "Paracomp compositing failed: " << error << endl;
        return false;
    }

    EQINFO << "Paracomp compositing successful" << endl;
    return true;
#else
    return false;
#endif
}


void Compositor::assembleFrame( const Frame* frame, Channel* channel )
{
    const vector< Image* >& images = frame->getImages();
    if( images.empty( ))
        EQINFO << "No images to assemble" << endl;

    ImageOp operation;
    operation.channel = channel;
    operation.buffers = frame->getBuffers();
    operation.offset  = frame->getOffset();
    operation.pixel   = frame->getPixel();
    operation.zoom    = frame->getZoom();

    for( vector< Image* >::const_iterator i = images.begin(); 
         i != images.end(); ++i )
    {
        const Image* image = *i;
        assembleImage( image, operation );
    }
}

void Compositor::assembleImage( const Image* image, const ImageOp& op )
{
    ImageOp operation = op;
    operation.buffers = Frame::BUFFER_NONE;

    if( op.buffers&Frame::BUFFER_COLOR && image->hasData( Frame::BUFFER_COLOR ))
        operation.buffers |= Frame::BUFFER_COLOR;

    if( op.buffers&Frame::BUFFER_DEPTH && image->hasData( Frame::BUFFER_DEPTH ))
        operation.buffers |= Frame::BUFFER_DEPTH;

    if( operation.buffers == Frame::BUFFER_NONE )
    {
        EQWARN << "No image attachment buffers to assemble" << endl;
        return;
    }

    setupStencilBuffer( image, operation );

    if( operation.buffers == Frame::BUFFER_COLOR )
        assembleImage2D( image, operation );
    else if( operation.buffers == ( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH ))
        assembleImageDB( image, operation );
    else
        EQWARN << "Don't know how to assemble using buffers " 
               << operation.buffers << endl;
}

void Compositor::setupStencilBuffer( const Image* image, const ImageOp& op )
{
    if( op.pixel == Pixel::ALL )
        return;

    // mark stencil buffer where pixel shall not pass
    // TODO: OPT!
    glClear( GL_STENCIL_BUFFER_BIT );
    glEnable( GL_STENCIL_TEST );
    glEnable( GL_DEPTH_TEST );

    glStencilFunc( GL_ALWAYS, 1, 1 );
    glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );

    glLineWidth( 1.0f );
    glDepthMask( false );
    glColorMask( false, false, false, false );
    
    const PixelViewport& pvp    = image->getPixelViewport();

    glPixelZoom( static_cast< float >( op.pixel.w ),
                 static_cast< float >( op.pixel.h ));

    if( op.pixel.w > 1 )
    {
        const float width  = static_cast< float >( pvp.w * op.pixel.w );
        const float step   = static_cast< float >( op.pixel.w );

        const float startX = 
            static_cast< float >( op.offset.x() + pvp.x ) + 0.5f - 
            static_cast< float >( op.pixel.w );
        const float endX   = startX + width + op.pixel.w + step;

        const float startY = 
            static_cast< float >( op.offset.y() + pvp.y + op.pixel.y );
        const float endY   = static_cast< float >( startY + pvp.h*op.pixel.h );

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
        const float height = static_cast< float >( pvp.h * op.pixel.h );
        const float step   = static_cast< float >( op.pixel.h );

        const float startX = 
            static_cast< float >( op.offset.x() + pvp.x + op.pixel.x );
        const float endX   = static_cast< float >( startX + pvp.w*op.pixel.w );

        const float startY = 
            static_cast< float >( op.offset.y() + pvp.y ) + 0.5f - 
            static_cast< float >( op.pixel.h );
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
    
    glDisable( GL_DEPTH_TEST );
    glStencilFunc( GL_EQUAL, 0, 1 );
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    
    const ColorMask& colorMask = op.channel->getDrawBufferMask();
    glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
    glDepthMask( true );
}

void Compositor::assembleImage2D( const Image* image, const ImageOp& op )
{
    EQASSERT( image->hasData( Frame::BUFFER_COLOR ));
    _drawPixels( image, op, Frame::BUFFER_COLOR );
}

void Compositor::_drawPixels( const Image* image, 
                              const ImageOp& op,
                              const Frame::Buffer which )
{
    const PixelViewport& pvp = image->getPixelViewport();
    EQLOG( LOG_ASSEMBLY ) << "_drawPixels " << pvp << " offset " << op.offset
                          << endl;
    
    if ( image->getStorageType() == Frame::TYPE_MEMORY )
    {
        EQASSERT( image->hasPixelData( which ));

        if( op.zoom == eq::Zoom::NONE )
        {
            glRasterPos2i( op.offset.x() + pvp.x, op.offset.y() + pvp.y );
            glDrawPixels( pvp.w, pvp.h, 
                          image->getFormat( which ), 
                          image->getType( which ), 
                          image->getPixelPointer( which ));
            return;
        }
        // else use texture with filtering to zoom

        Channel* channel = op.channel; // needed for glewGetContext
        Window::ObjectManager* objects = channel->getObjectManager();

        Texture* texture = objects->obtainEqTexture(
            which == Frame::BUFFER_COLOR ? colorKey : depthKey );

        texture->upload( image, which );
    }
    else // texture image
    {
        EQASSERT( image->hasTextureData( which ));
        image->getTexture( which ).bind();
    }

    if ( which == Frame::BUFFER_COLOR )
        glDepthMask( false );
    else
    {
        EQASSERT( which == Frame::BUFFER_DEPTH )
        glColorMask( false, false, false, false );
    }

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
                     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
                     GL_CLAMP_TO_EDGE );

    if( op.zoom == eq::Zoom::NONE )
    {
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                         GL_NEAREST );
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
                         GL_NEAREST );
    }else
    {
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                         GL_LINEAR );
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
                         GL_LINEAR );
    }

    glColor3f( 1.0f, 1.0f, 1.0f );

    const float startX = static_cast< float >
         ( op.offset.x() + pvp.x * op.pixel.w + op.pixel.x ) * op.zoom.x();
    const float endX   = static_cast< float >
         (op.offset.x() + (pvp.x+pvp.w) * op.pixel.w + op.pixel.x) *op.zoom.x();
    const float startY = static_cast< float >
         ( op.offset.y() + pvp.y * op.pixel.h + op.pixel.y ) * op.zoom.y();
    const float endY   = static_cast< float >
         (op.offset.y() + (pvp.y+pvp.h) * op.pixel.h + op.pixel.y) *op.zoom.y();

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( startX, startY, 0.0f );

        glTexCoord2f( static_cast< float >( pvp.w ), 0.0f );
        glVertex3f( endX, startY, 0.0f );

        glTexCoord2f( static_cast<float>( pvp.w ), static_cast<float>( pvp.h ));
        glVertex3f( endX, endY, 0.0f );
        
        glTexCoord2f( 0.0f, static_cast< float >( pvp.h ));
        glVertex3f( startX, endY, 0.0f );
    glEnd();

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );

    if ( which == Frame::BUFFER_COLOR )
        glDepthMask( true );
    else
    {
        const ColorMask& colorMask = op.channel->getDrawBufferMask();
        glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
    }
}

void Compositor::assembleImageDB( const Image* image, const ImageOp& op )
{
    Channel* channel = op.channel;

    if( GLEW_VERSION_2_0 )
        assembleImageDB_GLSL( image, op );
    else
        assembleImageDB_FF( image, op );
}

void Compositor::assembleImageDB_FF( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImageDB, fixed function " << pvp 
                          << endl;
    EQASSERT( image->hasData( Frame::BUFFER_COLOR ));
    EQASSERT( image->hasData( Frame::BUFFER_DEPTH ));

    // Z-Based sort-last assembly
    glRasterPos2i( op.offset.x() + pvp.x, op.offset.y() + pvp.y );
    glEnable( GL_STENCIL_TEST );

    // test who is in front and mark in stencil buffer
    glEnable( GL_DEPTH_TEST );

    const bool pixelComposite = ( op.pixel != Pixel::ALL );
    if( pixelComposite )
    {   // keep already marked stencil values
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_ZERO, GL_REPLACE );
    }
    else
    {
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );
    }

    _drawPixels( image, op, Frame::BUFFER_DEPTH );

    glDisable( GL_DEPTH_TEST );

    // draw front-most, visible pixels using stencil mask
    glStencilFunc( GL_EQUAL, 1, 1 );
    glStencilOp( GL_KEEP, GL_ZERO, GL_ZERO );

    _drawPixels( image, op, Frame::BUFFER_COLOR );

    glDisable( GL_STENCIL_TEST );
}

void Compositor::assembleImageDB_GLSL( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImageDB, GLSL " << pvp 
                          << endl;

    Channel*               channel = op.channel; // needed for glewGetContext
    Window::ObjectManager* objects = channel->getObjectManager();

    GLuint program = objects->getProgram( shaderDBKey );

    if( program == Window::ObjectManager::INVALID )
    {
        // Create fragment shader which reads color and depth values from 
        // rectangular textures
        const GLuint shader = objects->newShader( shaderDBKey, 
                                                  GL_FRAGMENT_SHADER );
        EQASSERT( shader != Window::ObjectManager::INVALID );

        const char* source =
            "uniform sampler2DRect color; \
             uniform sampler2DRect depth; \
             void main(void){ \
                 gl_FragColor = texture2DRect( color, gl_TexCoord[0].st ); \
                 gl_FragDepth = texture2DRect( depth, gl_TexCoord[0].st ).x; }";

        EQ_GL_CALL( glShaderSource( shader, 1, &source, 0 ));
        EQ_GL_CALL( glCompileShader( shader ));

        GLint status;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
        if( !status )
            EQERROR << "Failed to compile fragment shader for DB compositing" 
                    << endl;

        program = objects->newProgram( shaderDBKey );

        EQ_GL_CALL( glAttachShader( program, shader ));
        EQ_GL_CALL( glLinkProgram( program ));

        glGetProgramiv( program, GL_LINK_STATUS, &status );
        if( !status )
        {
            EQWARN << "Failed to link shader program for DB compositing" 
                   << endl;
            return;
        }

        // use fragment shader and setup uniforms
        EQ_GL_CALL( glUseProgram( program ));
        
        const GLint depthParam = glGetUniformLocation( program, "depth" );
        glUniform1i( depthParam, 0 );
        const GLint colorParam = glGetUniformLocation( program, "color" );
        glUniform1i( colorParam, 1 );
    }
    else
    {
        // use fragment shader
        EQ_GL_CALL( glUseProgram( program ));
    }

    // Enable & download color and depth textures
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE1 ));
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_NEAREST );

    const bool useImageTexture = image->getStorageType() == Frame::TYPE_TEXTURE;
    if( useImageTexture )
        image->getTexture( Frame::BUFFER_COLOR ).bind();
    else
    {
        Texture* texture = objects->obtainEqTexture( colorDBKey );
        texture->upload( image, Frame::BUFFER_COLOR );
    }

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE0 ));
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_NEAREST );

    if( useImageTexture )
        image->getTexture( Frame::BUFFER_DEPTH ).bind();
    else
    {
        Texture* texture = objects->obtainEqTexture( depthDBKey );
        texture->upload( image, Frame::BUFFER_DEPTH );
    }

    // Draw a quad using shader & textures in the right place
    glEnable( GL_DEPTH_TEST );
    glColor3f( 1.0f, 1.0f, 1.0f );

    const float startX = static_cast< float >
        ( op.offset.x() + pvp.x * op.pixel.w + op.pixel.x );
    const float endX   = static_cast< float >
        ( op.offset.x() + (pvp.x + pvp.w) * op.pixel.w + op.pixel.x );
    const float startY = static_cast< float >
        ( op.offset.y() + pvp.y * op.pixel.h + op.pixel.y );
    const float endY   = static_cast< float >
        ( op.offset.y() + (pvp.y + pvp.h) * op.pixel.h + op.pixel.y );
    
    const float w = static_cast< float >( pvp.w );
    const float h = static_cast< float >( pvp.h );

    glBegin( GL_TRIANGLE_STRIP );
    glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 0.0f );
    glMultiTexCoord2f( GL_TEXTURE1, 0.0f, 0.0f );
    glVertex3f( startX, startY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, w, 0.0f );
    glMultiTexCoord2f( GL_TEXTURE1, w, 0.0f );
    glVertex3f( endX, startY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, 0.0f, h );
    glMultiTexCoord2f( GL_TEXTURE1, 0.0f, h );
    glVertex3f( startX, endY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, w, h );
    glMultiTexCoord2f( GL_TEXTURE1, w, h );
    glVertex3f( endX, endY, 0.0f );

    glEnd();

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDisable( GL_DEPTH_TEST );
    EQ_GL_CALL( glUseProgram( 0 ));
}


}
