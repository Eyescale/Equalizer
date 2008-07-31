/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_WINDOW_WGL_H
#define EQ_OS_WINDOW_WGL_H

#include "osWindow.h"

namespace eq
{
    /**
     */
    class EQ_EXPORT WGLWindow: public OSWindow
    {
    public:
        WGLWindow( Window* parent );
        virtual ~WGLWindow( );

        virtual void configExit( );
        virtual void makeCurrent() const;
        virtual void swapBuffers();

        virtual bool checkConfigInit() const;

        virtual bool isInitialized() const;

        virtual WindowSystem getWindowSystem() const
        {
            return WINDOW_SYSTEM_WGL;
        }

        /** @return the Win32 window handle. */
        HWND getWGLWindowHandle() const { return _wglWindow; }

        /** @return the Win32 off screen PBuffer handle. */
        HPBUFFERARB getWGLPBufferHandle() const { return _wglPBuffer; }

        /** @return the Win32 device context used for the current drawable. */
        HDC getWGLDC() const { return _wglDC; }

        /** @return the WGL rendering context. */
        HGLRC getWGLContext() const { return _wglContext; }

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
         * This method first calls getWGLPipeDC(), then chooses a pixel
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
         * @param deleteProc returns the function to be used to dispose the
         *                   device context when it is no longer needed.
         * @return the device context, or 0 when no special device context is 
         *         needed.
         */
        virtual HDC getWGLPipeDC( PFNEQDELETEDCPROC& deleteProc );

        /** 
         * Choose a pixel format based on the window's attributes.
         * 
         * Sets the chosen pixel format on the given device context.
         *
         * @param dc the device context for the pixel format.
         * @return a pixel format, or 0 if no pixel format was found.
         */
        virtual int chooseWGLPixelFormat( HDC dc );

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
        //*}

    private:

        HWND             _wglWindow;
        HPBUFFERARB      _wglPBuffer;
        HGLRC            _wglContext;
        HDC              _wglDC;
    };
}

#endif // EQ_OS_WINDOW_WGL_H

