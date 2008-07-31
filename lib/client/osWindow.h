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
        OSWindow( Window* parent ) : _parent( parent )
        {
            EQASSERT( _parent );
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

    // Interface to Window class

        void setErrorMessage( const std::string& message )
        {
            _parent->setErrorMessage( message );
        }

        int32_t  getIAttribute( const Window::IAttribute attr ) const
        {
            return _parent->getIAttribute( attr );
        }

        /**
         * Initialize the event handling for this window. 
         */
        void initEventHandler()
        {
            _parent->initEventHandler();
        }

        /**
         * De-initialize the event handling for this window. 
         */
        void exitEventHandler()
        {
            _parent->exitEventHandler();
        }

        Pipe* getPipe() const { return _parent->getPipe(); }

        const std::string& getName() const { return _parent->getName(); }

        void setPixelViewport( const PixelViewport& pvp )
        {
            _parent->setPixelViewport( pvp );
        }

        /** @return the generic WGL function table for the window's pipe. */
        WGLEWContext* wglewGetContext() { return _parent->wglewGetContext(); }

        /**
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _parent->glewGetContext(); }

    protected:

        void _initializeGLData() { _parent->_initializeGLData(); }
        void _clearGLData()      { _parent->_clearGLData();      }

        void _setAbsPVP( const PixelViewport& pvp ) { _parent->_pvp = pvp; }

        void _invalidatePVP() { _parent->_invalidatePVP(); }

        const PixelViewport& _getAbsPVP( ) const { return _parent->_pvp; }

    private:
        Window* _parent;
    };
}


#endif // EQ_OS_WINDOW_H

