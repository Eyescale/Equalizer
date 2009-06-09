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

#ifndef EQ_OS_WINDOW_WGL_H
#define EQ_OS_WINDOW_WGL_H

#include <eq/client/osWindow.h>       // base class
#include <eq/client/wglWindowEvent.h> // used in inline method

namespace eq
{
    class WGLEventHandler;

    /** The interface defining the minimum functionality for a WGL window. */
    class EQ_EXPORT WGLWindowIF : public OSWindow
    {
    public:
        WGLWindowIF( Window* parent ) : OSWindow( parent ) {}
        virtual ~WGLWindowIF() {}

        /** @return the WGL rendering context. */
        virtual HGLRC getWGLContext() const = 0;

        /** @return the Win32 window handle. */
        virtual HWND getWGLWindowHandle() const = 0;

        /** @return the Win32 off screen PBuffer handle. */
        virtual HPBUFFERARB getWGLPBufferHandle() const = 0;

        /** @return the Win32 device context used for the current drawable. */
        virtual HDC getWGLDC() const = 0;

        /** @return the Win32 affinity device context, if used. */
        virtual HDC getWGLAffinityDC() { return 0; }

        virtual bool processEvent( const WGLWindowEvent& event )
            { return _window->processEvent( event ); }
    };

    /** Equalizer default implementation of a WGL window */
    class EQ_EXPORT WGLWindow : public WGLWindowIF
    {
    public:
        WGLWindow( Window* parent );
        virtual ~WGLWindow( );

        virtual void configExit( );
        virtual void makeCurrent() const;
        virtual void swapBuffers();
        virtual void joinNVSwapBarrier( const uint32_t group,
                                        const uint32_t barrier );

        /** @return the Win32 window handle. */
        virtual HWND getWGLWindowHandle() const { return _wglWindow; }

        /** @return the Win32 off screen PBuffer handle. */
        virtual HPBUFFERARB getWGLPBufferHandle() const { return _wglPBuffer; }

        /** @return the Win32 device context used for the current drawable. */
        virtual HDC getWGLDC() const { return _wglDC; }

        /** @return the WGL rendering context. */
        virtual HGLRC getWGLContext() const { return _wglContext; }

        /** @return the WGL event handler. */
        const WGLEventHandler* getWGLEventHandler() const 
            { return _wglEventHandler; }

        /** @name Data Access */
        //@{
        /** 
         * Set the Win32 window handle for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param handle the window handle.
         */
        virtual void setWGLWindowHandle( HWND handle );
        
        /** 
         * Set the Win32 off screen pbuffer handle for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param handle the pbuffer handle.
         */
        virtual void setWGLPBufferHandle( HPBUFFERARB handle );

        /** 
         * Set the WGL rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         * The context has to be set to 0 before it is destroyed.
         *
         * @param context the WGL rendering context.
         */
        virtual void setWGLContext( HGLRC context );
        //@}

        /** @name WGL/Win32 initialization */
        //@{
        /** 
         * Initialize this window for the WGL window system.
         *
         * This method first calls createWGLAffinityDC(), then chooses a pixel
         * format with chooseWGLPixelFormat(), then creates a drawable using 
         * configInitWGLDrawable() and finally creates the context using
         * createWGLContext().
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInit();

        /** 
         * Create, if needed, an affinity device context for this window.
         *
         * @return false on error, true otherwise
         */
        virtual bool initWGLAffinityDC();

        /** Destroy the affinity device context. */
        virtual void exitWGLAffinityDC();

        /** @return the affinity device context. */
        virtual HDC getWGLAffinityDC();

        /**
         * Create a device context for the display device of the window.
         *
         * The returned device context has to be released using DeleteDC. The
         * window's error message is set if an error occured.
         *
         * @return the DC, or 0 upon error.
         */
        virtual HDC createWGLDisplayDC();

        /** 
         * Choose a pixel format based on the window's attributes.
         * 
         * Uses the currently set DC (if any) and sets the chosen pixel format
         * on it.
         *
         * @return a pixel format, or 0 if no pixel format was found.
         */
        virtual int chooseWGLPixelFormat();

        /** 
         * Initialize the window's drawable (pbuffer or window) and
         * bind the WGL context.
         *
         * Sets the window handle on success.
         * 
         * @param pixelFormat the window's target pixel format.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitWGLDrawable( int pixelFormat );

        /** 
         * Initialize the window with an on-screen Win32 window.
         *
         * Sets the window handle on success.
         * 
         * @param pixelFormat the window's target pixel format.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitWGLWindow( int pixelFormat );

        /** 
         * Initialize the window with an off-screen WGL PBuffer.
         *
         * Sets the window handle on success.
         * 
         * @param pixelFormat the window's target pixel format.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitWGLPBuffer( int pixelFormat );

        /** Initialize the window for an off-screen FBO */
        virtual bool configInitWGLFBO( int pixelFormat );

        /** 
         * Create a WGL context.
         * 
         * This method does not set the window's WGL context.
         *
         * @return the context, or 0 if context creation failed.
         */
        virtual HGLRC createWGLContext();

        virtual void initEventHandler();
        virtual void exitEventHandler();
        virtual bool processEvent( const WGLWindowEvent& event );
        //@}

    protected:
        /** The type of the Win32 device context. */
        enum WGLDCType
        {
            WGL_DC_NONE,     //!< No device context is set
            WGL_DC_WINDOW,   //!< The WGL DC belongs to a window
            WGL_DC_PBUFFER,  //!< The WGL DC belongs to a PBuffer
            WGL_DC_AFFINITY, //!< The WGL DC is an affinity DC
            WGL_DC_DISPLAY   //!< The WGL DC is a display DC
        };

        /** 
         * Set new device context and release the old DC.
         * 
         * @param dc the new DC.
         * @param type the type of the new DC.
         */
        void setWGLDC( HDC dc, const WGLDCType type );

        /** Unbind a WGL_NV_swap_barrier. */ 
        void leaveNVSwapBarrier();

    private:

        HWND             _wglWindow;
        HPBUFFERARB      _wglPBuffer;
        HGLRC            _wglContext;

        HDC              _wglDC;
        WGLDCType        _wglDCType;
        HDC              _wglAffinityDC;

        WGLEventHandler* _wglEventHandler;
        BOOL             _screenSaverActive;

        uint32_t         _wglNVSwapGroup;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** Create an unmapped WGL window. */
        HWND _createWGLWindow( int pixelFormat, const PixelViewport& pvp );
    };
}

#endif // EQ_OS_WINDOW_WGL_H

