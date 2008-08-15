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
        OSWindow( Window* parent );
        virtual ~OSWindow( );

        /** @name Methods forwarded from eq::Window */
        //*{
        virtual bool configInit( ) = 0;

        virtual void configExit( ) = 0;

        virtual void makeCurrent() const = 0;

        virtual void swapBuffers() = 0;

        virtual base::SpinLock* getContextLock() { return 0; }

        virtual bool isInitialized() const = 0;

        /**
         * Initialize the event handling for this window. 
         * 
         * This function initializes the necessary event handler for this
         * window, if required by the window system. Can be overriden by an
         * empty method to disable built-in event handling.
         * @sa EventHandler, eq::Pipe::useMessagePump()
         */
        virtual void initEventHandler() = 0;

        /** De-initialize the event handling for this window. */
        virtual void exitEventHandler() = 0;

        /**
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _glewContext; }

        /** @return information about the current drawable. */
        const Window::DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }

        /** @return the object manager instance. */
        Window::ObjectManager* getObjectManager()
            { return _objectManager.get(); }
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

        /** Set up OpenGL-specific window data, e.g., GLEW. */
        void _initializeGLData();
        /** Clear OpenGL-specific window data. */
        void _clearGLData();

        /** Set up object manager during initialization. */
        void _setupObjectManager();
        /** Release object manager. */
        void _releaseObjectManager();

        /** Set up _drawableConfig by querying the current context. */
        void _queryDrawableConfig();

    private:
        /** Drawable characteristics of this window */
        Window::DrawableConfig _drawableConfig;

        /** Extended OpenGL function entries when window has a context. */
        GLEWContext*   _glewContext;

        /** OpenGL object management. */
        base::RefPtr< Window::ObjectManager > _objectManager;

    };
}


#endif // EQ_OS_WINDOW_H

