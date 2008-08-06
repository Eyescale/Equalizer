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
        OSWindow( Window* parent ) : _window( parent )
            { EQASSERT( _window ); }

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

        /** @name Convenience interface to eq::Window methods */
        //*{

        Pipe* getPipe() const { return _window->getPipe(); }
        int32_t getIAttribute( const Window::IAttribute attr ) const
            { return _window->getIAttribute( attr ); }

        /** @return the generic WGL function table for the window's pipe. */
        WGLEWContext* wglewGetContext() { return _window->wglewGetContext(); }

        /**
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _window->glewGetContext(); }
        //*}

    protected:

        void _initializeGLData() { _window->_initializeGLData(); }
        void _clearGLData()      { _window->_clearGLData();      }

        void _setAbsPVP( const PixelViewport& pvp ) { _window->_pvp = pvp; }

        void _invalidatePVP() { _window->_invalidatePVP(); }

        const PixelViewport& _getAbsPVP( ) const { return _window->_pvp; }

        Window* const _window;
    };
}


#endif // EQ_OS_WINDOW_H

