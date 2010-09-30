
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
                          , Maxim Makhinya
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
#include <eq/client/glXWindowEvent.h> // used in inline method

namespace eq
{
    /** The interface defining the minimum functionality for a GLX window. */
    class EQ_EXPORT GLXWindowIF : public GLWindow
    {
    public:
        GLXWindowIF( Window* parent ) : GLWindow( parent ) {}
        virtual ~GLXWindowIF() {}

        /** @return the GLX rendering context. */
        virtual GLXContext getGLXContext() const = 0;

        /**  @return  the X11 drawable ID. */
        virtual XID getXDrawable() const = 0;

        virtual bool processEvent( const GLXWindowEvent& event )
            { return _window->processEvent( event ); }

        /** @return GLX display */
        virtual Display* getXDisplay() = 0;
        virtual Display* getXDisplay() const = 0;
    };

    /** Equalizer default implementation of a GLX window */
    class EQ_EXPORT GLXWindow : public GLXWindowIF
    {
    public:
        GLXWindow( Window* parent );
        virtual ~GLXWindow( );

        virtual void configExit( );
        virtual void makeCurrent() const;
        virtual void swapBuffers();
        virtual void joinNVSwapBarrier( const uint32_t group,
                                        const uint32_t barrier );

        /** @name GLX/X11 initialization */
        //@{
        /** 
         * Initialize this window for the GLX window system.
         *
         * This method first call chooseXVisualInfo(), then createGLXContext()
         * with the chosen visual, and finally creates a drawable using
         * configInitGLXDrawable().
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInit();

        /** @return the GLX rendering context. */
        virtual GLXContext getGLXContext() const { return _glXContext; }

        /**  @return  the X11 drawable ID. */
        virtual XID getXDrawable() const { return _xDrawable; }

        /** @return the X11 display */
        virtual Display* getXDisplay();
        virtual Display* getXDisplay() const;
        GLXEWContext* glxewGetContext() { return _glxewContext; }
        
        /**
         * Register with the pipe's GLXEventHandler, called by setXDrawable().
         * @version 1.0
         */
        EQ_EXPORT virtual void initEventHandler();

        /**
         * Deregister with the GLXEventHandler, called by setXDrawable().
         * @version 1.0
         */
        EQ_EXPORT virtual void exitEventHandler();
        //@}

    protected:
        /** 
         * Choose a X11 visual based on the window's attributes.
         * 
         * The returned XVisualInfo has to be freed using XFree().
         *  
         * @return a pixel format, or 0 if no pixel format was found.
         */
        virtual XVisualInfo* chooseXVisualInfo();

        /** 
         * Create a GLX context.
         * 
         * This method does not set the window's GLX context.
         *
         * @param visualInfo the visual info for the context.
         * @return the context, or 0 if context creation failed.
         */
        virtual GLXContext createGLXContext( XVisualInfo* visualInfo );

        /** 
         * Initialize the window's drawable (fullscreen, pbuffer or window) and
         * bind the GLX context.
         *
         * Sets the window's X11 drawable on success
         * 
         * @param visualInfo the visual info for the context.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitGLXDrawable( XVisualInfo* visualInfo );
        
        /** 
         * Initialize the window with a window and bind the GLX context.
         *
         * Sets the window's X11 drawable on success
         * 
         * @param visualInfo the visual info for the context.
         * @return true if the window was created, false otherwise.
         */
        virtual bool configInitGLXWindow( XVisualInfo* visualInfo );

        /** 
         * Initialize the window with a PBuffer and bind the GLX context.
         *
         * Sets the window's X11 drawable on success
         * 
         * @param visualInfo the visual info for the context.
         * @return true if the PBuffer was created, false otherwise.
         */
        virtual bool configInitGLXPBuffer( XVisualInfo* visualInfo );

        /** @name Data Access */
        //@{
        /** 
         * Set the X11 drawable ID for this window.
         * 
         * This function should only be called from configInit() or 
         * configExit().
         *
         * @param drawable the X11 drawable ID.
         */
        virtual void setXDrawable( XID drawable );

        /** 
         * Set the GLX rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         * The context has to be set to 0 before it is destroyed.
         *
         * @param context the GLX rendering context.
         */
        virtual void setGLXContext( GLXContext context );
        //@}

        /** Initialize and join a GLX_NV_swap_barrier. */ 
        bool joinNVSwapBarrier();

        /** Unbind a GLX_NV_swap_barrier. */ 
        void leaveNVSwapBarrier();

        /** Initialize the GLXEW context for this window. */
        void initGLXEW();

        /** De-initialize the GLXEW context. */
        void exitGLXEW() { _glxewInitialized = false; }

    private:
        /** The X11 drawable ID of the window. */
        XID        _xDrawable;
        /** The glX rendering context. */
        GLXContext _glXContext;
        /** The currently joined swap group. */
        uint32_t _glXNVSwapGroup;

        /** The GLX extension pointer table. */
        GLXEWContext* const _glxewContext;

        /** The GLX extension pointer table is initialized. */
        bool _glxewInitialized;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** Create an unmapped X11 window. */
        XID _createGLXWindow( XVisualInfo* visualInfo , 
                              const PixelViewport& pvp );
    };
}

#endif // EQ_OS_WINDOW_GLX_H

