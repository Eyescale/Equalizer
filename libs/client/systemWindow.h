
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

#include <eq/types.h>
#include <eq/window.h>         // Window::IAttribute enum
#include <eq/windowSystem.h>   // GLEWContext

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
     * Window::configInitSystemWindow.
     */
    class SystemWindow
    {
    public:
        /** Create a new SystemWindow for the given eq::Window. @version 1.0 */
        EQ_API SystemWindow( Window* parent );

        /** Destroy the SystemWindow. @version 1.0 */
        EQ_API virtual ~SystemWindow();
        
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
         * @version 1.0
         */
        EQ_API virtual bool configInit() = 0;

        /** 
         * De-initialize this system window.
         * 
         * This function might be called on partially or uninitialized system
         * windows, and the implemenation has therefore be tolerant enough to
         * handle this case.
         * @version 1.0
         */
        EQ_API virtual void configExit() = 0;

        /** 
         * Make the system window rendering context and drawable current.
         *
         * This function invalidates the pipe's make current cache. If this
         * function is not called, Pipe::setCurrent() has to be called
         * appropriately.
         * @version 1.0
         */
        EQ_API virtual void makeCurrent() const = 0;

        /** Bind the window's FBO, if it uses an FBO drawable. @version 1.0 */
        EQ_API virtual void bindFrameBuffer() const = 0;

        /** Swap the front and back buffer. @version 1.0 */
        EQ_API virtual void swapBuffers() = 0;

        /** 
         * Join a NV_swap_group.
         *
         * See WGL or GLX implementation and OpenGL extension for details on how
         * to implement this function.
         * 
         * @param group the swap group name.
         * @param barrier the swap barrier name.
         * @version 1.0
         */
        EQ_API virtual void joinNVSwapBarrier( const uint32_t group,
                                               const uint32_t barrier ) = 0;
        //@}

        /** @name Frame Buffer Object support. */
        //@{
        /** Build and initialize the FBO. @version 1.0 */
        EQ_API bool configInitFBO();

        /** Destroy FBO. @version 1.0 */
        EQ_API void configExitFBO();

        /** @return the FBO of this window, or 0. @version 1.0 */
        virtual const util::FrameBufferObject* getFrameBufferObject()
            const { return 0; }
        //@}

        /** @name Convenience interface to eq::Window methods */
        //@{
        /** @return the parent window. @version 1.0 */
        Window* getWindow() { return _window; }

        /** @return the parent window. @version 1.0 */
        const Window* getWindow() const { return _window; }

        /** @return the parent pipe. @version 1.0 */
        EQ_API Pipe* getPipe();

        /** @return the parent pipe. @version 1.0 */
        EQ_API const Pipe* getPipe() const;

        /** @return the parent node. @version 1.0 */
        EQ_API Node* getNode();

        /** @return the parent node. @version 1.0 */
        EQ_API const Node* getNode() const;

        /** @return the parent config. @version 1.0 */
        EQ_API Config* getConfig();

        /** @return the parent config. @version 1.0 */
        EQ_API const Config* getConfig() const;

        /** @return an integer attribute of the parent window. @version 1.0 */
        EQ_API int32_t getIAttribute( const Window::IAttribute attr ) const;
        //@}

        /**
         * Set up the given drawable based on the current context.
         * @version 1.0
         */
        EQ_API virtual void queryDrawableConfig( DrawableConfig& dc ) = 0;

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
         * @version 1.0
         */
        virtual const GLEWContext* glewGetContext() const { return 0;}

        /** 
         * Set an error code why the last operation failed.
         * 
         * The error will be set on the parent window.
         *
         * @param error the error message.
         * @version 1.0
         */
        EQ_API void setError( const int32_t error );

        /** @return the error from the last failed operation. @version 1.0 */
        EQ_API co::base::Error getError() const;

        /** Process an event. @version 1.0 */
        EQ_API virtual bool processEvent( const Event& event );

    private:
        /** The parent eq::Window. */
        Window* const _window;
        
        struct Private;
        Private* _private; // placeholder for binary-compatible changes
    };
}


#endif // EQ_SYSTEM_WINDOW_H

