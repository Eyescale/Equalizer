/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
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
        virtual bool configInit( ) = 0;

        virtual void configExit( ) = 0;

        virtual void makeCurrent() const;

        virtual void swapBuffers() = 0;

        virtual base::SpinLock* getContextLock() { return 0; }
        //*}

        /** @name Frame Buffer Object support. */
        //*{
        /** Build and initialize the FBO. */
        bool configInitFBO();
        /** Destroy FBO. */
        virtual void configExitFBO();
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
         * from an initialized Window.
         * 
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _glewContext; }
        GLEWContext* glewGetContext() const { return _glewContext; }
        
    protected:
        /** The parent eq::Window. */
        Window* const _window;
        
        /** Initialization glew. */
        void _initGlew(); 
    private:
        
        /** Extended OpenGL function entries when window has a context. */
        GLEWContext*   _glewContext; 
        
        bool _glewInitialized ;
        
        /** For use Frame buffer object. */		
		FrameBufferObject* _fbo; 
       
		
        

    };
}


#endif // EQ_OS_WINDOW_H

