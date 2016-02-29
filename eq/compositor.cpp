
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "imageOp.h"
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

struct CPUAssemblyFormat
{
    CPUAssemblyFormat( const bool blend_ )
        : colorInt(0), colorExt(0), depthInt(0), depthExt(0), blend( blend_ ) {}

    uint32_t colorInt;
    uint32_t colorExt;
    uint32_t depthInt;
    uint32_t depthExt;
    const bool blend;
};

bool _useCPUAssembly( const Image* image, CPUAssemblyFormat& format )
{
    const bool hasColor = image->hasPixelData( Frame::BUFFER_COLOR );
    const bool hasDepth = image->hasPixelData( Frame::BUFFER_DEPTH );

    if( // Not an alpha-blending compositing
        ( !format.blend || !hasColor || !image->hasAlpha( )) &&
        // and not a depth-sorting compositing
        ( !hasColor || !hasDepth ))
    {
        return false;
    }

    if( format.colorInt == 0 )
        format.colorInt = image->getInternalFormat( Frame::BUFFER_COLOR );
    if( format.colorExt == 0 )
        format.colorExt = image->getExternalFormat( Frame::BUFFER_COLOR );

    if( format.colorInt != image->getInternalFormat( Frame::BUFFER_COLOR ) ||
        format.colorExt != image->getExternalFormat( Frame::BUFFER_COLOR ))
    {
        return false;
    }

    switch( format.colorExt )
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

    if( !hasDepth )
        return true;

    if( format.depthInt == 0 )
        format.depthInt = image->getInternalFormat( Frame::BUFFER_DEPTH );
    if( format.depthExt == 0 )
        format.depthExt = image->getExternalFormat( Frame::BUFFER_DEPTH );

    if( format.depthInt != image->getInternalFormat( Frame::BUFFER_DEPTH ) ||
        format.depthExt != image->getExternalFormat( Frame::BUFFER_DEPTH ))
    {
        return false;
    }

    return format.depthExt == EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
}

bool _useCPUAssembly( const Frames& frames, Channel* channel,
                      const bool blend = false )
{
    // It doesn't make sense to use CPU-assembly for only one frame
    if( frames.size() < 2 )
        return false;

    // Test that the input frames have color and depth buffers or that
    // alpha-blended assembly is used with multiple RGBA buffers. We assume then
    // that we will have at least one image per frame so most likely it's worth
    // to wait for the images and to do a CPU-based assembly. Also test early
    // for unsupported decomposition modes
    const uint32_t desiredBuffers = blend ? Frame::BUFFER_COLOR :
                                    Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH;
    for( const Frame* frame : frames )
    {
        const RenderContext& context = frame->getFrameData()->getContext();

        if( frame->getBuffers() != desiredBuffers ||
            context.pixel != Pixel::ALL || context.subPixel != SubPixel::ALL ||
            frame->getFrameData()->getZoom() != Zoom::NONE ||
            frame->getZoom() != Zoom::NONE ) // Not supported by CPU compositor
        {
            return false;
        }
    }

    // Wait for all images to be ready and test if our assumption was correct,
    // that there are enough images to make a CPU-based assembly worthwhile and
    // all other preconditions for our CPU-based assembly code are true.
    size_t nImages = 0;
    const uint32_t timeout = channel->getConfig()->getTimeout();
    CPUAssemblyFormat format( blend );

    for( const Frame* frame : frames )
    {
        {
            ChannelStatistics event( Statistic::CHANNEL_FRAME_WAIT_READY,
                                     channel );
            frame->waitReady( timeout );
        }

        const Images& images = frame->getImages();
        for( const Image* image : images )
        {
            if( !_useCPUAssembly( image, format ))
                return false;
            ++nImages;
        }
    }
    return (nImages > 1);
}

bool _useCPUAssembly( const ImageOps& ops, const bool blend )
{
    CPUAssemblyFormat format( blend );
    size_t nImages = 0;

    for( const ImageOp& op : ops )
    {
        if( !_useCPUAssembly( op.image, format ))
            return false;
        ++nImages;
    }
    return (nImages > 1);
}

uint32_t _assembleCPUImage( const Image* image, Channel* channel )
{
    if( !image )
        return 0;

    // assemble result on dest channel
    ImageOp operation;
    operation.image = image;
    operation.buffers = Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH;
    Compositor::assembleImage( operation, channel );

#if 0
    static uint32_t counter = 0;
    std::ostringstream stringstream;
    stringstream << "Image_" << ++counter;
    image->writeImages( stringstream.str( ));
#endif

    return 1;
}

void _collectOutputData( const PixelData& pixelData, uint32_t& internalFormat,
                         uint32_t& pixelSize, uint32_t& externalFormat )
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

bool _collectOutputData( const ImageOps& ops, PixelViewport& destPVP,
                         uint32_t& colorInt, uint32_t& colorPixelSize,
                         uint32_t& colorExt, uint32_t& depthInt,
                         uint32_t& depthPixelSize, uint32_t& depthExt )
{
    for( const ImageOp& op : ops )
    {
        const RenderContext& context = op.image->getContext();
        if( context.pixel != Pixel::ALL || context.subPixel != SubPixel::ALL ||
            op.zoom != Zoom::NONE ||
            op.image->getStorageType() != Frame::TYPE_MEMORY )
        {
            return false;
        }

        if( !op.image->hasPixelData( Frame::BUFFER_COLOR ))
            continue;

        destPVP.merge( op.image->getPixelViewport() + op.offset );

        _collectOutputData( op.image->getPixelData( Frame::BUFFER_COLOR ),
                            colorInt, colorPixelSize, colorExt );

        if( op.image->hasPixelData( Frame::BUFFER_DEPTH ))
        {
            _collectOutputData( op.image->getPixelData( Frame::BUFFER_DEPTH ),
                                depthInt, depthPixelSize, depthExt );
        }
    }

    if( !destPVP.hasArea( ))
        LBWARN << "Nothing to assemble: " << destPVP << std::endl;
    return destPVP.hasArea();
}

void _mergeDBImage( void* destColor, void* destDepth,
                    const PixelViewport& destPVP, const Image* image,
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

void _merge2DImage( void* destColor, void* destDepth,
                    const eq::PixelViewport& destPVP, const Image* image,
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

void _blendImage( void* dest, const eq::PixelViewport& destPVP,
                  const Image* image, const Vector2i& offset )
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

void _mergeImages( const ImageOps& ops, const bool blend, void* colorBuffer,
                   void* depthBuffer, const PixelViewport& destPVP )
{
    for( const ImageOp& op : ops )
    {
        if( !op.image->hasPixelData( Frame::BUFFER_COLOR ))
            continue;

        if( op.image->hasPixelData( Frame::BUFFER_DEPTH ))
            _mergeDBImage( colorBuffer, depthBuffer, destPVP, op.image,
                           op.offset );
        else if( blend && op.image->hasAlpha( ))
            _blendImage( colorBuffer, destPVP, op.image, op.offset );
        else
            _merge2DImage( colorBuffer, depthBuffer, destPVP, op.image,
                           op.offset );
    }
}

Vector4f _getCoords( const ImageOp& op, const PixelViewport& pvp )
{
    const Pixel& pixel = op.image->getContext().pixel;
    return Vector4f(
        op.offset.x() + pvp.x * pixel.w + pixel.x,
        op.offset.x() + pvp.getXEnd() * pixel.w * op.zoom.x() + pixel.x,
        op.offset.y() + pvp.y * pixel.h + pixel.y,
        op.offset.y() + pvp.getYEnd() * pixel.h * op.zoom.y() + pixel.y );
}

bool _setupDrawPixels( const ImageOp& op, const Frame::Buffer which,
                       Channel* channel )
{
    const PixelViewport& pvp = op.image->getPixelViewport();
    const util::Texture* texture = 0;
    if( op.image->getStorageType() == Frame::TYPE_MEMORY )
    {
        LBASSERT( op.image->hasPixelData( which ));
        util::ObjectManager& objects = channel->getObjectManager();

        const bool coreProfile = channel->getWindow()->getIAttribute(
                    WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
        if( op.zoom == Zoom::NONE && !coreProfile )
        {
            op.image->upload( which, 0, op.offset, objects );
            return false;
        }
        util::Texture* ncTexture = objects.obtainEqTexture(
            which == Frame::BUFFER_COLOR ? colorDBKey : depthDBKey,
            GL_TEXTURE_RECTANGLE_ARB );
        texture = ncTexture;

        const Vector2i offset( -pvp.x, -pvp.y ); // will be applied with quad
        op.image->upload( which, ncTexture, offset, objects );
    }
    else // texture image
    {
        LBASSERT( op.image->hasTextureData( which ));
        texture = &op.image->getTexture( which );
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

void _drawPixelsFF( const ImageOp& op, const Frame::Buffer which,
                    Channel* channel )
{
    const PixelViewport& pvp = op.image->getPixelViewport();
    LBLOG( LOG_ASSEMBLY ) << "_drawPixelsFF " << pvp << " offset " << op.offset
                          << std::endl;

    if( !_setupDrawPixels( op, which, channel ))
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
        const ColorMask& colorMask = channel->getDrawBufferMask();
        EQ_GL_CALL( glColorMask( colorMask.red, colorMask.green, colorMask.blue,
                                 true ));
    }
}

template< typename T >
void _drawTexturedQuad( const T* key, const ImageOp& op,
                        const PixelViewport& pvp, const bool withDepth,
                        Channel* channel )
{
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

    const GLfloat uvs[] = { 0.0f, 0.0f,
                            float( pvp.w ), 0.0f,
                            0.0f, float( pvp.h ),
                            float( pvp.w ), float( pvp.h ) };
    const eq::Matrix4f& proj =
        eq::Frustumf( channel->getPixelViewport().x,
                      channel->getPixelViewport().getXEnd(),
                      channel->getPixelViewport().y,
                      channel->getPixelViewport().getYEnd(),
                      -1.0f, 1.0f ).computeOrthoMatrix();
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

void _drawPixelsGLSL( const ImageOp& op, const Frame::Buffer which,
                      Channel* channel )
{
    const PixelViewport& pvp = op.image->getPixelViewport();
    LBLOG( LOG_ASSEMBLY ) << "_drawPixelsGLSL " << pvp << " offset "
                          << op.offset << std::endl;

    if( !_setupDrawPixels( op, which, channel ))
        return;

    _drawTexturedQuad( channel, op, pvp, false, channel );

    // restore state
    if ( which == Frame::BUFFER_COLOR )
        EQ_GL_CALL( glDepthMask( true ))
    else
    {
        const ColorMask& colorMask = channel->getDrawBufferMask();
        EQ_GL_CALL( glColorMask( colorMask.red, colorMask.green, colorMask.blue,
                                 true ));
    }
}

util::Accum* _obtainAccum( Channel* channel )
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

uint32_t Compositor::blendImages( const ImageOps& ops, Channel* channel,
                                  util::Accum* accum )
{
    if( ops.empty( ))
        return 0;

    if( isSubPixelDecomposition( ops ))
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

            const SubPixel& subPixel = ops.front().image->getContext().subPixel;
            accum->setTotalSteps( subPixel.size );
        }

        uint32_t count = 0;
        ImageOps opsLeft = ops;
        while( !opsLeft.empty( ))
        {
            ImageOps current = extractOneSubPixel( opsLeft );
            const uint32_t subCount = blendImages( current, channel, accum );
            LBASSERT( subCount < 2 );

            if( subCount > 0 )
                accum->accum();
            count += subCount;
        }
        if( count > 0 )
            accum->display();
        return count;
    }

    uint32_t count = 1;
    if( _useCPUAssembly( ops, true ))
        count = assembleImagesCPU( ops, channel, true );
    else for( const ImageOp& op : ops )
        assembleImage( op, channel );
    return count;
}

uint32_t Compositor::blendFrames( const Frames& frames, Channel* channel,
                                  util::Accum* accum )
{
    ImageOps ops;
    for( const Frame* frame : frames )
    {
        {
            const uint32_t timeout = channel->getConfig()->getTimeout();
            ChannelStatistics event( Statistic::CHANNEL_FRAME_WAIT_READY,
                                     channel );
            frame->waitReady( timeout );
        }

        for( const Image* image : frame->getImages( ))
            ops.push_back( ImageOp( frame, image ));
    }

    return blendImages( ops, channel, accum );
}

bool Compositor::isSubPixelDecomposition( const Frames& frames )
{
    if( frames.empty( ))
        return false;

    const SubPixel& subpixel =
        frames.front()->getFrameData()->getContext().subPixel;
    for( const Frame* frame : frames )
        if( subpixel != frame->getFrameData()->getContext().subPixel )
            return true;
    return false;
}

bool Compositor::isSubPixelDecomposition( const ImageOps& ops )
{
    if( ops.empty( ))
        return false;

    const SubPixel& subPixel = ops.front().image->getContext().subPixel;
    for( const ImageOp& op : ops )
        if( op.image->getContext().subPixel != subPixel )
            return true;
    return false;
}

Frames Compositor::extractOneSubPixel( Frames& frames )
{
    Frames current;

    const SubPixel& subPixel =
        frames.back()->getFrameData()->getContext().subPixel;
    current.push_back( frames.back( ));
    frames.pop_back();

    for( Frames::iterator i = frames.begin(); i != frames.end(); )
    {
        Frame* frame = *i;

        if( frame->getFrameData()->getContext().subPixel == subPixel )
        {
            current.push_back( frame );
            i = frames.erase( i );
        }
        else
            ++i;
    }

    return current;
}

ImageOps Compositor::extractOneSubPixel( ImageOps& ops )
{
    ImageOps current;

    const SubPixel& subPixel = ops.back().image->getContext().subPixel;
    current.push_back( ops.back( ));
    ops.pop_back();

    for( ImageOps::iterator i = ops.begin(); i != ops.end(); )
    {
        ImageOp& op = *i;

        if( op.image->getContext().subPixel == subPixel )
        {
            current.push_back( op );
            i = ops.erase( i );
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
    if( isSubPixelDecomposition( frames ))
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

            const SubPixel& subPixel =
                frames.back()->getFrameData()->getContext().subPixel;
            accum->setTotalSteps( subPixel.size );
        }

        Frames framesLeft = frames;
        while( !framesLeft.empty( ))
        {
            // get the frames with the same subpixel compound
            Frames current = extractOneSubPixel( framesLeft );

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
                                        const bool blend )
{
    if( frames.empty( ))
        return 0;

    // Assembles images from DB and 2D compounds using the CPU and then
    // assembles the result image. Does not support Pixel or Eye compounds.
    LBVERB << "Sorted CPU assembly" << std::endl;

    const Image* result = mergeFramesCPU( frames, blend,
                                          channel->getConfig()->getTimeout( ));
    return _assembleCPUImage( result, channel );
}

uint32_t Compositor::assembleImagesCPU( const ImageOps& images,
                                        Channel* channel,
                                        const bool blend )
{
    if( images.empty( ))
        return 0;

    // Assembles images from DB and 2D compounds using the CPU and then
    // assembles the result image. Does not support Pixel or Eye compounds.
    LBVERB << "Sorted CPU assembly" << std::endl;

    const Image* result = mergeImagesCPU( images, blend );
    return _assembleCPUImage( result, channel );
}

const Image* Compositor::mergeFramesCPU( const Frames& frames, const bool blend,
                                         const uint32_t timeout )
{
    ImageOps ops;
    for( const Frame* frame : frames )
    {
        frame->waitReady( timeout );
        for( const Image* image : frame->getImages( ))
        {
            ImageOp op( frame, image );
            op.offset = frame->getOffset();
            ops.emplace_back( op );
        }
    }
    return mergeImagesCPU( ops, blend );
}

const Image* Compositor::mergeImagesCPU( const ImageOps& ops, const bool blend )
{
    LBVERB << "Sorted CPU assembly" << std::endl;

    // Collect input image information and check preconditions
    PixelViewport destPVP;
    uint32_t colorInt = 0;
    uint32_t colorExt = 0;
    uint32_t colorPixelSize = 0;
    uint32_t depthInt = 0;
    uint32_t depthExt = 0;
    uint32_t depthPixelSize = 0;

    if( !_collectOutputData( ops, destPVP, colorInt, colorPixelSize,
                             colorExt, depthInt, depthPixelSize, depthExt ))
    {
        return 0;
    }

    // prepare output image
    if( !_resultImage )
        _resultImage = new Image;
    Image* result = _resultImage.get();

    // pre-condition check for current _merge implementations
    LBASSERT( colorInt != 0 );

    result->setPixelViewport( destPVP );

    PixelData colorPixels;
    colorPixels.internalFormat = colorInt;
    colorPixels.externalFormat = colorExt;
    colorPixels.pixelSize      = colorPixelSize;
    colorPixels.pvp            = destPVP;
    result->setPixelData( Frame::BUFFER_COLOR, colorPixels );

    void* destDepth = 0;
    if( depthInt != 0 ) // at least one depth assembly
    {
        LBASSERT( depthExt ==
                  EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT );
        PixelData depthPixels;
        depthPixels.internalFormat = depthInt;
        depthPixels.externalFormat = depthExt;
        depthPixels.pixelSize      = depthPixelSize;
        depthPixels.pvp            = destPVP;
        result->setPixelData( Frame::BUFFER_DEPTH, depthPixels );
        destDepth = result->getPixelPointer( Frame::BUFFER_DEPTH );
    }

    // assembly
    _mergeImages( ops, blend, result->getPixelPointer( Frame::BUFFER_COLOR ),
                  destDepth, destPVP );
    return result;
}

void Compositor::assembleFrame( const Frame* frame, Channel* channel )
{
    const Images& images = frame->getImages();
    if( images.empty( ))
        LBINFO << "No images to assemble" << std::endl;

    for( Image* image : images )
    {
        ImageOp op( frame, image );
        op.offset = frame->getOffset();
        assembleImage( op, channel );
    }
}

void Compositor::assembleImage( const ImageOp& op, Channel* channel )
{
    const bool coreProfile = channel->getWindow()->getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( coreProfile && op.image->getContext().pixel != Pixel::ALL )
    {
        LBERROR << "No support for pixel assembly for OpenGL core profile,"
                   "skipping image" << std::endl;
        return;
    }

    ImageOp operation = op;
    operation.buffers = Frame::BUFFER_NONE;

    const Frame::Buffer buffer[] = { Frame::BUFFER_COLOR, Frame::BUFFER_DEPTH };
    for( unsigned i = 0; i<2; ++i )
    {
        if( (op.buffers & buffer[i]) &&
            ( op.image->hasPixelData( buffer[i] ) ||
              op.image->hasTextureData( buffer[i] )) )
        {
            operation.buffers |= buffer[i];
        }
    }

    if( operation.buffers == Frame::BUFFER_NONE )
    {
        LBWARN << "No image attachment buffers to assemble" << std::endl;
        return;
    }

    setupStencilBuffer( operation, channel );

    if( operation.buffers == Frame::BUFFER_COLOR )
        assembleImage2D( operation, channel );
    else if( operation.buffers == ( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH ))
        assembleImageDB( operation, channel );
    else
        LBWARN << "Don't know how to assemble using buffers "
               << operation.buffers << std::endl;

    clearStencilBuffer( operation );
}

void Compositor::setupStencilBuffer( const ImageOp& op, const Channel* channel )
{
    const Pixel& pixel = op.image->getContext().pixel;
    if( pixel == Pixel::ALL )
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

    const PixelViewport& pvp = op.image->getPixelViewport();

    EQ_GL_CALL( glPixelZoom( float( pixel.w ), float( pixel.h )));

    if( pixel.w > 1 )
    {
        const float width  = float( pvp.w * pixel.w );
        const float step   = float( pixel.w );

        const float startX = float( op.offset.x() + pvp.x ) + 0.5f -
                             float( pixel.w );
        const float endX   = startX + width + pixel.w + step;
        const float startY = float( op.offset.y() + pvp.y + pixel.y );
        const float endY   = float( startY + pvp.h*pixel.h );

        glBegin( GL_QUADS );
        for( float x = startX + pixel.x + 1.0f ; x < endX; x += step)
        {
            glVertex3f( x-step, startY, 0.0f );
            glVertex3f( x-1.0f, startY, 0.0f );
            glVertex3f( x-1.0f, endY, 0.0f );
            glVertex3f( x-step, endY, 0.0f );
        }
        glEnd();
    }
    if( pixel.h > 1 )
    {
        const float height = float( pvp.h * pixel.h );
        const float step   = float( pixel.h );
        const float startX = float( op.offset.x() + pvp.x + pixel.x );
        const float endX   = float( startX + pvp.w * pixel.w );
        const float startY = float( op.offset.y() + pvp.y ) + 0.5f -
                             float( pixel.h );
        const float endY   = startY + height + pixel.h + step;

        glBegin( GL_QUADS );
        for( float y = startY + pixel.y; y < endY; y += step)
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

    const ColorMask& colorMask = channel->getDrawBufferMask();
    EQ_GL_CALL( glColorMask( colorMask.red, colorMask.green, colorMask.blue,
                             true ));
    EQ_GL_CALL( glDepthMask( true ));
}

void Compositor::clearStencilBuffer( const ImageOp& op )
{
    if( op.image->getContext().pixel == Pixel::ALL )
        return;

    EQ_GL_CALL( glPixelZoom( 1.f, 1.f ));
    EQ_GL_CALL( glDisable( GL_STENCIL_TEST ));
}

void Compositor::assembleImage2D( const ImageOp& op, Channel* channel )
{
    if( GLEW_VERSION_3_3 )
        _drawPixelsGLSL( op, Frame::BUFFER_COLOR, channel );
    else
        _drawPixelsFF( op, Frame::BUFFER_COLOR, channel );
    declareRegion( op, channel );
#if 0
    static lunchbox::a_int32_t counter;
    std::ostringstream stringstream;
    stringstream << "Image_" << ++counter;
    op.image->writeImages( stringstream.str( ));
#endif
}

void Compositor::assembleImageDB( const ImageOp& op, Channel* channel )
{
    if( GLEW_VERSION_3_3 )
        assembleImageDB_GLSL( op, channel );
    else
        assembleImageDB_FF( op, channel );
}

void Compositor::assembleImageDB_FF( const ImageOp& op, Channel* channel )
{
    const PixelViewport& pvp = op.image->getPixelViewport();
    LBLOG( LOG_ASSEMBLY ) << "assembleImageDB_FF " << pvp << std::endl;

    // Z-Based sort-last assembly
    EQ_GL_CALL( glRasterPos2i( op.offset.x() + pvp.x, op.offset.y() + pvp.y ));
    EQ_GL_CALL( glEnable( GL_STENCIL_TEST ));

    // test who is in front and mark in stencil buffer
    EQ_GL_CALL( glEnable( GL_DEPTH_TEST ));

    const bool pixelComposite = ( op.image->getContext().pixel != Pixel::ALL );
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

    _drawPixelsFF( op, Frame::BUFFER_DEPTH, channel );

    EQ_GL_CALL( glDisable( GL_DEPTH_TEST ));

    // draw front-most, visible pixels using stencil mask
    EQ_GL_CALL( glStencilFunc( GL_EQUAL, 1, 1 ));
    EQ_GL_CALL( glStencilOp( GL_KEEP, GL_ZERO, GL_ZERO ));

    _drawPixelsFF( op, Frame::BUFFER_COLOR, channel );

    EQ_GL_CALL( glDisable( GL_STENCIL_TEST ));
    declareRegion( op, channel );
}

void Compositor::assembleImageDB_GLSL( const ImageOp& op, Channel* channel )
{
    const PixelViewport& pvp = op.image->getPixelViewport();
    LBLOG( LOG_ASSEMBLY ) << "assembleImageDB_GLSL " << pvp << std::endl;

    util::ObjectManager& om = channel->getObjectManager();
    const bool useImageTexture =
        op.image->getStorageType() == Frame::TYPE_TEXTURE;

    const util::Texture* textureColor = 0;
    const util::Texture* textureDepth = 0;
    if ( useImageTexture )
    {
        textureColor = &op.image->getTexture( Frame::BUFFER_COLOR );
        textureDepth = &op.image->getTexture( Frame::BUFFER_DEPTH );
    }
    else
    {
        util::Texture* ncTextureColor = om.obtainEqTexture( colorDBKey,
                                                     GL_TEXTURE_RECTANGLE_ARB );
        util::Texture* ncTextureDepth = om.obtainEqTexture( depthDBKey,
                                                     GL_TEXTURE_RECTANGLE_ARB );
        const Vector2i offset( -pvp.x, -pvp.y ); // will be applied with quad

        op.image->upload( Frame::BUFFER_COLOR, ncTextureColor, offset, om );
        op.image->upload( Frame::BUFFER_DEPTH, ncTextureDepth, offset, om );

        textureColor = ncTextureColor;
        textureDepth = ncTextureDepth;
    }

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE1 ));
    textureDepth->bind();
    textureDepth->applyZoomFilter( op.zoomFilter );

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE0 ));
    textureColor->bind();
    textureColor->applyZoomFilter( op.zoomFilter );

    _drawTexturedQuad( shaderDBKey, op, pvp, true, channel );

    declareRegion( op, channel );
}

void Compositor::declareRegion( const ImageOp& op, Channel* channel )
{
    if( !channel )
        return;

    const eq::PixelViewport area = op.image->getPixelViewport() + op.offset;
    channel->declareRegion( area );
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
