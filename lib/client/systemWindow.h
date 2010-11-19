
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

#ifndef EQ_SYSTEM_WINDOW_H
#define EQ_SYSTEM_WINDOW_H

#include <eq/client/types.h>
#include <eq/client/window.h>         // Window::IAttribute enum
#include <eq/client/windowSystem.h>   // GLEWContext

namespace eq
{
namespace util
{
    class FrameBufferObject;
}

    /**
     * The interface definition for system-specific windowing code.
     *
     * The system window abstracts all window system specific code and
     * facilitates porting to new windowing systems. Each eq::Window uses one
     * eq::SystemWindow, which is created and initialized in
     * Window::configInitSystemWindow().
     */
    class SystemWindow
    {
    public:
        /** Create a new SystemWindow for the given eq::Window. */
        EQ_CLIENT_DECL SystemWindow( Window* parent );

        /** Destroy the SystemWindow. */
        EQ_CLIENT_DECL virtual ~SystemWindow();
        
        /** @name Methods forwarded from eq::Window */
        //@{
        /** 
         * Initialize this system window.
         * 
         * This method should take into account all attributes of the parent
         * Window.
         * 
         * @return true if the window was correctly initialized, false
         *         on any error.
         */
        EQ_CLIENT_DECL virtual bool configInit( ) = 0;

        /** 
         * De-initialize this system window.
         * 
         * This function might be called on partially or uninitialized system
         * windows, and the implemenation has therefore be tolerant enough to
         * handle this case.
         */
        EQ_CLIENT_DECL virtual void configExit( ) = 0;

        /** 
         * Make the system window rendering context and drawable current.
         *
         * This function invalidates the pipe's make current cache. If this
         * function is not called, Pipe::setCurrent() has to be called
         * appropriately.
         */
        EQ_CLIENT_DECL virtual void makeCurrent() const = 0;

        /** Bind the window's FBO, if it uses an FBO drawable. */
        EQ_CLIENT_DECL virtual void bindFrameBuffer() const = 0;

        /** Swap the front and back buffer, for doublebuffered drawables. */
        EQ_CLIENT_DECL virtual void swapBuffers() = 0;

        /** 
         * Join a NV_swap_group.
         *
         * See WGL or GLX implementation and OpenGL extension for details on how
         * to implement this function.
         * 
         * @param group the swap group name.
         * @param barrier the swap barrier name.
         */
        EQ_CLIENT_DECL virtual void joinNVSwapBarrier( const uint32_t group,
                                                  const uint32_t barrier ) = 0;
        //@}

        /** @name Frame Buffer Object support. */
        //@{
        /** Build and initialize the FBO. */
        EQ_CLIENT_DECL bool configInitFBO();

        /** Destroy FBO. */
        EQ_CLIENT_DECL void configExitFBO();

        /** @return the FBO of this window, or 0 if no FBO is used. */
        EQ_CLIENT_DECL virtual const util::FrameBufferObject* getFrameBufferObject()
            const { return 0; }
        //@}

        /** @name Convenience interface to eq::Window methods */
        //@{
        Window* getWindow() { return _window; }
        const Window* getWindow() const { return _window; }

        EQ_CLIENT_DECL Pipe* getPipe(); 
        EQ_CLIENT_DECL const Pipe* getPipe() const;

        EQ_CLIENT_DECL Node* getNode(); 
        EQ_CLIENT_DECL const Node* getNode() const;

        EQ_CLIENT_DECL Config* getConfig();
        EQ_CLIENT_DECL const Config* getConfig() const;

        EQ_CLIENT_DECL int32_t getIAttribute( const Window::IAttribute attr ) const;
        //@}

        /** Set up _drawableConfig by querying the current context. */
        EQ_CLIENT_DECL virtual void queryDrawableConfig( 
            DrawableConfig& drawableConfig ) = 0;

        /**
         * Get the GLEW context for this window.
         *
         * The glew context is initialized during window initialization, and
         * provides access to OpenGL extensions. This function does not follow
         * the Equalizer naming conventions, since GLEW uses a function of this
         * name to automatically resolve OpenGL function entry
         * points. Therefore, any supported GL function can be called directly
         * from an initialized SystemWindow.
         *
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        EQ_CLIENT_DECL virtual const GLEWContext* glewGetContext() const { return 0;}

        /** 
         * Set an error code why the last operation failed.
         * 
         * The error will be set on the parent window.
         *
         * @param error the error message.
         * @version 1.0
         */
        EQ_CLIENT_DECL void setError( const int32_t error );

        /** @return the last error from the last failed window operation. */
        EQ_CLIENT_DECL base::Error getError() const;

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


#endif // EQ_SYSTEM_WINDOW_H

