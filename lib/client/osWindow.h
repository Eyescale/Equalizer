/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_WINDOW_H
#define EQ_OS_WINDOW_H

#include <eq/client/windowSystem.h>   // WGLew
#include <eq/client/window.h>         // used in inline methods

#include <eq/base/spinLock.h>       

namespace eq
{
    /**
     */

    class EQ_EXPORT OSWindow
    {
    public:
        OSWindow( Window* parent );
        virtual ~OSWindow( );

        /** @name Methods forwarded from eq::Window */
        //*{
        virtual bool configInit( ) = 0;

        virtual void configExit( ) = 0;

        virtual void makeCurrent() const = 0;

        virtual void swapBuffers() = 0;

        virtual base::SpinLock* getContextLock() { return 0; }
        //*}

        /** @name Convenience interface to eq::Window methods */
        //*{

        const Window* getWindow() const { return _window; }
        const Pipe* getPipe()     const { return _window->getPipe(); }
        const Node* getNode()     const { return _window->getNode(); }
        const Config* getConfig() const { return _window->getConfig(); }

        Window* getWindow() { return _window; }
        Pipe* getPipe()     { return _window->getPipe(); }
        Node* getNode()     { return _window->getNode(); }
        Config* getConfig() { return _window->getConfig(); }

        int32_t getIAttribute( const Window::IAttribute attr ) const
            { return _window->getIAttribute( attr ); }

        /** @return the generic WGL function table for the window's pipe. */
        WGLEWContext* wglewGetContext() { return _window->wglewGetContext(); }
        //*}

    protected:
        /** The parent eq::Window. */
        Window* const _window;
    };
}


#endif // EQ_OS_WINDOW_H

