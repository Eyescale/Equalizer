/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_WINDOW_AGL_H
#define EQ_OS_WINDOW_AGL_H

#include "osWindow.h"

namespace eq
{
    /** The interface defining the minimum functionality for an AGL window. */
    class EQ_EXPORT AGLWindowIF : public OSWindow
    {
    public:
        AGLWindowIF( Window* parent ) : OSWindow( parent ), _carbonHandler( 0 )
            {}
        virtual ~AGLWindow() {}

        /** @return the AGL rendering context. */
        virtual AGLContext getAGLContext() = 0;

        /** @return the carbon window reference. */
        virtual WindowRef getCarbonWindow() = 0;

        /** @return the AGL PBuffer object. */
        virtual AGLPbuffer getAGLPBuffer() = 0;

        /** Used by the AGL event handler to store the event handler ref. */
        EventHandlerRef& getCarbonEventHandler() const { return _carbonHandler; }

    private:
        /** Used by AGLEventHandler to keep the handler for removal. */
        EventHandlerRef _carbonHandler;
    }

    /** Equalizer default implementation of an AGL window */
    class EQ_EXPORT AGLWindow : public AGLWindowIF
    {
    public:
        AGLWindow( Window* parent );
        virtual ~AGLWindow( );

        virtual void configExit( );
        virtual void makeCurrent() const;
        virtual void swapBuffers();

        virtual base::SpinLock* getContextLock() { return &_renderContextLock; }

        virtual bool isInitialized() const;

        virtual void refreshContext();

        /** @return the AGL rendering context. */
        virtual AGLContext getAGLContext() const { return _aglContext; }

        /** @return the carbon window reference. */
        virtual WindowRef getCarbonWindow() const { return _carbonWindow; }

        /** @return the AGL PBuffer object. */
        virtual AGLPbuffer getAGLPBuffer() const { return _aglPBuffer; }

        /** @name Data Access */
        //*{
        /** 
         * Set the AGL rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         * The context has to be set to 0 before it is destroyed.
         *
         * @param context the AGL rendering context.
         */
        virtual void setAGLContext( AGLContext context );

        /** 
         * Set the carbon window to be used with the current AGL context.
         * 
         * @param window the window reference.
         */
        virtual void setCarbonWindow( WindowRef window );
        
        /** 
         * Set the AGL PBUffer object to be used with the current AGL context.
         * 
         * @param pbuffer the PBuffer.
         */
        virtual void setAGLPBuffer( AGLPbuffer pbuffer );
        //*}

        //* @name AGL/Carbon initialization
        //*{
        /** 
         * Initialize this window for the AGL window system.
         *
         * This method first call chooseAGLPixelFormat(), then
         * createAGLContext() with the chosen pixel format, destroys the pixel
         * format using destroyAGLPixelFormat and finally creates a drawable
         * using configInitAGLDrawable().
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInit();

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
         */
        virtual AGLPixelFormat chooseAGLPixelFormat();

        /** 
         * Destroy a pixel format obtained with chooseAGLPixelFormat().
         * 
         * This method uses Global::enterCarbon() and Global::leaveCarbon() to
         * protect the calls to AGL/Carbon.
         *
         * @param pixelFormat a pixel format.
         */
        virtual void destroyAGLPixelFormat( AGLPixelFormat pixelFormat );

        /** 
         * Create an AGL context.
         * 
         * This method does not set the window's AGL context.
         *
         * This method uses Global::enterCarbon() and Global::leaveCarbon() to
         * protect the calls to AGL/Carbon.
         *
         * @param pixelFormat the pixel format for the context.
         * @return the context, or 0 if context creation failed.
         */
        virtual AGLContext createAGLContext( AGLPixelFormat pixelFormat );

        /** 
         * Initialize the window's drawable (fullscreen, pbuffer or window) and
         * bind the AGL context.
         *
         * Sets the window's carbon window on success. Calls
         * configInitAGLFullscreen() or configInitAGLWindow().
         * 
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitAGLDrawable();

        /** 
         * Initialize the window with a fullscreen Carbon window.
         *
         * Sets the window's carbon window on success.
         *
         * This method uses Global::enterCarbon() and Global::leaveCarbon() to
         * protect the calls to AGL/Carbon.
         *
         * @return true if the window was created, false otherwise.
         */
        virtual bool configInitAGLFullscreen();

        /** 
         * Initialize the window with a normal Carbon window.
         *
         * Sets the window's carbon window on success.
         *
         * This method uses Global::enterCarbon() and Global::leaveCarbon() to
         * protect the calls to AGL/Carbon.
         *
         * @return true if the window was created, false otherwise.
         */
        virtual bool configInitAGLWindow();

        /** 
         * Initialize the window with an offscreen AGL PBuffer.
         *
         * Sets the window's AGL PBuffer on success.
         *
         * @return true if the PBuffer was created, false otherwise.
         */
        virtual bool configInitAGLPBuffer(); 
        //*}

    private:
        /** The AGL context. */
        AGLContext   _aglContext;
        /** The carbon window reference. */
        WindowRef    _carbonWindow;
        /** The AGL PBuffer object. */
        AGLPbuffer   _aglPBuffer;

        base::SpinLock _renderContextLock;
    };
}

#endif // EQ_OS_WINDOW_AGL_H

