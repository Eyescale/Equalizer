/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_WINDOW_H
#define EQ_OS_WINDOW_H

#include <eq/base/spinLock.h>
#include <eq/client/windowSystem.h>
#include <eq/client/window.h>

namespace eq
{
    /**
     */

    class EQ_EXPORT OSWindow
    {
    public:
        OSWindow( Window* parent ) : _parentWnd( parent )
        {
            EQASSERT( _parentWnd );
        }

        virtual ~OSWindow( ) {}

        virtual bool configInit( ) = 0;

        virtual void configExit( ) = 0;

        virtual void makeCurrent() const = 0;

        virtual void swapBuffers() = 0;

        virtual base::SpinLock* getContextLock() { return 0; }

        virtual bool checkConfigInit() const = 0;

        virtual WindowSystem getWindowSystem() const = 0;

        virtual void refreshContext() { }

        virtual bool isInitialized() const = 0;

        static OSWindow* createOSWindow( Window* parent );


    // Interface to Window class

        void setErrorMessage( const std::string& message )
        {
            _parentWnd->setErrorMessage( message );
        }

        int32_t  getIAttribute( const Window::IAttribute attr ) const
        {
            return _parentWnd->getIAttribute( attr );
        }

        /**
         * Initialize the event handling for this window. 
         */
        void initEventHandler()
        {
            _parentWnd->initEventHandler();
        }

        /**
         * De-initialize the event handling for this window. 
         */
        void exitEventHandler()
        {
            _parentWnd->exitEventHandler();
        }

        Pipe* getPipe() const { return _parentWnd->getPipe(); }

        const std::string& getName() const { return _parentWnd->getName(); }

        void setPixelViewport( const PixelViewport& pvp )
        {
            _parentWnd->setPixelViewport( pvp );
        }

        /** @return the generic WGL function table for the window's pipe. */
        WGLEWContext* wglewGetContext() { return _parentWnd->wglewGetContext(); }

        /**
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _parentWnd->glewGetContext(); }

    protected:

        void _initializeGLData() { _parentWnd->_initializeGLData(); }
        void _clearGLData()      { _parentWnd->_clearGLData();      }

        void _setAbsPVP( const PixelViewport& pvp ) { _parentWnd->_pvp = pvp; }

        void _invalidatePVP() { _parentWnd->_invalidatePVP(); }

        const PixelViewport& _getAbsPVP( ) const { return _parentWnd->_pvp; }

    private:
        Window* _parentWnd;
    };
}


#endif // EQ_OS_WINDOW_H

