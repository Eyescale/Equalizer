
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Makhinya Maxim
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

#ifndef EQ_GL_WINDOW_H
#define EQ_GL_WINDOW_H

#include <eq/systemWindow.h>         // base class

namespace eq
{
namespace detail { class GLWindow; }

/**
 * A system window for OpenGL rendering.
 *
 * The GLWindow implements all generic OpenGL functionality for a SystemWindow.
 * It is subclassed by OS-specific implementations which provide the the glue to
 * the actual window system.
 */
class GLWindow : public SystemWindow
{
public:
    /** Construct a new OpenGL window. @version 1.7.2 */
    EQ_API GLWindow( NotifierInterface& parent,
                     const WindowSettings& settings );

    /** Destruct a new OpenGL window. @version 1.0 */
    EQ_API virtual ~GLWindow();

    /** Bind the FBO and update the current cache. @version 1.0 */
    EQ_API void makeCurrent( const bool cache = true ) const override;

    /** Unbind the FBO and remove context from current cache. @version 1.10 */
    EQ_API void doneCurrent() const override;

    /**
     * @return true if this window was last made current in this thread.
     * @version 1.3.2
     */
    EQ_API bool isCurrent() const;

    /** @name Frame Buffer Object support. */
    //@{
    /** Bind the window's FBO, if it uses an FBO drawable. @version 1.0 */
    EQ_API void bindFrameBuffer() const override;

    /** Bind the window's draw FBO, used for multisampling. @version 1.9 */
    EQ_API void bindDrawFrameBuffer() const override;

    /** Update the window's FBO from the multisampled FBO. @version 1.9 */
    EQ_API void updateFrameBuffer() const override;

    /** Flush all command buffers. @version 1.5.2 */
    EQ_API void flush() override;

    /** Finish execution of  all commands. @version 1.5.2 */
    EQ_API void finish() override;

    /** Build and initialize the FBO. @version 1.0 */
    EQ_API virtual bool configInitFBO();

    /** Destroy the FBO. @version 1.0 */
    EQ_API virtual void configExitFBO();

    /** @return the FBO of this window, or 0. @version 1.0 */
    EQ_API const util::FrameBufferObject* getFrameBufferObject() const override;

    /** @return the FBO of this window, or 0. @version 1.9 */
    EQ_API util::FrameBufferObject* getFrameBufferObject() override;
    //@}

    /** Initialize the GLEW context for this window. @version 1.0 */
    EQ_API virtual void initGLEW();

    /** De-initialize the GLEW context. @version 1.0 */
    EQ_API virtual void exitGLEW();

    /**
     * Get the GLEW context for this window.
     *
     * The glew context is initialized during window initialization, and
     * provides access to OpenGL extensions. This function does not follow the
     * Equalizer naming conventions, since GLEW uses a function of this name to
     * automatically resolve OpenGL function entry points. Therefore, any
     * supported GL function can be called directly from an initialized
     * GLWindow.
     *
     * @return the OpenGL function table for the window's OpenGL context
     * @version 1.0
     */
    EQ_API const GLEWContext* glewGetContext() const override;

    /**
     * Set up the drawable config by querying the current context.
     * @version 1.0
     */
    EQ_API void queryDrawableConfig( DrawableConfig& ) override;

private:
    GLWindow( const GLWindow& ) = delete;
    GLWindow& operator=( const GLWindow& ) = delete;
    bool _createFBO( util::FrameBufferObject*& fbo, const int samplesSize );
    void _destroyFBO( util::FrameBufferObject*& fbo );

    detail::GLWindow* const _impl;

    GLEWContext* glewGetContext();
};
}


#endif // EQ_GL_WINDOW_H

