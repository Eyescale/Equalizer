
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *                        , Maxim Makhinya
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

#ifndef EQ_OS_WINDOW_H
#define EQ_OS_WINDOW_H

#include <eq/client/types.h>
#include <eq/client/window.h>         // Window::IAttribute enum
#include <eq/client/windowSystem.h>   // GLEWContext

namespace eq
{
    class FrameBufferObject;

    /**
     * The interface definition for OS-specific windowing code.
     *
     * The OSWindow abstracts all window system specific code and facilitates
     * porting to new windowing systems. Each Windows uses one OSWindow, which
     * is created and initialized in Window::configInitOSWindow.
     */
    class EQ_EXPORT OSWindow
    {
    public:
        /** Create a new OSWindow for the given eq::Window. */
        OSWindow( Window* parent );

        /** Destroy the OSWindow. */
        virtual ~OSWindow( );
        
        /** @name Methods forwarded from eq::Window */
        //@{
        /** 
         * Initialize this OS window.
         * 
         * This method should take into account all attributes of the parent
         * Window.
         * 
         * @return true if the window was correctly initialized, false
         *         on any error.
         */
        virtual bool configInit( ) = 0;

        /** 
         * De-initialize this OS window.
         * 
         * This function might be called on partially or uninitialized OS
         * windows, and the implemenation has therefore be tolerant enough to
         * handle this case.
         */
        virtual void configExit( ) = 0;

        /** 
         * Make the OS window's rendering context and drawable current.
         *
         * This function invalidates the pipe's make current cache. If this
         * function is not called, Pipe::setCurrent() has to be called
         * appropriately.
         */
        virtual void makeCurrent() const = 0;

        /** Bind the window's FBO, if it uses an FBO drawable. */
        virtual void bindFrameBuffer() const = 0;

        /** Swap the front and back buffer, for doublebuffered drawables. */
        virtual void swapBuffers() = 0;

        /** 
         * Join a NV_swap_group.
         *
         * See WGL or GLX implementation and OpenGL extension for details on how
         * to implement this function.
         * 
         * @param group the swap group name.
         * @param barrier the swap barrier name.
         */
        virtual void joinNVSwapBarrier( const uint32_t group,
									    const uint32_t barrier ) = 0;
        //@}

        /** @name Frame Buffer Object support. */
        //@{
        /** Build and initialize the FBO. */
        bool configInitFBO();

        /** Destroy FBO. */
        void configExitFBO();

        /** @return the FBO of this window, or 0 if no FBO is used. */
        virtual const FrameBufferObject* getFrameBufferObject() { return 0; }
        //@}

        /** @name Convenience interface to eq::Window methods */
        //@{
        Window* getWindow() { return _window; }
        const Window* getWindow() const { return _window; }

        Pipe* getPipe(); 
        const Pipe* getPipe() const;

        Node* getNode(); 
        const Node* getNode() const;

        Config* getConfig();
        const Config* getConfig() const;

        int32_t getIAttribute( const Window::IAttribute attr ) const;
        //@}
		
		/** Set up _drawableConfig by querying the current context. */
		virtual void queryDrawableConfig() = 0;

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
        virtual GLEWContext* glewGetContext() { return 0; }
        virtual const GLEWContext* glewGetContext() const { return 0; }

    protected:
        /** The parent eq::Window. */
        Window* const _window;
        
    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}


#endif // EQ_OS_WINDOW_H

