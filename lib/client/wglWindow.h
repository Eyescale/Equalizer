/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_WINDOW_WGL_H
#define EQ_OS_WINDOW_WGL_H

#include <eq/client/osWindow.h>
#include <eq/client/wglWindowEvent.h>

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

        /** @return the Win32 window handle. */
        virtual HWND getWGLWindowHandle() const { return _wglWindow; }

        /** @return the Win32 off screen PBuffer handle. */
        virtual HPBUFFERARB getWGLPBufferHandle() const { return _wglPBuffer; }

        /** @return the Win32 device context used for the current drawable. */
        virtual HDC getWGLDC() const { return _wglDC; }

        /** @return the WGL rendering context. */
        virtual HGLRC getWGLContext() const { return _wglContext; }

        /** @name Data Access */
        //*{
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
        //*}

        //* @name WGL/Win32 initialization
        //*{
        /** 
         * Initialize this window for the WGL window system.
         *
         * This method first calls getWGLAffinityDC(), then chooses a pixel
         * format with chooseWGLPixelFormat(), then creates a drawable using 
         * configInitWGLDrawable() and finally creates the context using
         * createWGLContext().
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInit();

        typedef BOOL (WINAPI * PFNEQDELETEDCPROC)( HDC hdc );

        /** 
         * Get a device context for this window.
         *
         * The returned context has to be deleted using wglDeleteDCNV when it is
         * no longer needed.
         *
         * @return the device context, or 0 when no special device context is
         * needed.
         */
        virtual HDC getWGLAffinityDC();

        /** 
         * Choose a pixel format based on the window's attributes.
         * 
         * Sets the chosen pixel format on the given device context.
         *
         * @param dc the device context for the pixel format.
         * @return a pixel format, or 0 if no pixel format was found.
         */
        virtual int chooseWGLPixelFormat( HDC dc );

        virtual bool configInitWGLFBO( HDC dc, int pixelFormat );
        /** 
         * Initialize the window's drawable (pbuffer or window) and
         * bind the WGL context.
         *
         * Sets the window handle on success.
         * 
         * @param dc the device context of the pixel format.
         * @param pixelFormat the window's target pixel format.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitWGLDrawable( HDC dc, int pixelFormat );

        /** 
         * Initialize the window's with an on-screen Win32 window.
         *
         * Sets the window handle on success.
         * 
         * @param dc the device context of the pixel format, can be 0.
         * @param pixelFormat the window's target pixel format.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitWGLWindow( HDC dc, int pixelFormat );

        /** 
         * Initialize the window's with an off-screen WGL PBuffer.
         *
         * Sets the window handle on success.
         * 
         * @param dc the device context of the pixel format, can be 0.
         * @param pixelFormat the window's target pixel format.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitWGLPBuffer( HDC dc, int pixelFormat );

        /** 
         * Create a WGL context.
         * 
         * This method does not set the window's WGL context.
         *
         * @param dc the device context for the rendering context.
         * @return the context, or 0 if context creation failed.
         */
        virtual HGLRC createWGLContext( HDC dc );

        virtual void initEventHandler();
        virtual void exitEventHandler();
        virtual bool processEvent( const WGLWindowEvent& event );
        //*}

    private:

        HWND             _wglWindow;
        HPBUFFERARB      _wglPBuffer;
        HGLRC            _wglContext;
        HDC              _wglDC;
        
        WGLEventHandler* _eventHandler;
        BOOL             _screenSaverActive;

        bool             _wglIsAffinityDC;
    };
}

#endif // EQ_OS_WINDOW_WGL_H

