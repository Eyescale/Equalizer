
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_OS_WINDOW_GLX_H
#define EQ_OS_WINDOW_GLX_H

#include <eq/client/glWindow.h>       // base class

namespace eq
{
    /** The interface defining the minimum functionality for a glX window. */
    class GLXWindowIF : public GLWindow
    {
    public:
        GLXWindowIF( Window* parent ) : GLWindow( parent ) {}
        virtual ~GLXWindowIF() {}

        /** @return the glX rendering context. @version 1.0 */
        virtual GLXContext getGLXContext() const = 0;

        /** @return the X11 drawable ID. @version 1.0 */
        virtual XID getXDrawable() const = 0;

        /** @return X11 display connection. @version 1.0 */
        virtual Display* getXDisplay() = 0;
    };

    /** Equalizer default implementation of a glX window */
    class GLXWindow : public GLXWindowIF
    {
    public:
        /**
         * Construct a new glX/X11 system window.
         *
         * If no display connection or no GLXEWContext is given (the default),
         * the constructor will try to query the corresponding data from the
         * pipe's system pipe (GLXPipe). The GLXEWContext has to be initialized.
         * @version 1.0
         */
        GLXWindow( Window* parent, Display* xDisplay = 0,
                   GLXEWContext* glxewContext = 0 );

        /** Destruct this glX window. @version 1.0 */
        virtual ~GLXWindow();

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
        virtual bool configInit();

        /** @version 1.0 */
        virtual void configExit();

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
        virtual GLXContext getGLXContext() const { return _glXContext; }

        /**  @return  the X11 drawable ID. @version 1.0 */
        virtual XID getXDrawable() const { return _xDrawable; }

        /** @return the X11 display. @version 1.0 */
        virtual Display* getXDisplay() { return _xDisplay; }

        /** @return the GLXEW context. @version 1.0*/
        GLXEWContext* glxewGetContext() { return _glxewContext; }

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
        virtual void makeCurrent() const;

        /** @version 1.0 */
        virtual void swapBuffers();

        /** Implementation untested for glX. @version 1.0 */
        virtual void joinNVSwapBarrier( const uint32_t group,
                                        const uint32_t barrier );

        /** Unbind a GLX_NV_swap_barrier. @version 1.0 */
        void leaveNVSwapBarrier();
        //@}

    private:
        /** The display connection (maintained by GLXPipe) */
        Display*   _xDisplay;
        /** The X11 drawable ID of the window. */
        XID        _xDrawable;
        /** The glX rendering context. */
        GLXContext _glXContext;
        /** The currently joined swap group. */
        uint32_t _glXNVSwapGroup;

        /** The event handler. */
        GLXEventHandler* _glXEventHandler;

        /** The glX extension pointer table. */
        GLXEWContext* _glxewContext;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        /** Create an unmapped X11 window. */
        XID _createGLXWindow( GLXFBConfig* fbConfig, const PixelViewport& pvp );

        /** Init sync-to-vertical-retrace setting. */
        void _initSwapSync();
    };
}

#endif // EQ_OS_WINDOW_GLX_H

