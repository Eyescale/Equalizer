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
    class OSWindow;
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
    class EQ_EXPORT Window : public net::Object
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
                              public base::Referenced
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
        net::CommandQueue* getPipeThreadQueue()
            { return _pipe->getPipeThreadQueue(); }

        /** @return the pipe of this window. */
        Pipe* getPipe() const { return _pipe; }
        Node* getNode() const 
            { return ( _pipe ? _pipe->getNode() : 0 );}
        Config* getConfig() const { return (_pipe ? _pipe->getConfig() : 0);}
        base::RefPtr< Client > getClient() const 
            { return ( _pipe ? _pipe->getClient() : 0 ); }
        base::RefPtr< Server > getServer() const 
            { return ( _pipe ? _pipe->getServer() : 0 ); }
        const ChannelVector& getChannels() const { return _channels; }

        const std::string& getName() const { return _name; }

        /** 
         * Traverse this window and all children using a window visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        WindowVisitor::Result accept( WindowVisitor* visitor );

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
            IATTR_PLANES_ACCUM,
            IATTR_PLANES_ACCUM_ALPHA,
            IATTR_PLANES_SAMPLES,
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

        /** @name Window system specific implementation */
        //*{
        friend class OSWindow;

        /**
         * Set the OS-specific window.
         * 
         * The OS-specific window implements the window-system-dependent part,
         * e.g., the drawable creation. This window forwards certain calls, 
         * e.g., swapBuffers() to the OS window.
         */
        void setOSWindow( OSWindow* window ) { _osWindow = window; }

        const OSWindow* getOSWindow() const
            { EQASSERT(_osWindow); return _osWindow; }

        OSWindow* getOSWindow() { EQASSERT(_osWindow); return _osWindow; }
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
        void setIAttribute( const IAttribute attr,
                            const int32_t value )
            { _iAttributes[attr] = value; }
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
         * Initialize the OS-specific window.
         *
         * @sa setOSWindow()
         */
        virtual bool configInitOSWindow( const uint32_t initID );

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
         * @return true when the event was handled, false if not.
         */
        virtual bool processEvent( const WindowEvent& event );
        friend class EventHandler;
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
        void _invalidatePVP() { _pvp.invalidate(); }

        /** Drawable characteristics of this window */
        DrawableConfig _drawableConfig;

        /** The window sharing the OpenGL context. */
        Window* _sharedContextWindow;

        /** Extended OpenGL function entries when window has a context. */
        GLEWContext*   _glewContext;

        /** OpenGL object management. */
        base::RefPtr< ObjectManager > _objectManager;

        /** The reason for the last error. */
        std::string    _error;

        /** Window-system specific functions class */
        OSWindow* _osWindow;

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

        friend class Channel;
        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );
        Channel* _findChannel( const uint32_t id );

        bool _setPixelViewport( const PixelViewport& pvp );
        void _setViewport( const Viewport& vp );

        /** Check that the pipe's WS matches the OSWindow's ws. */
        bool _checkWindowType() const;

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
        net::CommandResult _cmdCreateChannel( net::Command& command );
        net::CommandResult _cmdDestroyChannel(net::Command& command );
        net::CommandResult _cmdConfigInit( net::Command& command );
        net::CommandResult _cmdConfigExit( net::Command& command );
        net::CommandResult _cmdFrameStart( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
        net::CommandResult _cmdBarrier( net::Command& command );
        net::CommandResult _cmdFinish( net::Command& command );
        net::CommandResult _cmdSwap( net::Command& command );
        net::CommandResult _cmdFrameDrawFinish( net::Command& command );
    };

    std::ostream& operator << ( std::ostream& , const Window::DrawableConfig& );
}

#endif // EQ_WINDOW_H

