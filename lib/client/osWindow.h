
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_WINDOW_H
#define EQ_OS_WINDOW_H

#include <eq/client/windowSystem.h>   // WGLew
#include <eq/client/window.h>         // used in inline methods
#include <eq/base/spinLock.h>       
#include <eq/client/frameBufferObject.h>

namespace eq
{
    /**
     * The interface definition for OS-specific windowing code.
     *
     * The OSWindow abstracts all window system specific code and facilitates
     * porting to new windowing systems. Each Windows uses one OSWindow, which
     * is initialized in Window::configInitOSWindow.
     */
    class EQ_EXPORT OSWindow
    {
    public:
        OSWindow( Window* parent );
        virtual ~OSWindow( );
        
        /** @name Methods forwarded from eq::Window */
        //*{
        /** 
         * Initialize this OS window.
         * 
         * This method should take into account all attributes of the parent
         * Window.
         * 
         * @return true if the window was correctly initialized, false
         *         on any error.
         */
        virtual bool configInit( ) = 0;

        /** 
         * De-initialize this OS window.
         * 
         * This function might be called on partially or uninitialized OS
         * windows, and has therefore be tolerant enough to handle this.
         */
        virtual void configExit( ) = 0;

        /** 
         * Make the OS window's rendering context and drawable current.
         *
         * This function invalidates the pipe's make current cache. If this
         * function is not called, Pipe::setCurrent() has to be called
         * appropriately.
         */
        virtual void makeCurrent() const;

        /** Bind the window's FBO, if it uses an FBO drawable. */
        virtual void bindFrameBuffer() const;

        /** Swap the front and back buffer, for doublebuffered drawables. */
        virtual void swapBuffers() = 0;

        virtual base::SpinLock* getContextLock() { return 0; }
        //*}

        /** @name Frame Buffer Object support. */
        //*{
        /** Build and initialize the FBO. */
        bool configInitFBO();

        /** Destroy FBO. */
        void configExitFBO();

        /** @return the FBO of this window, or 0 if no FBO is used. */
        const FrameBufferObject* getFBO() const { return _fbo; }
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
		        
        /** 
         * Get the GLEW context for this window.
         * 
         * The glew context is initialized during window initialization, and
         * provides access to OpenGL extensions. This function does not follow
         * the Equalizer naming conventions, since GLEW uses a function of this
         * name to automatically resolve OpenGL function entry
         * points. Therefore, any supported GL function can be called directly
         * from an initialized OSWindow.
         * 
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _glewContext; }
        const GLEWContext* glewGetContext() const { return _glewContext; }
        
    protected:
        /** The parent eq::Window. */
        Window* const _window;
        
        /** Initialization glew. */
        void _initGlew(); 
        
    private:
        /** Extended OpenGL function entries when window has a context. */
        GLEWContext*   _glewContext; 
        
        bool _glewInitialized ;
        
        /** Frame buffer object for FBO drawables. */		
		FrameBufferObject* _fbo; 
    };
}


#endif // EQ_OS_WINDOW_H

