
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include <eq/base/perThread.h>

#include "compositor.h"

#include "channel.h"
#include "channelStatistics.h"

#include "log.h"
#include "image.h"
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
static const char glslKey = 42;

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
        if( frame->getPixel() != Pixel::ALL ) // Not supported yet on the CPU
            return false;

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
                                 Channel* channel )
{
    if( frames.empty( ))
        return;

    if( _useCPUAssembly( frames, channel ))
        assembleFramesCPU( frames, channel );
    else
        assembleFramesUnsorted( frames, channel );
}

void Compositor::assembleFramesSorted( const FrameVector& frames,
                                       Channel* channel, const bool blendAlpha )
{
    if( frames.empty( ))
        return;

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
            assembleFrame( frame, channel );
        }
    }

    if( blendAlpha )
        glDisable( GL_BLEND );
}

void Compositor::assembleFramesUnsorted( const FrameVector& frames, 
                                         Channel* channel )
{
    if( frames.empty( ))
        return;

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

            assembleFrame( frame, channel );
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

    // collect and categorize input images
    vector< FrameImage > imagesDB;
    vector< FrameImage > images2D;
    vector< FrameImage > imagesBlend;
    PixelViewport        resultPVP;

    int colorFormat = GL_BGRA;
    int colorType   = GL_UNSIGNED_BYTE;
    int depthFormat = GL_DEPTH_COMPONENT;
    int depthType   = GL_FLOAT;

    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->waitReady();

        EQASSERTINFO( frame->getPixel() == Pixel::ALL,
                      "CPU-based pixel recomposition not implemented" );

        const vector< Image* >& images = frame->getImages();        
        for( vector< Image* >::const_iterator j = images.begin(); 
             j != images.end(); ++j )
        {
            const Image* image = *j;
            EQASSERT( image->getStorageType() == Frame::TYPE_MEMORY );

            if( !image->hasPixelData( Frame::BUFFER_COLOR ))
                continue;

            resultPVP.merge( image->getPixelViewport() + frame->getOffset( ));
            
            colorFormat = image->getFormat( Frame::BUFFER_COLOR );
            colorType   = image->getType( Frame::BUFFER_COLOR );

            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
            {
                imagesDB.push_back( FrameImage( frame, image ));
                depthFormat = image->getFormat( Frame::BUFFER_DEPTH );
                depthType   = image->getType( Frame::BUFFER_DEPTH );
            }
            else if( blendAlpha && image->hasAlpha( ))
                imagesBlend.push_back( FrameImage( frame, image ));
            else
                images2D.push_back( FrameImage( frame, image ));
        }
    }

    if( !resultPVP.hasArea( ))
    {
        EQWARN << "Nothing to assemble: " << resultPVP << endl;
        return 0;
    }

    // prepare output image
    Image* result = _resultImage.get();
    if( !result )
    {
        _resultImage = new ResultImage;
        result       = _resultImage.get();;
    }

    result->setFormat( Frame::BUFFER_COLOR, colorFormat );
    result->setType(   Frame::BUFFER_COLOR, colorType );
    result->setFormat( Frame::BUFFER_DEPTH, depthFormat );
    result->setType(   Frame::BUFFER_DEPTH, depthType );
    result->setPixelViewport( resultPVP );
    result->clearPixelData( Frame::BUFFER_COLOR );
    if( !imagesDB.empty( ))
        result->clearPixelData( Frame::BUFFER_DEPTH );

    // assembly
    _assembleDBImages( result, imagesDB );
    _assemble2DImages( result, images2D );
    _assembleBlendImages( result, imagesBlend );

    return result;
}


void Compositor::_assembleDBImages( Image* result, 
                                    const std::vector< FrameImage >& images )
{
    if( images.empty( ))
        return;

    EQVERB << "CPU-DB assembly of " << images.size() << " images" << endl;

    EQASSERT( result->getFormat( Frame::BUFFER_DEPTH ) == GL_DEPTH_COMPONENT);
    EQASSERT( result->getType( Frame::BUFFER_DEPTH )   == GL_FLOAT );

    const eq::PixelViewport resultPVP = result->getPixelViewport();
    uint32_t* destColor = reinterpret_cast< uint32_t* >
        ( result->getPixelPointer( Frame::BUFFER_COLOR ));
    float*    destDepth = reinterpret_cast< float* >
        ( result->getPixelPointer( Frame::BUFFER_DEPTH ));

    for( vector< FrameImage >::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const Frame*          frame  = i->first;
        const Image*          image  = i->second;
        const vmml::Vector2i& offset = frame->getOffset();
        const PixelViewport&  pvp    = image->getPixelViewport();

        EQASSERT( image->getDepth( Frame::BUFFER_COLOR ) == 4 );
        EQASSERT( image->getDepth( Frame::BUFFER_DEPTH ) == 4 );
        EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
        EQASSERT( image->hasPixelData( Frame::BUFFER_DEPTH ));
        EQASSERTINFO( image->getFormat( Frame::BUFFER_COLOR ) ==
                      result->getFormat( Frame::BUFFER_COLOR ),
                      image->getFormat( Frame::BUFFER_COLOR ) << " != "
                      << result->getFormat( Frame::BUFFER_COLOR ));
        EQASSERT( image->getType( Frame::BUFFER_COLOR ) ==
                  result->getType( Frame::BUFFER_COLOR ));
        EQASSERT( image->getFormat( Frame::BUFFER_DEPTH ) ==
                  result->getFormat( Frame::BUFFER_DEPTH ));
        EQASSERT( image->getType( Frame::BUFFER_DEPTH ) ==
                  result->getType( Frame::BUFFER_DEPTH ));

#ifdef EQ_USE_PARACOMP_DEPTH
        if( pvp == resultPVP && offset == vmml::Vector2i::ZERO )
        {
            // Use Paracomp to composite
            if( !_assembleImage_PC( PC_COMP_DEPTH, result, image ))
                EQWARN << "Paracomp compositing failed, using fallback" << endl;
            else
                continue; // Go to next input image
        }
#endif

        const int32_t         destX  = offset.x + pvp.x - resultPVP.x;
        const int32_t         destY  = offset.y + pvp.y - resultPVP.y;

        const uint32_t* color = reinterpret_cast< const uint32_t* >
            ( image->getPixelPointer( Frame::BUFFER_COLOR ));
        const float*   depth = reinterpret_cast< const float* >
            ( image->getPixelPointer( Frame::BUFFER_DEPTH ));

#  pragma omp parallel for
        for( int32_t y = 0; y < pvp.h; ++y )
        {
            const uint32_t skip =  (destY + y) * resultPVP.w + destX;
            EQASSERT( skip * sizeof( int32_t ) <= 
                      result->getPixelDataSize( Frame::BUFFER_COLOR ));
            EQASSERT( skip * sizeof( float ) <= 
                      result->getPixelDataSize( Frame::BUFFER_DEPTH ));

            uint32_t*   destColorIt = destColor + skip;
            float*      destDepthIt = destDepth + skip;
            const uint32_t* colorIt = color + y * pvp.w;
            const float*    depthIt = depth + y * pvp.w;

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
}

void Compositor::_assemble2DImages( Image* result, 
                                    const std::vector< FrameImage >& images )
{
    // This is mostly copy&paste code from _assembleDBImages :-/
    if( images.empty( ))
        return;

    EQVERB << "CPU-2D assembly of " << images.size() << " images" << endl;

    const eq::PixelViewport resultPVP = result->getPixelViewport();
    uint8_t* destColor = result->getPixelPointer( Frame::BUFFER_COLOR );
    uint8_t* destDepth = result->hasPixelData( Frame::BUFFER_DEPTH ) ?
        reinterpret_cast< uint8_t* >(result->getPixelPointer( Frame::BUFFER_DEPTH))
        : 0;

    for( vector< FrameImage >::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const Frame*          frame  = i->first;
        const Image*          image  = i->second;
        const vmml::Vector2i& offset = frame->getOffset();
        const PixelViewport&  pvp    = image->getPixelViewport();
        const int32_t         destX  = offset.x + pvp.x - resultPVP.x;
        const int32_t         destY  = offset.y + pvp.y - resultPVP.y;

        EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
        EQASSERT( image->getFormat( Frame::BUFFER_COLOR ) ==
                  result->getFormat( Frame::BUFFER_COLOR ));
        EQASSERT( image->getType( Frame::BUFFER_COLOR ) ==
                  result->getType( Frame::BUFFER_COLOR ));

        const uint8_t*   color = image->getPixelPointer( Frame::BUFFER_COLOR );
        const size_t pixelSize = image->getDepth( Frame::BUFFER_COLOR );
        const size_t rowLength = pvp.w * pixelSize;

#  pragma omp parallel for
        for( int32_t y = 0; y < pvp.h; ++y )
        {
            const size_t skip = ( (destY + y) * resultPVP.w + destX ) *
                                    pixelSize;
            EQASSERT( skip + rowLength <= 
                      result->getPixelDataSize( Frame::BUFFER_COLOR ));

            memcpy( destColor + skip, color + y * pvp.w * pixelSize, rowLength);
            if( destDepth )
            {
                EQASSERT( pixelSize == image->getDepth( Frame::BUFFER_DEPTH ));
                bzero( destDepth + skip, rowLength );
            }
        }
    }
}


void Compositor::_assembleBlendImages( Image* result, 
                                       const std::vector< FrameImage >& images )
{
    if( images.empty( ))
        return;

    EQVERB << "CPU-Blend assembly of " << images.size() <<" images"<< endl;

    const eq::PixelViewport resultPVP = result->getPixelViewport();
    int32_t* destColor = reinterpret_cast< int32_t* >
        ( result->getPixelPointer( Frame::BUFFER_COLOR ));

    for( vector< FrameImage >::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const Frame*          frame  = i->first;
        const Image*          image  = i->second;
        const vmml::Vector2i& offset = frame->getOffset();
        const PixelViewport&  pvp    = image->getPixelViewport();
        const int32_t         destX  = offset.x + pvp.x - resultPVP.x;
        const int32_t         destY  = offset.y + pvp.y - resultPVP.y;

        EQASSERT( image->getDepth( Frame::BUFFER_COLOR ) == 4 );
        EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
        EQASSERT( image->hasAlpha( ));
        EQASSERT( image->getFormat( Frame::BUFFER_COLOR ) ==
                  result->getFormat( Frame::BUFFER_COLOR ));
        EQASSERT( image->getType( Frame::BUFFER_COLOR ) ==
                  result->getType( Frame::BUFFER_COLOR ));

#ifdef EQ_USE_PARACOMP_BLEND
        if( pvp == resultPVP && offset == vmml::Vector2i::ZERO )
        { 
            // Use Paracomp to composite
            if( !_assembleImage_PC( PC_COMP_ALPHA_SORT2_HP,result, image ))
                EQWARN << "Paracomp compositing failed, using fallback" << endl;
            else
                continue; // Go to next input image
        }
#endif

        const int32_t* color = reinterpret_cast< const int32_t* >
                                ( image->getPixelPointer( Frame::BUFFER_COLOR ));

        // Check if we have enought space
        EQASSERT( ((destY+pvp.h-1)*resultPVP.w+destX+pvp.w)*sizeof(uint32_t) <=
                  result->getPixelDataSize( Frame::BUFFER_COLOR ));


        // Blending of two slices, none of which is on final image (i.e. result
        // could be blended on to something else) should be performed with:
        // glBlendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA )
        // which means:
        // dstColor = 1*srcColor + srcAlpha*dstColor
        // dstAlpha = 0*srcAlpha + srcAlpha*dstAlpha
        // because we accumulate light which is go through (= 1-Alpha) and we
        // already have colors as Alpha*Color

        const int32_t* colorIt     = color;
        int32_t*       destColorIt = destColor + destY*resultPVP.w + destX;
        const uint32_t step        = sizeof( int32_t );

#  pragma omp parallel for
        for( int32_t y = 0; y < pvp.h; ++y )
        {
            const unsigned char* src =
                reinterpret_cast< const unsigned char* >( colorIt );
            unsigned char*       dst =
                reinterpret_cast< unsigned char* >( destColorIt );

            for( int32_t x = 0; x < pvp.w; ++x )
            {
                dst[0] = EQ_MIN( src[0] + (src[3]*dst[0] >> 8), 255 );
                dst[1] = EQ_MIN( src[1] + (src[3]*dst[1] >> 8), 255 );
                dst[2] = EQ_MIN( src[2] + (src[3]*dst[2] >> 8), 255 );
                dst[3] =                   src[3]*dst[3] >> 8;

                src += step;
                dst += step;
            }
            colorIt     += pvp.w;
            destColorIt += resultPVP.w;
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

bool Compositor::_assembleImage_PC( int operation, Image* result, 
                                    const Image* source )
{
#ifdef EQ_USE_PARACOMP
    const unsigned colorFormat = 
        glToPCFormat( result->getFormat( Frame::BUFFER_COLOR ),
                      result->getType(   Frame::BUFFER_COLOR ));

    if( colorFormat == 0 )
    {
        EQWARN << "Format or type of image not supported by Paracomp" << endl;
        return false;
    }

    PCchannel input[2];
    PCchannel output[2];
    
    input[0].pixelFormat  = colorFormat;
    input[0].size         = result->getDepth( Frame::BUFFER_COLOR );
    output[0].pixelFormat = colorFormat;
    output[0].size        = result->getDepth( Frame::BUFFER_COLOR );

    const PixelViewport& pvp = source->getPixelViewport();
    EQASSERT( pvp == result->getPixelViewport( ));

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
    output[0].address   = result->getPixelPointer( Frame::BUFFER_COLOR );

    
    const bool useDepth = ( operation == PC_COMP_DEPTH );
    if( useDepth )
    {
        const unsigned depthFormat = 
            glToPCFormat( result->getFormat( Frame::BUFFER_DEPTH ),
                          result->getType(   Frame::BUFFER_DEPTH ));

        if( depthFormat == 0 )
        {
            EQWARN << "Format or type of image not supported by Paracomp" 
                   << endl;
            return false;
        }

        input[1].pixelFormat  = depthFormat;
        input[1].size         = result->getDepth( Frame::BUFFER_DEPTH );
        output[1].pixelFormat = depthFormat;
        output[1].size        = result->getDepth( Frame::BUFFER_DEPTH );

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
        output[1].address   = result->getPixelPointer( Frame::BUFFER_DEPTH );
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
        EQWARN << "No images to assemble" << endl;

    ImageOp operation;
    operation.channel = channel;
    operation.buffers = frame->getBuffers();
    operation.offset  = frame->getOffset();
    operation.pixel   = frame->getPixel();

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

    if( op.buffers & Frame::BUFFER_COLOR && 
        image->hasPixelData( Frame::BUFFER_COLOR ))

        operation.buffers |= Frame::BUFFER_COLOR;

    if( op.buffers & Frame::BUFFER_DEPTH &&
        image->hasPixelData( Frame::BUFFER_DEPTH ))

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
            static_cast< float >( op.offset.x + pvp.x ) + 0.5f - 
            static_cast< float >( op.pixel.w );
        const float endX   = startX + width + op.pixel.w + step;

        const float startY = 
            static_cast< float >( op.offset.y + pvp.y + op.pixel.y );
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
            static_cast< float >( op.offset.x + pvp.x + op.pixel.x );
        const float endX   = static_cast< float >( startX + pvp.w*op.pixel.w );

        const float startY = 
            static_cast< float >( op.offset.y + pvp.y ) + 0.5f - 
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
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImage2D " << pvp << " offset " 
                          << op.offset << endl;
    EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));

    glRasterPos2i( op.offset.x + pvp.x, op.offset.y + pvp.y );

    _drawPixels( image, op, Frame::BUFFER_COLOR );
}

void Compositor::_drawPixels( const Image* image, 
                              const ImageOp& op,
                              const Frame::Buffer which )
{

    const PixelViewport& pvp = image->getPixelViewport();
    
    if ( image->getStorageType() == Frame::TYPE_MEMORY )
    {
        glDrawPixels( pvp.w, pvp.h, 
                     image->getFormat( which ), 
                     image->getType( which ), 
                     image->getPixelPointer( which ));
        return;
    }

    // else texture
    GLuint textureId;
    GLenum internalformat;
        
    if ( which == Frame::BUFFER_COLOR )
    {
         textureId = image->getColorTexture();
         internalformat = GL_RGBA;
    }
    else
    {
         glColorMask( false, false, false, false );
         textureId = image->getDepthTexture();
         internalformat = GL_DEPTH_COMPONENT;
    }

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, textureId );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
                     GL_NEAREST );

    glColor3f( 1.0f, 1.0f, 1.0f );

    const float startX = static_cast< float >
         ( op.offset.x + pvp.x * op.pixel.w + op.pixel.x );
    const float endX   = static_cast< float >
         ( op.offset.x + (pvp.x + pvp.w) * op.pixel.w + op.pixel.x );
    const float startY = static_cast< float >
         ( op.offset.y + pvp.y * op.pixel.h + op.pixel.y );
    const float endY   = static_cast< float >
         ( op.offset.y + (pvp.y + pvp.h) * op.pixel.h + op.pixel.y );

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( startX, startY, 0.0f );

        glTexCoord2f( pvp.w, 0.0f );
        glVertex3f( endX, startY, 0.0f );

        glTexCoord2f( pvp.w, pvp.h );
        glVertex3f( endX, endY, 0.0f );
        
        glTexCoord2f( 0.0f, pvp.h );
        glVertex3f( startX, endY, 0.0f );
    glEnd();

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );

    if( which == Frame::BUFFER_COLOR ) 
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
    EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
    EQASSERT( image->hasPixelData( Frame::BUFFER_DEPTH ));

    // Z-Based sort-last assembly
    glRasterPos2i( op.offset.x + pvp.x, op.offset.y + pvp.y );
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
    Window*                window  = channel->getWindow();
    Window::ObjectManager* objects = window->getObjectManager();
    
    // use per-window textures: using a shared texture across multiple windows
    // creates undefined texture contents, since each context has it's own
    // command buffer which is flushed unpredictably (at least on Darwin). A
    // glFlush at the end of this method would do it too, but the performance
    // use case is one window per pipe, hence we'll optimize for that and remove
    // the glFlush.
    const char*            key     = reinterpret_cast< char* >( window );

    GLuint depthTexture;
    GLuint colorTexture;

    if ( image->getStorageType() == Frame::TYPE_TEXTURE )
    {
        depthTexture = image->getDepthTexture();
        colorTexture = image->getColorTexture();
    }
    else
    {
        depthTexture = objects->obtainTexture( key );
        colorTexture = objects->obtainTexture( key+1 );
    }

    GLuint program      = objects->getProgram( &glslKey );

    if( program == Window::ObjectManager::FAILED )
    {
        // Create fragment shader which reads color and depth values from 
        // rectangular textures
        const GLuint shader = objects->newShader( &glslKey, GL_FRAGMENT_SHADER);
        EQASSERT( shader != Window::ObjectManager::FAILED );

        const char* source = "uniform sampler2DRect color; uniform sampler2DRect depth; void main(void){ gl_FragColor = texture2DRect( color, gl_TexCoord[0].st ); gl_FragDepth = texture2DRect( depth, gl_TexCoord[0].st ).x; }";

        EQ_GL_CALL( glShaderSource( shader, 1, &source, 0 ));
        EQ_GL_CALL( glCompileShader( shader ));

        GLint status;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
        if( !status )
            EQERROR << "Failed to compile fragment shader for DB compositing" 
                    << endl;

        program = objects->newProgram( &glslKey );

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

        // make sure the other shared context see the program
        glFlush();
    }
    else
        // use fragment shader
        EQ_GL_CALL( glUseProgram( program ));

    // Enable & download color and depth textures
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE1 ));
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_RECTANGLE_ARB, colorTexture ));
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_NEAREST );

    if ( image->getStorageType() != Frame::TYPE_TEXTURE )
    {
        EQ_GL_CALL( glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                                  GL_RGBA,
                                  pvp.w, pvp.h, 0,
                                  image->getFormat( Frame::BUFFER_COLOR ), 
                                  image->getType( Frame::BUFFER_COLOR ),
                                  image->getPixelPointer( Frame::BUFFER_COLOR )
                                  ));
    }

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE0 ));
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_RECTANGLE_ARB, depthTexture ));
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_NEAREST );

    if ( image->getStorageType() != Frame::TYPE_TEXTURE )
    {
        EQ_GL_CALL( glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                                  GL_DEPTH_COMPONENT32_ARB,
                                  pvp.w, pvp.h, 0,
                                  image->getFormat( Frame::BUFFER_DEPTH ), 
                                  image->getType( Frame::BUFFER_DEPTH ),
                                  image->getPixelPointer( Frame::BUFFER_DEPTH )));
    }

    // Draw a quad using shader & textures in the right place
    glEnable( GL_DEPTH_TEST );
    glColor3f( 1.0f, 1.0f, 1.0f );

    const float startX = static_cast< float >
        ( op.offset.x + pvp.x * op.pixel.w + op.pixel.x );
    const float endX   = static_cast< float >
        ( op.offset.x + (pvp.x + pvp.w) * op.pixel.w + op.pixel.x );
    const float startY = static_cast< float >
        ( op.offset.y + pvp.y * op.pixel.h + op.pixel.y );
    const float endY   = static_cast< float >
        ( op.offset.y + (pvp.y + pvp.h) * op.pixel.h + op.pixel.y );

    glBegin( GL_TRIANGLE_STRIP );
    glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 0.0f );
    glMultiTexCoord2f( GL_TEXTURE1, 0.0f, 0.0f );
    glVertex3f( startX, startY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, pvp.w, 0.0f );
    glMultiTexCoord2f( GL_TEXTURE1, pvp.w, 0.0f );
    glVertex3f( endX, startY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, 0.0f, pvp.h );
    glMultiTexCoord2f( GL_TEXTURE1, 0.0f, pvp.h );
    glVertex3f( startX, endY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, pvp.w, pvp.h );
    glMultiTexCoord2f( GL_TEXTURE1, pvp.w, pvp.h );
    glVertex3f( endX, endY, 0.0f );

    glEnd();

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDisable( GL_DEPTH_TEST );
    EQ_GL_CALL( glUseProgram( 0 ));
}


}
