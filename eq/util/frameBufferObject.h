
/* Copyright (c) 2008-2015, Cedric Stalder <cedric.stalder@gmail.com>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQUTIL_FRAMEBUFFEROBJECT_H
#define EQUTIL_FRAMEBUFFEROBJECT_H

#include <eq/util/texture.h> // member
#include <eq/util/types.h>

#include <vector>

namespace eq
{
namespace util
{
/** A C++ class to abstract OpenGL frame buffer objects */
class FrameBufferObject
{
public:
    /**
     * Construct a new Frame Buffer Object. The first color texture is
     * automatically created.
     * @version 1.0
     */
    EQ_API FrameBufferObject( const GLEWContext* const glewContext,
                              const unsigned textureTarget = 0x84F5
                              /*GL_TEXTURE_RECTANGLE_ARB*/ );

    /** Destruct the Frame Buffer Object. @version 1.0 */
    EQ_API ~FrameBufferObject();

    /**
     * Add one color texture to the FBO.
     *
     * The maximum number of textures per FBO is 16. Added color textures will
     * have the same format as the existing texture(s). This method has to be
     * called on an uninitialized FBO.
     *
     * @return false if color texture can't be added, otherwise true.
     * @version 1.0
     */
    EQ_API bool addColorTexture();

    /**
     * Initialize the Frame Buffer Object.
     *
     * On successful initialization, the FBO is bound.
     *
     * @param width the initial width of the rendering buffer.
     * @param height the initial height of the rendering buffer.
     * @param colorFormat The internal color texture format, e.g., GL_RGBA.
     * @param depthSize The bit depth of the depth attachment.
     * @param stencilSize The bit depth of the stencil attachment.
     * @param samplesSize The number of multisamples.
     * @return ERROR_NONE on success, Error code on failure.
     * @sa resize()
     * @version 1.0
     */
    EQ_API Error init( const int32_t width, const int32_t height,
                       const unsigned colorFormat, const int32_t depthSize,
                       const int32_t stencilSize,
                       const int32_t samplesSize = 1 );

    /** De-initialize the Frame Buffer Object. @version 1.0 */
    EQ_API void exit();

    /**
     * Bind to the Frame Buffer Object.
     *
     * The FBO becomes the read and/or draw buffer of the current context,
     * depending on the given target.
     * @param target the framebuffer target to bind, default read and draw
     * @version 1.0
     */
    EQ_API void bind( const uint32_t target = 0x8D40 /*GL_FRAMEBUFFER_EXT*/ );

    /**
     * Unbind any Frame Buffer Object and use the default drawable for the
     * current context.
     * @param target the framebuffer target to unbind, default read and draw
     * @version 1.0
     */
    EQ_API void unbind( const uint32_t target = 0x8D40 /*GL_FRAMEBUFFER_EXT*/ );

    /**
     * Resize the FBO.
     *
     * The FBO has to be initialized and bound. It is not changed if the size
     * does not change.
     *
     * @return true on success, false on error.
     * @return ERROR_NONE on success, Error code on failure.
     * @version 1.0
     */
    EQ_API Error resize( const int32_t width, const int32_t height );

    /** @return the current width. @version 1.0 */
    int32_t getWidth() const
        { LBASSERT( !_colors.empty( )); return _colors.front()->getWidth();}

    /** @return the current height. @version 1.0 */
    int32_t getHeight() const
        { LBASSERT( !_colors.empty()); return _colors.front()->getHeight();}

    /** @return the vector of color textures. @version 1.0 */
    const Textures& getColorTextures() const { return _colors; }

    /** @return the depth texture. @version 1.0 */
    const Texture& getDepthTexture() const { return _depth; }

    /** @return the GLEW context. @version 1.0 */
    const GLEWContext* glewGetContext() const { return _glewContext; }

    /** @return true if the fbo is valid. @version 1.0 */
    bool isValid() const { return _valid; }

private:
    unsigned _fboID; //!< the FBO GL name

    Textures _colors; //!< Multiple color textures
    Texture _depth;

    const GLEWContext* const _glewContext;

    bool _valid;

    LB_TS_VAR( _thread );

    /** Check the result after changes to an FBO and set the _valid flag. */
    Error _checkStatus();
};
}
}

#endif // EQUTIL_FRAMEBUFFEROBJECT_H
