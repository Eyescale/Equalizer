
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *                        , Makhinya Maxim
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

#include <eq/client/osWindow.h>         // Window::IAttribute enum

namespace eq
{
    class FrameBufferObject;

    /**
     * The interface definition for OS-specific windowing code.
     *
     * The OSWindow abstracts all window system specific code and facilitates
     * porting to new windowing systems. Each Windows uses one OSWindow, which
     * is initialized in Window::configInitOSWindow.
     */
    class EQ_EXPORT GLWindow : public OSWindow
    {
    public:
        GLWindow( Window* parent );
        virtual ~GLWindow( );
		
		virtual void makeCurrent() const;

        /** Bind the window's FBO, if it uses an FBO drawable. */
        virtual void bindFrameBuffer() const;

        /** @name Frame Buffer Object support. */
        //@{
        /** Build and initialize the FBO. */
		virtual bool configInitFBO();

        /** Destroy FBO. */
		virtual void configExitFBO();

        /** @return the FBO of this window, or 0 if no FBO is used. */
		virtual const FrameBufferObject* getFrameBufferObject() const { return _fbo; }
        //@}
     
        /** Initialize the GLEW context for this window. */
		virtual void initGLEW();
        
        /** De-initialize the GLEW context. */
		virtual void exitGLEW() { _glewInitialized = false; }
		
		/**
		 * Get the GLEW context for this window.
		 *
		 * The glew context is initialized during window initialization, and
		 * provides access to OpenGL extensions. This function does not follow
		 * the Equalizer naming conventions, since GLEW uses a function of this
		 * name to automatically resolve OpenGL function entry
		 * points. Therefore, any supported GL function can be called directly
		 * from an initialized OSWindow.
		 *
		 * @return the extended OpenGL function table for the window's OpenGL
		 *         context.
		 */
		GLEWContext* glewGetContext() { return _glewContext; }
		const GLEWContext* glewGetContext() const { return _glewContext; }

		/** Set up _drawableConfig by querying the current context. */
		virtual void queryDrawableConfig();

    private:
        bool _glewInitialized ;
        
		/** Extended OpenGL function entries when window has a context. */
        GLEWContext*   _glewContext;

        /** Frame buffer object for FBO drawables. */		
        FrameBufferObject* _fbo;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}


#endif // EQ_GL_WINDOW_H

