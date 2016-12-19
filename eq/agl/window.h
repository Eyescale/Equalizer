
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_AGL_WINDOW_H
#define EQ_AGL_WINDOW_H

#include <eq/defines.h>
#include <eq/os.h>

#ifdef AGL
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include <eq/agl/types.h>
#include <eq/glWindow.h>       // base class

namespace eq
{
namespace agl
{
namespace detail { class Window; }

/** The interface defining the minimum functionality for an AGL window. */
class WindowIF : public GLWindow
{
public:
    /** Construct a new AGL window for the given eq::Window. @version 1.0 */
    WindowIF( NotifierInterface& parent, const WindowSettings& settings )
        : GLWindow( parent, settings ) {}

    /** Destruct the AGL window. @version 1.0 */
    virtual ~WindowIF() {}

    /** @return the AGL rendering context. @version 1.0 */
    virtual AGLContext getAGLContext() const = 0;

    /** @return the carbon window reference. @version 1.0 */
    virtual WindowRef getCarbonWindow() const = 0;

    /**
     * @return true if the window does not run in the main thread.
     * @version 1.7.2
     */
    virtual bool isThreaded() const { return false; }

    /** Process a simple event. @return true if the event was handled. */
    virtual bool processEvent( EventType type, EventRef )
        { return GLWindow::processEvent( type ); }

    /** Process a (re)size event. @return true if the event was handled. */
    virtual bool processEvent( EventType type, EventRef, SizeEvent& event )
        { return GLWindow::processEvent( type, event ); }

    /** Process a mouse event. @return true if the event was handled. */
    virtual bool processEvent( EventType type, EventRef, PointerEvent& event )
        { return GLWindow::processEvent( type, event ); }

    /** Process a keyboard event. @return true if the event was handled. */
    virtual bool processEvent( EventType type, EventRef, KeyEvent& event )
        { return GLWindow::processEvent( type, event ); }
};

/** Equalizer default implementation of an AGL window interface. */
class Window : public WindowIF
{
public:
    /**
     * Create a new AGL window for the given eq::Window.
     *
     * If kCGNullDirectDisplay is specified as the displayID (the default),
     * the constructor will try to query the corresponding data from the
     * pipe's system pipe (agl::Pipe).
     * @version 1.0
     */
    EQ_API Window( NotifierInterface& parent, const WindowSettings& settings,
                   const CGDirectDisplayID displayID, const bool threaded );

    /** Destruct the AGL window. @version 1.0 */
    EQ_API virtual ~Window();

    /** @name Data Access */
    //@{
    /**
     * Set the AGL rendering context for this window.
     *
     * This function should only be called from configInit() or
     * configExit(). The context has to be set to 0 before it is destroyed.
     *
     * @param context the AGL rendering context.
     * @version 1.0
     */
    EQ_API virtual void setAGLContext( AGLContext context );

    /**
     * Set the carbon window to be used with the current AGL context.
     *
     * @param window the window reference.
     * @version 1.0
     */
    EQ_API virtual void setCarbonWindow( WindowRef window );

    /** @return the AGL rendering context. @version 1.0 */
    EQ_API AGLContext getAGLContext() const override;

    /** @return the carbon window reference. @version 1.0 */
    EQ_API WindowRef getCarbonWindow() const override;

    /**
     * @return true if the window does not run in the main thread.
     * @version 1.7.2
     */
    EQ_API bool isThreaded() const override;

    /** @return the CG display id used by this window. @version 1.1.1 */
    CGDirectDisplayID getCGDisplayID() const;
    //@}

    /** @name AGL/Carbon initialization */
    //@{
    /**
     * Initialize this window for the AGL window system.
     *
     * This method first call chooseAGLPixelFormat(), then
     * createAGLContext() with the chosen pixel format, destroys the pixel
     * format using destroyAGLPixelFormat() and finally creates a drawable
     * using configInitAGLDrawable().
     *
     * @return true if the initialization was successful, false otherwise.
     * @version 1.0
     */
    EQ_API bool configInit() override;

    /** @version 1.0 */
    EQ_API void configExit() override;

    /**
     * Choose a pixel format based on the window's attributes.
     *
     * The returned pixel format has to be destroyed using
     * destroyAGLPixelFormat() to avoid memory leaks.
     *
     * This method uses Global::enterCarbon() and Global::leaveCarbon() to
     * protect the calls to AGL/Carbon.
     *
     * @return a pixel format, or 0 if no pixel format was found.
     * @version 1.0
     */
    EQ_API virtual AGLPixelFormat chooseAGLPixelFormat();

    /**
     * Destroy a pixel format obtained with chooseAGLPixelFormat().
     *
     * This method uses Global::enterCarbon() and Global::leaveCarbon() to
     * protect the calls to AGL/Carbon.
     *
     * @param format a pixel format.
     * @version 1.0
     */
    EQ_API virtual void destroyAGLPixelFormat( AGLPixelFormat format );

    /**
     * Create an AGL context.
     *
     * This method does not set the window's AGL context.
     *
     * This method uses Global::enterCarbon() and Global::leaveCarbon() to
     * protect the calls to AGL/Carbon.
     *
     * @param format the pixel format for the context.
     * @return the context, or 0 if context creation failed.
     * @version 1.0
     */
    EQ_API virtual AGLContext createAGLContext( AGLPixelFormat format );

    /**
     * Initialize the window's drawable and bind the AGL context.
     *
     * Sets the window's carbon window on success. Calls
     * configInitAGLFullscreen() or configInitAGLWindow().
     *
     * @return true if the drawable was created, false otherwise.
     * @version 1.0
     */
    EQ_API virtual bool configInitAGLDrawable();

    /**
     * Initialize the window with a fullscreen Carbon window.
     *
     * Sets the window's carbon window on success.
     *
     * This method uses Global::enterCarbon() and Global::leaveCarbon() to
     * protect the calls to AGL/Carbon.
     *
     * @return true if the window was created, false otherwise.
     * @version 1.0
     */
    EQ_API virtual bool configInitAGLFullscreen();

    /**
     * Initialize the window with a normal Carbon window.
     *
     * Sets the window's carbon window on success.
     *
     * This method uses Global::enterCarbon() and Global::leaveCarbon() to
     * protect the calls to AGL/Carbon.
     *
     * @return true if the window was created, false otherwise.
     * @version 1.0
     */
    EQ_API virtual bool configInitAGLWindow();

    /**
     * Set up an AGLEventHandler, called by setCarbonWindow().
     * @version 1.0
     */
    EQ_API virtual void initEventHandler();

    /**
     * Destroy the AGLEventHandler, called by setCarbonWindow().
     * @version 1.0
     */
    EQ_API virtual void exitEventHandler();
    //@}

    /** @name Operations. */
    //@{
    /** @version 1.0 */
    EQ_API void makeCurrent( const bool cache = true ) const override;

    /** @version 1.10 */
    EQ_API void doneCurrent() const override;

    /** @version 1.0 */
    EQ_API void swapBuffers() override;

    /** Not implemented for AGL. @version 1.0 */
    EQ_API void joinNVSwapBarrier( const uint32_t group,
                                   const uint32_t barrier ) override;

    EQ_API bool processEvent( EventType type, EventRef eventRef,
                              SizeEvent& event ) override;
    //@}

private:
    detail::Window* const _impl;

    void _initSwapSync( AGLContext context );
    void _resize( const PixelViewport& pvp ) override;
};
}
}
#endif // AGL
#endif // EQ_AGL_WINDOW_H
