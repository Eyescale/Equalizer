
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMPOSITOR_H
#define EQ_COMPOSITOR_H

#include <eq/client/pixel.h>          // member
#include <eq/client/types.h>          // type definitions
#include <eq/base/base.h>             // EQ_EXPORT definition

#include <vmmlib/vector2.h>
#include <vector>

namespace eq
{
    class Channel;
    class Frame;
    class Image;

    /** A facility class for image assembly operations */
    class EQ_EXPORT Compositor
    {
    public:
        struct ImageOp
        {
            ImageOp() : channel( 0 ), buffers( 0 )
                      , offset( vmml::Vector2i::ZERO ), useCPU( false ) {}

            Channel*       channel;
            uint32_t       buffers;
            vmml::Vector2i offset;
            Pixel          pixel;
            bool           useCPU;
        };

        /** @name Frame-based operations. */
        //*{
        /** 
         * Assemble all frames in arbitrary order using the default algorithm.
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         */
        static void assembleFrames( const FrameVector& frames,
                                    Channel* channel );

        /** 
         * Assemble all frames in the given order using the default algorithm.
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
         * Assemble all frames in the order they become available using the GPU.
         *
         * @param frames the frames to assemble.
         * @param channel the destination channel.
         */
        static void assembleFramesUnsorted( const FrameVector& frames,
                                            Channel* channel );

        /** 
         * Assemble all frames in arbitrary order using the CPU before
         * assembling the result on the destination channel, using the GPU.
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

        static const Image* assembleFramesCPU(const FrameVector& frames,
                                              const bool blendAlpha = false );

        /** Assemble a frame using the default algorithm. */
        static void assembleFrame( const Frame* frame, Channel* channel );
        //*}

        
        /** @name Image-based operations. */
        //*{
        /** 
         * Start assembling an image.
         * 
         * @param image the input image.
         * @param buffers the frame buffer attachements to be used.
         * @param offset the image offset wrt the window's origin.
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
        //*}
                                
      private:
        typedef std::pair< const Frame*, const Image* > FrameImage;

        static void _assembleDBImages( Image* result,
                                       const std::vector< FrameImage >& images);
        static void _assemble2DImages( Image* result,
                                       const std::vector< FrameImage >& images);
        static void _assembleBlendImages( Image* result,
                                       const std::vector< FrameImage >& images);
        static bool   _assembleImage_PC( int operation, Image* result,
                                         const Image* source );
        static bool   _assembleBlendImage_PC( int operation, Image* result,
                                         const Image* source );

    };
}

#endif // EQ_COMPOSITOR_H

