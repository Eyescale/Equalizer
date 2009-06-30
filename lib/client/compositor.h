
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

#ifndef EQ_COMPOSITOR_H
#define EQ_COMPOSITOR_H

#include <eq/client/frame.h>          // nested type Frame::Buffer 
#include <eq/client/pixel.h>          // member
#include <eq/client/types.h>          // type definitions
#include <eq/base/base.h>             // EQ_EXPORT definition

#include <vector>

namespace eq
{
    class Channel;
    class Image;

    /** A facility class for image assembly operations */
    class EQ_EXPORT Compositor
    {
    public:
        /** A structure describing an image assembly task. */
        struct ImageOp
        {
            ImageOp() : channel( 0 ), buffers( 0 )
                      , offset( Vector2i::ZERO ) {}

            Channel*       channel; //!< The destination channel
            uint32_t       buffers; //!< The Frame buffer attachments to use
            Vector2i offset;  //!< The offset wrt destination window
            Pixel          pixel;   //!< The pixel decomposition parameters
            Zoom           zoom;    //!< The zoom factor
        };

        /** @name Frame-based operations. */
        //@{
        /** 
         * Assemble all frames in an arbitrary order using the best algorithm
         * on the given channel
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         */
        static void assembleFrames( const FrameVector& frames,
                                    Channel* channel );

        /** 
         * Assemble all frames in the given order using the default algorithm
         * on the given channel.
         *
         * For alpha-blending see comment for assembleFramesCPU().
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         * @param blendAlpha blend color-only images if they have an alpha
         *                   channel
         */
        static void assembleFramesSorted( const FrameVector& frames,
                                          Channel* channel,
                                          const bool blendAlpha = false );

        /** 
         * Assemble all frames in the order they become available directly on
         * the given channel.
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         */
        static void assembleFramesUnsorted( const FrameVector& frames,
                                              Channel* channel );

        /** 
         * Assemble all frames in arbitrary order in a memory buffer using the
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
         */
        static void assembleFramesCPU( const FrameVector& frames,
                                       Channel* channel,
                                       const bool blendAlpha = false );

        /** Merge the provided frames into one image in main memory. */
        static const Image* mergeFramesCPU( const FrameVector& frames,
                                            const bool blendAlpha = false );

        /** 
         * Merge the provided frames into one main memory buffer.
         *
         * The called has to allocate and clear (if needed) the output buffers
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
         */
        static bool mergeFramesCPU( const FrameVector& frames,
                                    const bool blendAlpha,
                                    void* colorBuffer,
                                    const uint32_t colorBufferSize,
                                    void* depthBuffer, 
                                    const uint32_t depthBufferSize,
                                    PixelViewport& outPVP );

        /** Assemble a frame using the default algorithm. */
        static void assembleFrame( const Frame* frame, Channel* channel );
        //@}

        
        /** @name Image-based operations. */
        //@{
        /** 
         * Start assembling an image.
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

        static bool _collectOutputData( const FrameVector& frames,
                                        PixelViewport& destPVP, 
                                        uint32_t& colorFormat, 
                                        uint32_t& colorType,
                                        uint32_t& depthFormat,
                                        uint32_t& depthType );
                                        
        static void _mergeFrames( const FrameVector& frames,
                                  const bool blendAlpha, 
                                  void* colorBuffer, void* depthBuffer,
                                  const PixelViewport& destPVP );
                                  
        static void   _mergeDBImage( void* destColor, void* destDepth,
                                     const PixelViewport& destPVP, 
                                     const Image* image, 
                                     const Vector2i& offset );
                                     
        static void   _merge2DImage( void* destColor, void* destDepth,
                                     const PixelViewport& destPVP,
                                     const Image* input,
                                     const Vector2i& offset );
                                     
        static void   _mergeBlendImage( void* dest, 
                                        const PixelViewport& destPVP, 
                                        const Image* input,
                                        const Vector2i& offset );
        static bool     _mergeImage_PC( int operation, void* destColor, 
                                        void* destDepth, const Image* source );
        /** 
         * draw an image to the frame buffer using a texture quad or drawPixels.
         */
        static void _drawPixels( const Image* image, const ImageOp& op,
                                 const Frame::Buffer which );

    };
}

#endif // EQ_COMPOSITOR_H

