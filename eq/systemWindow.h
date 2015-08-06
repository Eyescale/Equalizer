
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Maxim Makhinya
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
#include <eq/windowSettings.h> // WindowSettings::IAttribute enum

namespace eq
{
/**
 * The interface definition for system-specific windowing code.
 *
 * The system window abstracts all window system specific code and facilitates
 * porting to new windowing systems. Each eq::Window uses one eq::SystemWindow,
 * which is created and initialized in Window::configInitSystemWindow.
 */
class SystemWindow
{
public:
    /** Create a new SystemWindow for the given eq::Window. @version 1.7.2 */
    EQ_API SystemWindow( NotifierInterface& parent,
                         const WindowSettings& settings );

    /** Destroy the SystemWindow. @version 1.0 */
    EQ_API virtual ~SystemWindow();

    /** @name Methods forwarded from eq::Window */
    //@{
    /**
     * Initialize this system window.
     *
     * This method should take into account all attributes of the parent Window.
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
     * windows, and the implemenation has therefore be tolerant enough to handle
     * this case.
     * @version 1.0
     */
    EQ_API virtual void configExit() = 0;

    /**
     * Make the system window rendering context and drawable current.
     *
     * This function invalidates the pipe's make current cache.
     * @version 1.0
     */
    EQ_API virtual void makeCurrent( const bool cache = true ) const = 0;

    /**
     * This results in no context being current in the current thread.
     *
     * This function resets the pipe's make current cache.
     * @version 1.10
     */
    EQ_API virtual void doneCurrent() const = 0;

    /** Bind the window's FBO, if it uses an FBO drawable. @version 1.0 */
    EQ_API virtual void bindFrameBuffer() const = 0;

    /** Bind the window's draw FBO, used for multisampling. @version 1.9 */
    EQ_API virtual void bindDrawFrameBuffer() const = 0;

    /** Update the window's FBO from the multisampled FBO. @version 1.9 */
    EQ_API virtual void updateFrameBuffer() const = 0;

    /** Swap the front and back buffer. @version 1.0 */
    EQ_API virtual void swapBuffers() = 0;

    /** Flush all command buffers. @version 1.5.2 */
    EQ_API virtual void flush() = 0;

    /** Finish execution of  all commands. @version 1.5.2 */
    EQ_API virtual void finish() = 0;

    /**
     * Join a NV_swap_group.
     *
     * See WGL or GLX implementation and OpenGL extension for details on how to
     * implement this function.
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
    /** @return the FBO of this window, or 0. @version 1.0 */
    virtual const util::FrameBufferObject* getFrameBufferObject()
        const { return 0; }
    /** @return the FBO of this window, or 0. @version 1.0 */
    virtual util::FrameBufferObject* getFrameBufferObject() { return 0; }
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
     * provides access to OpenGL extensions. This function does not follow the
     * Equalizer naming conventions, since GLEW uses a function of this name to
     * automatically resolve OpenGL function entry points. Therefore, any
     * supported GL function can be called directly from an initialized
     * SystemWindow.
     *
     * @return the extended OpenGL function table for the window's OpenGL
     *         context.
     * @version 1.0
     */
    virtual const GLEWContext* glewGetContext() const { return 0; }

    /**
     * Send a window error event to the application node.
     *
     * @param error the error code.
     * @version 1.7.1
     */
    EQ_API EventOCommand sendError( const uint32_t error );

    /** Process an event. @version 1.0 */
    EQ_API virtual bool processEvent( const Event& event );

    /**
     * Set the window's pixel viewport wrt its parent pipe.
     *
     * @param pvp the viewport in pixels.
     * @version 1.7.2
     */
    EQ_API void setPixelViewport( const PixelViewport& pvp );

    /**
     * @return the window's pixel viewport wrt the parent pipe.
     * @version 1.7.2
     */
    EQ_API const PixelViewport& getPixelViewport() const;

    /**
     * @internal
     * @return the OpenGL texture format corresponding to the window's color
     *         drawable configuration
     */
    EQ_API uint32_t getColorFormat() const;

    /** Set the window's name. @version 1.7.2 */
    EQ_API void setName( const std::string& name );

    /** @return the window's name. @version 1.7.2 */
    EQ_API const std::string& getName() const;

    /** @return the value of an integer attribute. @version 1.7.2 */
    EQ_API int32_t getIAttribute( const WindowSettings::IAttribute attr ) const;

    /**
     * @return the window with which this window shares the GL context.
     * @version 1.7.2
     */
    EQ_API const SystemWindow* getSharedContextWindow() const;

private:
    NotifierInterface& _parent;
    WindowSettings _settings;
};
}


#endif // EQ_SYSTEM_WINDOW_H
