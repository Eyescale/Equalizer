/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include <eq/client/pipe.h>          // used in inline functions
#include <eq/client/pixelViewport.h> // member

namespace eq
{
    class Channel;
    class WindowEvent;
    struct RenderContext;

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
        
        /** 
         * Constructs a new window.
         */
        Window();

        /** @name Data Access */
        //*{
        /** 
         * Returns the pipe of this window.
         * 
         * @return the pipe of this window. 
         */
        Pipe* getPipe() const { return _pipe; }
        Node* getNode() const 
            { return ( _pipe ? _pipe->getNode() : NULL );}
        Config* getConfig() const { return (_pipe ? _pipe->getConfig() : NULL);}
        eqBase::RefPtr< Client > getClient() const 
            { return ( _pipe ? _pipe->getClient() : NULL ); }
        eqBase::RefPtr< Server > getServer() const 
            { return ( _pipe ? _pipe->getServer() : NULL ); }
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

        /** @return the Win32 window handle. */
        HWND getWGLWindowHandle() const { return _wglWindowHandle; }

        /** @return the WGL rendering context. */
        HGLRC getWGLContext() const { return _wglContext; }

        /**
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _glewContext; }

        /** @return information about the current drawable. */
        const DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }

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
        void setXDrawable( XID drawable );

        /** 
         * Set the GLX rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param drawable the GLX rendering context.
         */
        void setGLXContext( GLXContext context );

        /** 
         * Set the AGL rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param drawable the AGL rendering context.
         */
        void setAGLContext( AGLContext context );

        /** 
         * Set the carbon window to be used with the current AGL context.
         * 
         * @param window the window reference.
         */
        void setCarbonWindow( WindowRef window );
        
        /** 
         * Set the Win32 window handle for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param drawable the window handle.
         */
        void setWGLWindowHandle( HWND handle );
        
        /** 
         * Set the WGL rendering context for this window.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param drawable the WGL rendering context.
         */
        void setWGLContext( HGLRC context );
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
        virtual bool configInitGLX();
        virtual bool configInitAGL();
        virtual bool configInitWGL();

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
         * This function is called from the event thread. The task of this
         * method is to update the window as necessary, and transform the event
         * into an config event to be send to the application using
         * Config::sendEvent().
         * 
         * @param event the received window system event.
         * @param true when the event was handled, false if not.
         */
        virtual bool processEvent( const WindowEvent& event );
        friend class GLXEventHandler;
        friend class WGLEventHandler;
        friend class AGLEventHandler;
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

        /** Extended OpenGL function entries when window has a context. */
        GLEWContext*   _glewContext;

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
                /** Used by AGLEventHandler to keep the handler for removal. */
                EventHandlerRef _carbonHandler;
            };

            struct
            {
                HWND             _wglWindowHandle;
                HGLRC            _wglContext;
            };
            char _windowFill[32];
        };

        /** The parent pipe. */
        friend class Pipe;
        Pipe*        _pipe;

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

        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );
        Channel* _findChannel( const uint32_t id );

        bool _setPixelViewport( const PixelViewport& pvp );
        void _setViewport( const Viewport& vp );

        /** Set up _drawableConfig by querying current context. */
        void _queryDrawableConfig();

        /* The command functions. */
        eqNet::CommandResult _pushCommand( eqNet::Command& command );
        eqNet::CommandResult _cmdCreateChannel( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyChannel(eqNet::Command& command );
        eqNet::CommandResult _reqConfigInit( eqNet::Command& command );
        eqNet::CommandResult _reqConfigExit( eqNet::Command& command );
        eqNet::CommandResult _reqFrameStart( eqNet::Command& command );
        eqNet::CommandResult _reqFrameFinish( eqNet::Command& command );
        eqNet::CommandResult _reqBarrier( eqNet::Command& command );
        eqNet::CommandResult _reqFinish( eqNet::Command& command );
        eqNet::CommandResult _reqSwap( eqNet::Command& command );
        eqNet::CommandResult _reqFrameDrawFinish( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& , const Window::DrawableConfig& );
}

#endif // EQ_WINDOW_H

