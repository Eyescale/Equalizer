
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2009, Makhinya Maxim
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
    /**
     * A system window for OpenGL rendering.
     *
     * The GLWindow implements all generic OpenGL functionality for a
     * SystemWindow. It is subclassed by OS-specific implementations which
     * provide the the glue to the actual window system.
     */
    class GLWindow : public SystemWindow
    {
    public:
        /** Construct a new OpenGL window. @version 1.0 */
        EQ_API GLWindow( Window* parent );

        /** Destruct a new OpenGL window. @version 1.0 */
        EQ_API virtual ~GLWindow();

        /** Bind the FBO and update the pipe's current cache. @version 1.0 */
        EQ_API virtual void makeCurrent() const;

        /** @name Frame Buffer Object support. */
        //@{
        /** Bind the window's FBO, if it uses an FBO drawable. @version 1.0 */
        EQ_API virtual void bindFrameBuffer() const;

        /** Build and initialize the FBO. @version 1.0 */
        EQ_API virtual bool configInitFBO();

        /** Destroy the FBO. @version 1.0 */
        EQ_API virtual void configExitFBO();

        /** @return the FBO of this window, or 0. @version 1.0 */
        virtual const util::FrameBufferObject* getFrameBufferObject()
            const { return _fbo; }
        //@}
     
        /** Initialize the GLEW context for this window. @version 1.0 */
        EQ_API virtual void initGLEW();
        
        /** De-initialize the GLEW context. @version 1.0 */
        virtual void exitGLEW() { _glewInitialized = false; }
        
        /**
         * Get the GLEW context for this window.
         *
         * The glew context is initialized during window initialization, and
         * provides access to OpenGL extensions. This function does not follow
         * the Equalizer naming conventions, since GLEW uses a function of this
         * name to automatically resolve OpenGL function entry
         * points. Therefore, any supported GL function can be called directly
         * from an initialized GLWindow.
         *
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         * @version 1.0
         */
        EQ_API virtual const GLEWContext* glewGetContext() const;

        /**
         * Set up the drawable config by querying the current context.
         * @version 1.0
         */
        EQ_API virtual void queryDrawableConfig( DrawableConfig& );

    private:
        bool _glewInitialized ;
        
        /** Extended OpenGL function entries when window has a context. */
        GLEWContext* const _glewContext;

        /** Frame buffer object for FBO drawables. */       
        util::FrameBufferObject* _fbo;

        GLEWContext* glewGetContext();

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}


#endif // EQ_GL_WINDOW_H

