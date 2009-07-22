
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include <eq/client/objectManager.h> // member
#include <eq/client/pixelViewport.h> // member
#include <eq/client/renderContext.h> // member
#include <eq/client/types.h>
#include <eq/client/visitorResult.h> // enum

#include <eq/util/bitmapFont.h>      // member
#include <eq/net/object.h>           // base class

namespace eq
{
    class OSWindow;
    class WindowVisitor;
    struct Event;

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
     * between them for optimal GPU memory usage. Please note that each window
     * might have it's own OpenGL command buffer, thus glFlush is needed
     * to synchronize the state of OpenGL objects between windows. Therefore,
     * Equalizer calls flush() at the end of each frame for each window.
     */
    class Window : public net::Object
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
        class ObjectManager : public eq::ObjectManager< const void* >
        {
        public:
            ObjectManager( Window* window ) 
                    : eq::ObjectManager<const void *>(window->glewGetContext( ))
                    , _smallFont( window )
                    , _mediumFont( window )
                {}
            ObjectManager( Window* window, ObjectManager* shared ) 
                    : eq::ObjectManager<const void *>(window->glewGetContext(),
                                                      shared )
                    , _smallFont( window )
                    , _mediumFont( window )
                {}
            virtual ~ObjectManager(){}
            
            /** @return A generic bitmap font renderer */
            const util::BitmapFont& getDefaultFont() const { return _smallFont;}

            /** @return A medium-sized bitmap font renderer */
            const util::BitmapFont& getMediumFont() const { return _mediumFont;}

        private:
            util::BitmapFont _smallFont;
            util::BitmapFont _mediumFont;

            friend class Window;
        };

        /** Constructs a new window. */
        EQ_EXPORT Window( Pipe* parent );

        /** Destructs the window. */
        EQ_EXPORT virtual ~Window();

        /** @name Data Access */
        //@{
        EQ_EXPORT net::CommandQueue* getPipeThreadQueue();

        /** @return the pipe of this window. */
        const Pipe* getPipe() const { return _pipe; }
        Pipe*       getPipe()       { return _pipe; }

        EQ_EXPORT const Node* getNode() const; 
        EQ_EXPORT Node*       getNode();

        EQ_EXPORT const Config* getConfig() const;
        EQ_EXPORT Config*       getConfig();

        EQ_EXPORT ClientPtr getClient();
        EQ_EXPORT ServerPtr getServer();

        const ChannelVector& getChannels() { return _channels; }

        const std::string& getName() const { return _name; }

        bool isRunning() const { return (_state == STATE_RUNNING); }

        /** 
         * Return the set of tasks this window's channels might execute in the
         * worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @warning Not finalized, might change in the future.
         * @return the tasks.
         */
        uint32_t getTasks() const { return _tasks; }

        /** 
         * Traverse this window and all children using a window visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQ_EXPORT VisitorResult accept( WindowVisitor& visitor );

        /** 
         * Set the window with which this window shares the OpenGL context,
         * defaults to the first window of the pipe.
         */
        void setSharedContextWindow( Window* sharedContextWindow )
            { _sharedContextWindow = sharedContextWindow; }

        /** @return the window with which this window shares the GL context */
        const Window* getSharedContextWindow() const 
            { return _sharedContextWindow; }

        /** @return the window with which this window shares the GL context */
        Window* getSharedContextWindow() 
            { return _sharedContextWindow; }


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
        EQ_EXPORT GLEWContext* glewGetContext();

        /** @return the generic WGL function table for the window's pipe. */
        EQ_EXPORT WGLEWContext* wglewGetContext();

        /** @return information about the current drawable. */
        const DrawableConfig& getDrawableConfig() const 
            { return _drawableConfig; }

        /** @return the window's object manager instance. */
        ObjectManager* getObjectManager() { return _objectManager; }
        const ObjectManager* getObjectManager() const { return _objectManager; }

        /** 
         * Set the window's pixel viewport wrt its parent pipe.
         *
         * Updates the fractional viewport accordingly.
         * 
         * @param pvp the viewport in pixels.
         */
        EQ_EXPORT void setPixelViewport( const PixelViewport& pvp );
        
        /** 
         * @return the window's pixel viewport
         */
        EQ_EXPORT const PixelViewport& getPixelViewport() const { return _pvp; }

        /** 
         * @return the window's fractional viewport.
         */
        const Viewport& getViewport() const { return _vp; }

        /** Add a channel's rendering context to the current frame's list */
        void addRenderContext( const RenderContext& context );

        /** Get the last rendering context at the x, y position. */
        EQ_EXPORT bool getRenderContext( const int32_t x, const int32_t y,
                                         RenderContext& context ) const;
        //@}

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in window.cpp
        /** Window (visual) attributes, used during configInit(). */
        enum IAttribute
        {
            IATTR_HINT_STEREO,           //!< Active Stereo
            IATTR_HINT_DOUBLEBUFFER,     //!< Front and back buffer
            IATTR_HINT_FULLSCREEN,       //!< Fullscreen drawable
            IATTR_HINT_DECORATION,       //!< Window decorations
            IATTR_HINT_SWAPSYNC,         //!< Swap sync on vertical retrace
            IATTR_HINT_DRAWABLE,         //!< Drawable type
            IATTR_HINT_STATISTICS,       //!< Statistics gathering hint
            IATTR_HINT_SCREENSAVER,      //!< Screensaver (de)activation (WGL)
            IATTR_PLANES_COLOR,          //!< No of per-component color planes
            IATTR_PLANES_ALPHA,          //!< No of alpha planes
            IATTR_PLANES_DEPTH,          //!< No of z-buffer planes
            IATTR_PLANES_STENCIL,        //!< No of stencil planes
            IATTR_PLANES_ACCUM,          //!< No of accumulation buffer planes
            IATTR_PLANES_ACCUM_ALPHA,    //!< No of alpha accum buffer planes
            IATTR_PLANES_SAMPLES,        //!< No of multisample (AA) planes
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };

        EQ_EXPORT void setIAttribute( const IAttribute attr,
                                      const int32_t value );
        EQ_EXPORT int32_t  getIAttribute( const IAttribute attr ) const;
        EQ_EXPORT static const std::string& getIAttributeString(
                                                const IAttribute attr );
        //@}

        /** @name Actions */
        //@{
        /** Flush outstanding rendering requests. */
        virtual void flush() const { glFlush(); } 
        /** Finish outstanding rendering requests. */
        virtual void finish() const { glFinish(); }
        //@}

        /** 
         * @name Interface to and from the OSWindow, the window-system specific 
         *       pieces for a Window.
         */
        //@{
        /**
         * Set the OS-specific window.
         * 
         * The OS-specific window implements the window-system-dependent part,
         * e.g., the drawable creation. This window forwards certain calls, 
         * e.g., swapBuffers() to the OS window. The os-specific window has to
         * be initialized.
         */
        EQ_EXPORT void setOSWindow( OSWindow* window );

        const OSWindow* getOSWindow() const { return _osWindow; }
        OSWindow*       getOSWindow()       { return _osWindow; }

        //@}

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
        EQ_EXPORT void setErrorMessage( const std::string& message );
        //@}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

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
        EQ_EXPORT virtual bool processEvent( const Event& event );

        //@}

        /** Returns averaged FPS count (averaging is not longer than 2 sec) */
        double getFPS() const { return _avgFPS; }

        /* Draw FPS count */
        EQ_EXPORT virtual void drawFPS() const;

        /** @return the internal color type */
        int getColorType();
        
        /** @return true if FBO is used */
        EQ_EXPORT bool isFBOWindow();

    protected:
        friend class Pipe;

        EQ_EXPORT virtual void attachToSession( const uint32_t id, 
                                                const uint32_t instanceID, 
                                                net::Session* session );

        /** @name Actions */
        //@{
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
        //@}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialize this window.
         * 
         * @param initID the init identifier.
         */
        EQ_EXPORT virtual bool configInit( const uint32_t initID );

        /** 
         * Initialize the OS-specific window.
         *
         * @sa setOSWindow()
         */
        EQ_EXPORT virtual bool configInitOSWindow( const uint32_t initID );

        /** 
         * Initialize the OpenGL state for this window.
         * 
         * @param initID the init identifier.
         * @return <code>true</code> if the initialization was successful,
         *         <code>false</code> if not.
         */
        EQ_EXPORT virtual bool configInitGL( const uint32_t initID );

        /** Exit this window. */
        EQ_EXPORT virtual bool configExit();

        /** De-initialize the OS-specific window. */
        EQ_EXPORT virtual bool configExitOSWindow();

        /** De-initializer the OpenGL state for this window. */
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
         * window-specific data. This method has to call releaseFrame(). The
         * default implementation also flushes all rendering commands. This
         * light-weight call ensures that all outstanding rendering commands for
         * the window's context are being executed in a timely fashion.
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finish.
         */
        EQ_EXPORT virtual void frameFinish( const uint32_t frameID, 
                                            const uint32_t frameNumber );

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

        /** 
         * Make the window's drawable and context current.
         *
         * GL drivers tend to be behave sub-optimally if two many makeCurrent
         * calls happen in a multi-threaded program. When caching is enabled,
         * this method will only call OSWindow::makeCurrent if it has not been
         * done before for this window.
         */
        EQ_EXPORT virtual void makeCurrent( const bool cache = true ) const;

        /** Bind the window's FBO, if it uses one. */
        EQ_EXPORT virtual void bindFrameBuffer() const;

        /** Swap the front and back buffer of the window. */
        EQ_EXPORT virtual void swapBuffers();
        //@}

    private:
        /** The parent pipe. */
        Pipe* const   _pipe;

        /** The name. */
        std::string    _name;

        /** The window sharing the OpenGL context. */
        Window* _sharedContextWindow;

        /** The reason for the last error. */
        std::string    _error;

        /** Window-system specific functions class */
        OSWindow* _osWindow;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The channels of this window. */
        ChannelVector     _channels;

        /** The pixel viewport wrt the pipe. */
        eq::PixelViewport _pvp;

        /** The fractional viewport wrt the pipe. */
        eq::Viewport      _vp;

        /** Drawable characteristics of this window */
        Window::DrawableConfig _drawableConfig;

        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZING,
            STATE_RUNNING
        };
        /** The configInit/configExit state. */
        State _state;

        /** OpenGL object management. */
        ObjectManager* _objectManager;

        /** Used to calculate time of last frame rendering */
        double _lastTime;

        /** averaged FPS value, to prevent FPS counter flickering */
        double _avgFPS;

        /** The list of render context used since the last frame start. */
        std::vector< RenderContext > _renderContexts[2];
        enum 
        {
            FRONT = 0,
            BACK  = 1
        };

        /** The time of the last swap command. */
        int64_t _lastSwapTime;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        friend class Channel;
        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );
        Channel* _findChannel( const uint32_t id );

        bool _setPixelViewport( const PixelViewport& pvp );
        void _setViewport( const Viewport& vp );

        /** Set up object manager during initialization. */
        void _setupObjectManager();
        /** Release object manager. */
        void _releaseObjectManager();

        /** Set up _drawableConfig by querying the current context. */
        void _queryDrawableConfig();

        /** Calculates per-window frame rate */
        void _updateFPS();

        /** Enter the given barrier. */
        void _enterBarrier( net::ObjectVersion barrier );

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /* The command functions. */
        net::CommandResult _cmdCreateChannel( net::Command& command );
        net::CommandResult _cmdDestroyChannel(net::Command& command );
        net::CommandResult _cmdConfigInit( net::Command& command );
        net::CommandResult _cmdConfigExit( net::Command& command );
        net::CommandResult _cmdFrameStart( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
        net::CommandResult _cmdThrottleFramerate( net::Command& command );
        net::CommandResult _cmdFinish( net::Command& command );
        net::CommandResult _cmdBarrier( net::Command& command );
        net::CommandResult _cmdNVBarrier( net::Command& command );
        net::CommandResult _cmdSwap( net::Command& command );
        net::CommandResult _cmdFrameDrawFinish( net::Command& command );

        CHECK_THREAD_DECLARE( _pipeThread );
    };

    std::ostream& operator << ( std::ostream& , const Window::DrawableConfig& );
}

#endif // EQ_WINDOW_H

