/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include <eq/client/objectManager.h> // member
#include <eq/client/pipe.h>          // used in inline functions
#include <eq/client/pixelViewport.h> // member

namespace eq
{
    class Channel;
    class WindowEvent;
    struct RenderContext;

    /**
     * A Window represents an on-screen or off-screen drawable, and manages an
     * OpenGL context.
     *
     * A Window is a child of a Pipe. The task methods for all windows of a pipe
     * are executed sequentially in the same thread, in the order they are
     * stored on the Pipe.
     *
     * The default window initialization methods do initialize all windows of
     * the same Pipe with a shared context, so that OpenGL objects can be reused
     * between them for optimal GPU memory usage. Please not that each window
     * might have it's own OpenGL command buffer, thus glFlush might be needed
     * to synchronize the state of OpenGL objects between windows.
     */
    class EQ_EXPORT Window : public eqNet::Object
    {
    public:
        /** Stores current drawable characteristics. */
        struct DrawableConfig
        {
            int32_t stencilBits;
            int32_t alphaBits;
            float   glVersion;
            bool    stereo;
            bool    doublebuffered;
        };
        
        /** The per-window object manager */
        class ObjectManager : public eq::ObjectManager< const void* >, 
                              public eqBase::Referenced
        {
        public:
            ObjectManager( GLEWContext* glewContext ) 
                    : eq::ObjectManager< const void * >( glewContext ) {}
            virtual ~ObjectManager(){}
        };

        /** 
         * Constructs a new window.
         */
        Window( Pipe* parent );

        /** @name Data Access */
        //*{
        eqNet::CommandQueue* getPipeThreadQueue()
            { return _pipe->getPipeThreadQueue(); }

        /** @return the pipe of this window. */
        Pipe* getPipe() const { return _pipe; }
        Node* getNode() const 
            { return ( _pipe ? _pipe->getNode() : 0 );}
        Config* getConfig() const { return (_pipe ? _pipe->getConfig() : 0);}
        eqBase::RefPtr< Client > getClient() const 
            { return ( _pipe ? _pipe->getClient() : 0 ); }
        eqBase::RefPtr< Server > getServer() const 
            { return ( _pipe ? _pipe->getServer() : 0 ); }
        const ChannelVector& getChannels() const { return _channels; }

        const std::string& getName() const { return _name; }

        /**  @return  the X11 drawable ID. */
        XID getXDrawable() const { return _xDrawable; }

        /** @return the GLX rendering context. */
        GLXContext getGLXContext() const { return _glXContext; }

        /** @return the AGL rendering context. */
        AGLContext getAGLContext() const { return _aglContext; }

        /** @return the carbon window reference. */
        WindowRef getCarbonWindow() const { return _carbonWindow; }

        /** @return the AGL PBuffer object. */
        AGLPbuffer getAGLPBuffer() const { return _aglPBuffer; }

        /** @return the Win32 window handle. */
        HWND getWGLWindowHandle() const { return _wglWindow; }

        /** @return the Win32 off screen PBuffer handle. */
        HPBUFFERARB getWGLPBufferHandle() const { return _wglPBuffer; }

        /** @return the Win32 device context used for the current drawable. */
        HDC getWGLDC() const { return _wglDC; }

        /** @return the WGL rendering context. */
        HGLRC getWGLContext() const { return _wglContext; }

        /** 
         * Set the window with which this window shares the OpenGL context,
         * defaults to the first window of the pipe.
         */
        void setSharedContextWindow( Window* sharedContextWindow )
            { _sharedContextWindow = sharedContextWindow; }

        /**
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _glewContext; }

        /** @return the generic WGL function table for the window's pipe. */
        WGLEWContext* wglewGetContext() { return _pipe->wglewGetContext(); }

        /** @return information about the current drawable. */
        const DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }

        /** @return the window's object manager instance. */
        ObjectManager* getObjectManager() { return _objectManager.get(); }

        /** 
         * Set the window's pixel viewport wrt its parent pipe.
         *
         * Updates the fractional viewport accordingly.
         * 
         * @param pvp the viewport in pixels.
         */
        void setPixelViewport( const PixelViewport& pvp );
        
        /** 
         * @return the window's pixel viewport
         */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /** 
         * @return the window's fractional viewport.
         */
        const Viewport& getViewport() const { return _vp; }

        /** Add a channel's rendering context to the current frame's list */
        void addRenderContext( const RenderContext& context );

        /** Get the last rendering context at the x, y position, or 0. */
        const RenderContext* getRenderContext( const int32_t x,
                                               const int32_t y ) const;
        //*}

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array initialization in window.cpp
        enum IAttribute
        {
            IATTR_HINT_STEREO,
            IATTR_HINT_DOUBLEBUFFER,
            IATTR_HINT_FULLSCREEN,
            IATTR_HINT_DECORATION,
            IATTR_HINT_SWAPSYNC,
            IATTR_HINT_DRAWABLE,
            IATTR_HINT_STATISTICS,
            IATTR_PLANES_COLOR,
            IATTR_PLANES_ALPHA,
            IATTR_PLANES_DEPTH,
            IATTR_PLANES_STENCIL,
            IATTR_ALL
        };
        
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
        //*}

        /** @name Actions */
        //*{
        /** Finish outstanding rendering requests. */
        virtual void finish() const { glFinish(); }
        //*}

    protected:
        /**
         * Destructs the window.
         */
        virtual ~Window();
        friend class Pipe;

        /**
         * @name Attributes
         */
        //*{
        void setIAttribute( const eq::Window::IAttribute attr,
                            const int32_t value )
            { _iAttributes[attr] = value; }
        //*}

        /** @name Data Access */
        //*{
        /** 
         * Set the X11 drawable ID for this window.
         * 
         * This function should only be called from configInit() or 
         * configExit().
         *
         * @param drawable the X11 drawable ID.
         */
        virtual void setXDrawable( XID drawable );

        /** 
         * Set the GLX rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         * The context has to be set to 0 before it is destroyed.
         *
         * @param drawable the GLX rendering context.
         */
        virtual void setGLXContext( GLXContext context );

        /** 
         * Set the AGL rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         * The context has to be set to 0 before it is destroyed.
         *
         * @param drawable the AGL rendering context.
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
         * @param drawable the WGL rendering context.
         */
        virtual void setWGLContext( HGLRC context );
        //*}

        /** @name Actions */
        //*{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         */
        void startFrame( const uint32_t frameNumber ) { /* currently nop */ }

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         */
        void releaseFrame( const uint32_t frameNumber ) { /* currently nop */ }

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         */
        void releaseFrameLocal( const uint32_t frameNumber ) { /* nop */ }
        //*}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //*{

        /** 
         * Initialize this window.
         * 
         * @param initID the init identifier.
         */
        virtual bool configInit( const uint32_t initID );

        /** 
         * Initialize the OpenGL state for this window.
         * 
         * @param initID the init identifier.
         * @return <code>true</code> if the initialization was successful,
         *         <code>false</code> if not.
         */
        virtual bool configInitGL( const uint32_t initID );

        /** 
         * Exit this window.
         */
        virtual bool configExit();
        virtual bool configExitGL() { return true; }
        virtual void configExitGLX();
        virtual void configExitAGL();
        virtual void configExitWGL();
        
        //* @name GLX/X11 initialization
        //*{
        /** 
         * Initialize this window for the GLX window system.
         *
         * This method first call chooseXVisualInfo(), then createGLXContext()
         * with the chosen visual, and finally creates a drawable using
         * configInitGLXDrawable().
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInitGLX();

        /** 
         * Choose a X11 visual based on the window's attributes.
         * 
         * The returned XVisualInfo has to be freed using XFree().
         *  
         * @return a pixel format, or 0 if no pixel format was found.
         */
        virtual XVisualInfo* chooseXVisualInfo();

        /** 
         * Create a GLX context.
         * 
         * This method does not set the window's GLX context.
         *
         * @param visualInfo the visual info for the context.
         * @return the context, or 0 if context creation failed.
         */
        virtual GLXContext createGLXContext( XVisualInfo* visualInfo );

        /** 
         * Initialize the window's drawable (fullscreen, pbuffer or window) and
         * bind the GLX context.
         *
         * Sets the window's X11 drawable on success
         * 
         * @param visualInfo the visual info for the context.
         * @return true if the drawable was created, false otherwise.
         */
        virtual bool configInitGLXDrawable( XVisualInfo* visualInfo );

        /** 
         * Initialize the window with a window and bind the GLX context.
         *
         * Sets the window's X11 drawable on success
         * 
         * @param visualInfo the visual info for the context.
         * @return true if the window was created, false otherwise.
         */
        virtual bool configInitGLXWindow( XVisualInfo* visualInfo );

        /** 
         * Initialize the window with a PBuffer and bind the GLX context.
         *
         * Sets the window's X11 drawable on success
         * 
         * @param visualInfo the visual info for the context.
         * @return true if the PBuffer was created, false otherwise.
         */
        virtual bool configInitGLXPBuffer( XVisualInfo* visualInfo );

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
        virtual bool configInitAGL();

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
        virtual bool configInitWGL();

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

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of window-specific data. This method has to call startFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @sa Config::beginFrame()
         */
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber ) 
            { startFrame( frameNumber ); }

        /**
         * Finish rendering a frame.
         *
         * Called once at the end of each frame, to do per-frame updates of
         * window-specific data.  This method has to call releaseFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finish.
         */
        virtual void frameFinish( const uint32_t frameID, 
                                  const uint32_t frameNumber )
            { releaseFrame( frameNumber ); }

        /** 
         * Finish drawing.
         * 
         * Called once per frame after the last draw operation. Typically
         * releases the local node thread synchronization for this frame.
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finished with draw.
         */
        virtual void frameDrawFinish( const uint32_t frameID, 
                                      const uint32_t frameNumber )
            { releaseFrameLocal( frameNumber ); }

        /** Make the window's drawable and context current. */
        virtual void makeCurrent() const;

        /** Swap the front and back buffer of the window. */
        virtual void swapBuffers();

        /**
         * Initialize the event handling for this window. 
         * 
         * This function initializes the necessary event handler for this
         * window, if required by the window system. Can be overriden by an
         * empty method to disable built-in event handling.
         * @sa EventHandler, eq::Pipe::useMessagePump()
         */
        virtual void initEventHandler();

        /**
         * De-initialize the event handling for this window. 
         */
        virtual void exitEventHandler();

        /** The current event handler, or 0. */
        EventHandler* _eventHandler;

        /** 
         * Process a received event.
         *
         * The task of this method is to update the window as necessary, and 
         * transform the event into an config event to be send to the 
         * application using Config::sendEvent().
         * 
         * @param event the received window system event.
         * @param true when the event was handled, false if not.
         */
        virtual bool processEvent( const WindowEvent& event );
        friend class GLXEventHandler;
        friend class WGLEventHandler;
        friend class AGLEventHandler;
        friend class WindowStatistics;
        //*}


        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the configInit method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //@}

    private:
        /** Drawable characteristics of this window */
        DrawableConfig _drawableConfig;

        /** The window sharing the OpenGL context. */
        Window* _sharedContextWindow;

        /** Extended OpenGL function entries when window has a context. */
        GLEWContext*   _glewContext;

        /** OpenGL object management. */
        eqBase::RefPtr< ObjectManager > _objectManager;

        /** The reason for the last error. */
        std::string    _error;

        /** Window-system specific information. */
        union
        {
            struct
            {
                /** The X11 drawable ID of the window. */
                XID        _xDrawable;
                /** The glX rendering context. */
                GLXContext _glXContext;
            };

            struct
            {
                /** The AGL context. */
                AGLContext   _aglContext;
                /** The carbon window reference. */
                WindowRef    _carbonWindow;
                /** The AGL PBuffer object. */
                AGLPbuffer   _aglPBuffer;
                /** Used by AGLEventHandler to keep the handler for removal. */
                EventHandlerRef _carbonHandler;
            };

            struct
            {
                HWND             _wglWindow;
                HPBUFFERARB      _wglPBuffer;
                HGLRC            _wglContext;
                HDC              _wglDC;
            };
            char _windowFill[32];
        };

        /** The parent pipe. */
        Pipe* const   _pipe;

        /** The name. */
        std::string    _name;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** The channels of this window. */
        ChannelVector     _channels;

        /** The pixel viewport wrt the pipe. */
        eq::PixelViewport _pvp;

        /** The fractional viewport wrt the pipe. */
        eq::Viewport      _vp;

        /** The list of render context used since the last frame start. */
        std::vector< RenderContext > _renderContexts[2];
        enum 
        {
            FRONT = 0,
            BACK  = 1
        };

        /** Used on AGL only */
        eqBase::SpinLock* _renderContextAGLLock;

        friend class Channel;
        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );
        Channel* _findChannel( const uint32_t id );

        bool _setPixelViewport( const PixelViewport& pvp );
        void _setViewport( const Viewport& vp );

        /** Set up _drawableConfig by querying current context. */
        void _queryDrawableConfig();

        /** Set up OpenGL-specific window data, e.g., GLEW. */
        void _initializeGLData();
        /** Clear OpenGL-specific window data. */
        void _clearGLData();

        /** Set up object manager during initialization. */
        void _setupObjectManager();

        /** Release object manager. */
        void _releaseObjectManager();

        /* The command functions. */
        eqNet::CommandResult _cmdCreateChannel( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyChannel(eqNet::Command& command );
        eqNet::CommandResult _cmdConfigInit( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigExit( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameStart( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameFinish( eqNet::Command& command );
        eqNet::CommandResult _cmdBarrier( eqNet::Command& command );
        eqNet::CommandResult _cmdFinish( eqNet::Command& command );
        eqNet::CommandResult _cmdSwap( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameDrawFinish( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& , const Window::DrawableConfig& );
}

#endif // EQ_WINDOW_H

