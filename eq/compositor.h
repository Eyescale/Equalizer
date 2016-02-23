
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/frame.h>          // nested type Frame::Buffer
#include <eq/types.h>          // type definitions

#include <eq/fabric/pixel.h>          // member
#include <eq/fabric/zoom.h>           // member

#include <vector>

namespace eq
{
/**
 * A set of functions performing compositing for a set of input frames.
 *
 * The following diagram depicts the call flow within the compositor. Typically,
 * an application uses one of the entry functions assembleFrames() or
 * assembleFramesUnsorted(), but the various lower-level functions are still
 * useful for advanced tasks, e.g., mergeFramesCPU() to perform compositing on
 * the CPU into a main memory buffer.
 *
 * <img src="http://www.equalizergraphics.com/documents/design/images/compositor.png">
 */
class EQ_API Compositor
{
public:
    /** @name Frame-based operations. */
    //@{
    /**
     * Assemble all frames in an arbitrary order using the fastest implemented
     * algorithm on the given channel.
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
     * @return the number of different subpixel steps assembled.
     * @version 1.11
     */
    static uint32_t blendFrames( const Frames& frames, Channel* channel,
                                 util::Accum* accum );
    static uint32_t blendImages( const ImageOps& images, Channel* channel,
                                 util::Accum* accum );

    /**
     * Assemble all frames in the order they become available directly on the
     * given channel.
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
     * Assemble all frames in the given order in a memory buffer using the CPU
     * before assembling the result on the given channel.
     *
     * If alpha-blending is enabled, the images are blended into the
     * intermediate image in main memory as if using: glBlendFuncSeparate(
     * GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA ). The resulting image is
     * composited into the current framebuffer, using preset OpenGL blending
     * state.
     *
     * @param frames the frames to assemble.
     * @param channel the destination channel.
     * @param blend blend color-only images if they have an alpha
     *                   channel
     * @return the number of different subpixel steps assembled (0 or 1).
     * @version 1.0
     */
    static uint32_t assembleFramesCPU( const Frames& frames, Channel* channel,
                                       const bool blend = false );
    static uint32_t assembleImagesCPU( const ImageOps& ops, Channel* channel,
                                       const bool blend );

    /**
     * Merge the provided frames in the given order into one image in main
     * memory.
     *
     * The returned image does not have to be freed. The compositor maintains
     * one image per thread, that is, the returned image is valid until the next
     * usage of the compositor in the current thread.
     *
     * @version 1.0
     */
    static const Image* mergeFramesCPU( const Frames& frames,
                                        const bool blend = false,
                               const uint32_t timeout = LB_TIMEOUT_INDEFINITE );
    static const Image* mergeImagesCPU( const ImageOps& ops, const bool blend );

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
     * @param operation an ImageOp struct describing the operation.
     * @param channel the destination channel
     */
    static void assembleImage( const ImageOp& operation, Channel* channel );

    /**
     * Setup the stencil buffer for a pixel compound recomposition.
     *
     * @param operation the assembly parameters.
     * @param channel the destination channel
     */
    static void setupStencilBuffer( const ImageOp& operation,
                                    const Channel* channel );

    /**
     * Clear the stencil buffer after a pixel compound recomposition.
     *
     * @param operation the assembly parameters.
     */
    static void clearStencilBuffer( const ImageOp& operation );

    /**
     * Setup the OpenGL state.
     * @param pvp the current pixel viewport.
     * @param gl the OpenGL function table
     */
    static void setupAssemblyState( const PixelViewport& pvp,
                                    const GLEWContext* gl );

    /**
     * Reset the OpenGL state.
     */
    static void resetAssemblyState();

    /** Start a tile-based assembly of the image color attachment. */
    static void assembleImage2D( const ImageOp& op, Channel* channel );
    /** Start a Z-based assembly of the image color and depth attachment. */
    static void assembleImageDB( const ImageOp& op, Channel* channel );

    /**
     * Start a Z-based assembly of the image color and depth attachment, based
     * on OpenGL 1.1 functionality.
     */
    static void assembleImageDB_FF( const ImageOp& op, Channel* channel );

    /**
     * Start a Z-based assembly of the image color and depth attachment,
     * using GLSL.
     */
    static void assembleImageDB_GLSL( const ImageOp& op, Channel* channel );
    //@}

    /** @name Region of Interest. */
    //@{
    /**
     * Declare the region covered by the image on the operation's channel.
     *
     * Called from all assembleImage methods.
     * @version 1.3
     */
    static void declareRegion( const ImageOp& op, Channel* channel );
    //@}

    /** @name Early assembly. */
    //@{
    /** A handle for one unordered assembly. @version 1.3.1 */
    class WaitHandle;

    /** Start waiting on a set of input frames. @version 1.3.1 */
    static WaitHandle* startWaitFrames( const Frames& frames,
                                        Channel* channel );

    /**
     * Wait for one input frame from a set of pending frames.
     *
     * Before the first call, a wait handle is acquired using
     * startWaitFrames(). When all frames have been processed, 0 is returned and
     * the wait handle is invalidated. If the wait times out, an exception is
     * thrown and the wait handle in invalidated.
     *
     * @param handle the wait handle acquires using startWaitFrames().
     * @return One ready frame, or 0 if all frames have been processed.
     * @version 1.3.1
     */
    static Frame* waitFrame( WaitHandle* handle );
    //@}

    /** @name Introspection and setup */
    //@{
    static bool isSubPixelDecomposition( const Frames& frames );
    static bool isSubPixelDecomposition( const ImageOps& ops );
    static Frames extractOneSubPixel( Frames& frames );
    static ImageOps extractOneSubPixel( ImageOps& ops );
    //@}

private:
    typedef std::pair< const Frame*, const Image* > FrameImage;
};
}

#endif // EQ_COMPOSITOR_H
