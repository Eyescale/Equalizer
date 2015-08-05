
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2009, Maxim Makhinya
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

#ifndef EQ_GLX_WINDOW_H
#define EQ_GLX_WINDOW_H

#include <eq/glx/types.h>
#include <eq/glWindow.h>       // base class

namespace eq
{
namespace glx
{
namespace detail { class Window; }

/** The interface defining the minimum functionality for a glX window. */
class WindowIF : public GLWindow
{
public:
    WindowIF( NotifierInterface& parent,
              const WindowSettings& settings ) : GLWindow( parent, settings ) {}
    virtual ~WindowIF() {}

    /** @return the glX rendering context. @version 1.0 */
    virtual GLXContext getGLXContext() const = 0;

    /** @return the X11 drawable ID. @version 1.0 */
    virtual XID getXDrawable() const = 0;

    /** @return X11 display connection. @version 1.0 */
    virtual Display* getXDisplay() = 0;

#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Woverloaded-virtual"
    /** Process the given event. @version 1.5.1 */
    virtual bool processEvent( const WindowEvent& event ) = 0;
#  pragma clang diagnostic pop
};

/** Equalizer default implementation of a glX window */
class Window : public WindowIF
{
public:
    /**
     * Construct a new glX/X11 system window.
     * @version 1.7.2
     */
    Window( NotifierInterface& parent, const WindowSettings& settings,
            Display* xDisplay, const GLXEWContext* glxewContext,
            MessagePump* messagePump );

    /** Destruct this glX window. @version 1.0 */
    virtual ~Window();

    /** @name GLX/X11 initialization */
    //@{
    /**
     * Initialize this window for the glX window system.
     *
     * This method first call chooseGLXFBConfig(), then createGLXContext()
     * with the chosen framebuffer config, and finally creates a drawable
     * using configInitGLXDrawable().
     *
     * @return true if the initialization was successful, false otherwise.
     * @version 1.0
     */
    bool configInit() override;

    /** @version 1.0 */
    void configExit() override;

    /**
     * Choose a GLX framebuffer config based on the window's attributes.
     *
     * The returned FB config has to be freed using XFree().
     *
     * @return a pixel format, or 0 if no pixel format was found.
     * @version 1.0
     */
    virtual GLXFBConfig* chooseGLXFBConfig();

    /**
     * Create a glX context.
     *
     * This method does not set the window's glX context.
     *
     * @param fbConfig the framebuffer config for the context.
     * @return the context, or 0 if context creation failed.
     * @version 1.0
     */
    virtual GLXContext createGLXContext( GLXFBConfig* fbConfig );

    /**
     * Initialize the window's drawable (fullscreen, pbuffer or window) and
     * bind the glX context.
     *
     * Sets the window's X11 drawable on success
     *
     * @param fbConfig the framebuffer config for the context.
     * @return true if the drawable was created, false otherwise.
     * @version 1.0
     */
    virtual bool configInitGLXDrawable( GLXFBConfig* fbConfig );

    /**
     * Initialize the window with a window and bind the glX context.
     *
     * Sets the window's X11 drawable on success
     *
     * @param fbConfig the framebuffer config for the context.
     * @return true if the window was created, false otherwise.
     * @version 1.0
     */
    virtual bool configInitGLXWindow( GLXFBConfig* fbConfig );

    /**
     * Initialize the window with a PBuffer and bind the glX context.
     *
     * Sets the window's X11 drawable on success
     *
     * @param fbConfig the framebuffer config for the context.
     * @return true if the PBuffer was created, false otherwise.
     * @version 1.0
     */
    virtual bool configInitGLXPBuffer( GLXFBConfig* fbConfig );

    /**
     * Register with the pipe's GLXEventHandler, called by setXDrawable().
     * @version 1.0
     */
    EQ_API virtual void initEventHandler();

    /**
     * Deregister with the GLXEventHandler, called by setXDrawable().
     * @version 1.0
     */
    EQ_API virtual void exitEventHandler();
    //@}

    /** @name Data Access. */
    //@{
    /** @return the glX rendering context. @version 1.0 */
    EQ_API GLXContext getGLXContext() const override;

    /**  @return  the X11 drawable ID. @version 1.0 */
    EQ_API XID getXDrawable() const override;

    /** @return the X11 display. @version 1.0 */
    EQ_API Display* getXDisplay() override;

    /** @return the GLXEW context. @version 1.0*/
    EQ_API const GLXEWContext* glxewGetContext() const;

    /**
     * Set the X11 drawable ID for this window.
     *
     * This function should only be called from configInit() or
     * configExit().
     *
     * @param drawable the X11 drawable ID.
     * @version 1.0
     */
    virtual void setXDrawable( XID drawable );

    /**
     * Set the glX rendering context for this window.
     *
     * This function should only be called from configInit() or
     * configExit().
     * The context has to be set to 0 before it is destroyed.
     *
     * @param context the glX rendering context.
     * @version 1.0
     */
    virtual void setGLXContext( GLXContext context );
    //@}

    /** @name Operations. */
    //@{
    /** @version 1.0 */
    void makeCurrent( const bool cache = true ) const override;

    /** @version 1.10 */
    void doneCurrent() const override;

    /** @version 1.0 */
    void swapBuffers() override;

    /** Implementation untested for glX. @version 1.0 */
    void joinNVSwapBarrier( const uint32_t group,
                            const uint32_t barrier ) override;

    /** Unbind a GLX_NV_swap_barrier. @version 1.0 */
    void leaveNVSwapBarrier();

    /** @version 1.5.1 */
    EQ_API bool processEvent( const WindowEvent& event ) override;
    //@}

private:
    detail::Window* const _impl;

    /** Create an unmapped X11 window. */
    XID _createGLXWindow( GLXFBConfig* fbConfig, const PixelViewport& pvp );

    /** Init sync-to-vertical-retrace setting. */
    void _initSwapSync();
};
}
}
#endif // EQ_GLX_WINDOW_H
