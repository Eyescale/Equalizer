
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_COMPOSITOR_H
#define EQ_COMPOSITOR_H

#include "frame.h"          // nested type Frame::Buffer
#include "image.h"          // nested type PixelData 
#include "types.h"          // type definitions

#include <eq/fabric/pixel.h>          // member

#include <vector>

namespace eq
{
namespace util
{
    class Accum;
}
    class Channel;

    /** 
     * A set of functions performing compositing for a set of input frames.
     *
     * The following diagram depicts the call flow within the
     * compositor. Typically, an application uses one of the entry functions
     * assembleFrames() or assembleFramesUnsorted(), but the various lower-level
     * functions are still useful for advanced tasks, e.g., mergeFramesCPU() to
     * perform compositing on the CPU into a main memory buffer.
     * 
     * <img src="http://www.equalizergraphics.com/documents/design/images/compositor.png">
     */
    class EQ_EXPORT Compositor
    {
    public:
        /** A structure describing an image assembly task. */
        struct ImageOp
        {
            ImageOp() : channel( 0 ), buffers( 0 )
                      , offset( Vector2i::ZERO ) {}

            Channel* channel; //!< The destination channel
            uint32_t buffers; //!< The Frame buffer attachments to use
            Vector2i offset;  //!< The offset wrt destination window
            Pixel pixel;      //!< The pixel decomposition parameters
            Zoom zoom;        //!< The zoom factor
        };

        /** @name Frame-based operations. */
        //@{
        /** 
         * Assemble all frames in an arbitrary order using the fastest
         * implemented algorithm on the given channel.
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         * @param accum the accumulation buffer.
         * @return the number of different subpixel steps assembled.
         * @version 1.0
         */
        static uint32_t assembleFrames( const Frames& frames,
                                        Channel* channel, util::Accum* accum );

        /** 
         * Assemble all frames in the given order using the fastest implemented
         * algorithm on the given channel.
         *
         * For alpha-blending see comment for assembleFramesCPU().
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         * @param accum the accumulation buffer.
         * @param blendAlpha blend color-only images if they have an alpha
         *                   channel
         * @return the number of different subpixel steps assembled.
         * @version 1.0
         */
        static uint32_t assembleFramesSorted( const Frames& frames,
                                              Channel* channel, 
                                              util::Accum* accum,
                                              const bool blendAlpha = false );

        /** 
         * Assemble all frames in the order they become available directly on
         * the given channel.
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         * @param accum the accumulation buffer.
         * @return the number of different subpixel steps assembled.
         * @version 1.0
         */
        static uint32_t assembleFramesUnsorted( const Frames& frames,
                                                Channel* channel,
                                                util::Accum* accum );

        /** 
         * Assemble all frames in the given order in a memory buffer using the
         * CPU before assembling the result on the given channel.
         *
         * If alpha-blending is enabled, the images are blended into the
         * intermediate image in main memory as if using:
         * glBlendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA )
         * The resulting image is composited using
         * glBlendFunc( GL_ONE, GL_SRC_ALPHA )
         * into the current framebuffer.
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         * @param blendAlpha blend color-only images if they have an alpha
         *                   channel
         * @return the number of different subpixel steps assembled (0 or 1).
         * @version 1.0
         */
        static uint32_t assembleFramesCPU( const Frames& frames,
                                           Channel* channel,
                                           const bool blendAlpha = false );

        /**
         * Merge the provided frames in the given order into one image in main
         * memory.
         *
         * The returned image does not have to be freed. The compositor
         * maintains one image per thread, that is, the returned image is valid
         * until the next usage of the compositor in the current thread.
         *
         * @version 1.0
         */
        static const Image* mergeFramesCPU( const Frames& frames,
                                            const bool blendAlpha = false );

        /** 
         * Merge the provided frames into one main memory buffer.
         *
         * The callee has to allocate and clear (if needed) the output buffers
         * to hold the necessary data. All input images have to use the same
         * format and type, which will also be the output format. The depth
         * buffer and depth buffer size may be 0, if the images contain no depth
         * information.
         *
         * The output pixel viewport receives the image size and offset wrt the
         * destination channel.
         *
         * @return true if the compositing was successful, false otherwise,
         *         e.g., a buffer is too small.
         * @version 1.0
         */
        static bool mergeFramesCPU( const Frames& frames,
                                    const bool blendAlpha,
                                    void* colorBuffer,
                                    const uint32_t colorBufferSize,
                                    void* depthBuffer, 
                                    const uint32_t depthBufferSize,
                                    PixelViewport& outPVP );

        /**
         * Assemble a frame into the frame buffer using the default algorithm.
         * @version 1.0
         */
        static void assembleFrame( const Frame* frame, Channel* channel );
        //@}

        
        /** @name Image-based operations. */
        //@{
        /** 
         * Assemble an image into the frame buffer.
         * 
         * @param image the input image.
         * @param operation an ImageOp struct describing the operation.
         */
        static void assembleImage( const Image* image,
                                   const ImageOp& operation );

        /** 
         * Setup the stencil buffer for a pixel compound recomposition.
         * 
         * @param image the image to be assembled.
         * @param operation the assembly parameters.
         */
        static void setupStencilBuffer( const Image* image, 
                                        const ImageOp& operation );

        /** 
         * Clear the stencil buffer after a pixel compound recomposition.
         * 
         * @param operation the assembly parameters.
         */
        static void clearStencilBuffer( const ImageOp& operation );

        /**
         * Setup the OpenGL state.
         * @param pvp the current pixel viewport.
         */
        static void setupAssemblyState( const PixelViewport& pvp );
        
        /**
         * Reset the OpenGL state.
         */
        static void resetAssemblyState();

        /** Start a tile-based assembly of the image color attachment. */
        static void assembleImage2D( const Image* image, const ImageOp& op );
        /** Start a Z-based assembly of the image color and depth attachment. */
        static void assembleImageDB( const Image* image, const ImageOp& op );

        /** 
         * Start a Z-based assembly of the image color and depth attachment,
         * based on OpenGL 1.1 functionality.
         */
        static void assembleImageDB_FF( const Image* image, const ImageOp& op );

        /** 
         * Start a Z-based assembly of the image color and depth attachment,
         * using GLSL.
         */
        static void assembleImageDB_GLSL( const Image* image, 
                                          const ImageOp& op );
        //@}
                                
      private:
        typedef std::pair< const Frame*, const Image* > FrameImage;

        static bool _isSubPixelDecomposition( const Frames& frames );
        static const Frames _extractOneSubPixel( Frames& frames );

        static bool _collectOutputData( 
                             const Frames& frames, 
                             PixelViewport& destPVP, 
                             uint32_t& colorInternalFormat, 
                             uint32_t& colorPixelSize,
                             uint32_t& colorExternalFormat,
                             uint32_t& depthInternalFormat,
                             uint32_t& depthPixelSize,
                             uint32_t& depthExternalFormat );
                              
        static void _collectOutputData( const Image::PixelData& pixelData, 
                                        uint32_t& internalFormat, 
                                        uint32_t& pixelSize, 
                                        uint32_t& externalFormat );

        static void _mergeFrames( const Frames& frames,
                                  const bool blendAlpha, 
                                  void* colorBuffer, void* depthBuffer,
                                  const PixelViewport& destPVP );
                                  
        static void _mergeDBImage( void* destColor, void* destDepth,
                                   const PixelViewport& destPVP, 
                                   const Image* image, 
                                   const Vector2i& offset );
                                     
        static void _merge2DImage( void* destColor, void* destDepth,
                                   const PixelViewport& destPVP,
                                   const Image* input,
                                   const Vector2i& offset );
                                     
        static void _mergeBlendImage( void* dest, 
                                      const PixelViewport& destPVP, 
                                      const Image* input,
                                      const Vector2i& offset );
        static bool _mergeImage_PC( int operation, void* destColor, 
                                    void* destDepth, const Image* source );
        /** 
         * draw an image to the frame buffer using a texture quad or drawPixels.
         */
        static void _drawPixels( const Image* image, const ImageOp& op,
                                 const Frame::Buffer which );

        /** @return the accumulation buffer used for subpixel compositing. */
        static util::Accum* _obtainAccum( Channel* channel );
    };
}

#endif // EQ_COMPOSITOR_H

