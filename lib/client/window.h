
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/drawableConfig.h> // member
#include <eq/client/pixelViewport.h>  // member
#include <eq/client/renderContext.h>  // member
#include <eq/client/types.h>
#include <eq/client/visitorResult.h> // enum

#include <eq/util/bitmapFont.h>      // member
#include <eq/util/objectManager.h>   // member
#include <eq/net/object.h>           // base class

namespace eq
{
    class OSPipe;
    class OSWindow;
    class WindowVisitor;
    struct Event;

    /**
     * A Window represents an on-screen or off-screen drawable, and manages an
     * OpenGL context.
     *
     * A window uses an OSWindow implementation to manage the operating system
     * specific handling of window and context creation.
     *
     * A Window is a child of a Pipe. The task methods for all windows of a pipe
     * are executed in the same pipe thread. All window and subsequent channel
     * task methods are executed in the order the windows are defined on the
     * pipe, with the exception of the swap and finish tasks, which are executed
     * after all windows have been updated. This ensures that all windows of a
     * given pipe swap at the same time.
     *
     * The default window initialization methods initialize all windows of the
     * same pipe with a shared context, so that OpenGL objects can be reused
     * between them for optimal GPU memory usage. The window facilitates OpenGL
     * object management by providing an ObjectManager for allocating and
     * sharing OpenGL objects.
     *
     * Please note that each window potentially has its own OpenGL command
     * buffer, thus glFlush is needed to synchronize the state of OpenGL objects
     * between windows. Therefore, Equalizer calls flush() at the end of each
     * frame for each window.
     */
    class Window : public net::Object
    {
    public:
        /** The per-window object manager. */
        typedef util::ObjectManager< const void* > ObjectManager;

        /** Fonts used for overlays. */
        typedef util::BitmapFont< const void* > Font;

        /** Construct a new window. */
        EQ_EXPORT Window( Pipe* parent );

        /** Destruct the window. */
        EQ_EXPORT virtual ~Window();

        /** @name Data Access */
        //@{
        EQ_EXPORT net::CommandQueue* getPipeThreadQueue(); //!< @internal

        /** @return the Pipe of this window. */
        const Pipe* getPipe() const { return _pipe; }
        /** @return the Pipe of this window. */
        Pipe*       getPipe()       { return _pipe; }

        /** @return the Node of this window. */
        EQ_EXPORT const Node* getNode() const; 
        /** @return the Node of this window. */
        EQ_EXPORT Node*       getNode();

        /** @return the Config of this window. */
        EQ_EXPORT const Config* getConfig() const;
        /** @return the Config of this window. */
        EQ_EXPORT Config*       getConfig();

        /** @return the Client of this window. */
        EQ_EXPORT ClientPtr getClient();

        /** @return the Server of this window. */
        EQ_EXPORT ServerPtr getServer();

        /** @return a vector of all channels of this window. */
        const ChannelVector& getChannels() const { return _channels; }

        /** @return the name of this window. */
        const std::string& getName() const { return _name; }

        /** @return true if this window is running, false otherwise. */
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

        /** Const-version of accept(). */
        EQ_EXPORT VisitorResult accept( WindowVisitor& visitor ) const;

        /** 
         * Set the window's pixel viewport wrt its parent pipe.
         *
         * Updates the fractional viewport of the window and its channels
         * accordingly.
         * 
         * @param pvp the viewport in pixels.
         */
        EQ_EXPORT void setPixelViewport( const PixelViewport& pvp );
        
        /** 
         * @return the window's pixel viewport
         */
        EQ_EXPORT const PixelViewport& getPixelViewport() const { return _pvp; }

        /** @return the window's fractional viewport. */
        const Viewport& getViewport() const { return _vp; }

        /** 
         * Get the last rendering context at the x, y position.
         *
         * If no render context is found on the given position, false is
         * returned and context is not modified.
         *
         * @return true if a render context was found, false otherwise.
         */
        EQ_EXPORT bool getRenderContext( const int32_t x, const int32_t y,
                                         RenderContext& context ) const;

        /** @return the window's average framerate */
        float getFPS() const { return _avgFPS; }
        //@}

        /** @name OpenGL context handling and sharing */
        //@{
        /** 
         * Set the window with which this window shares the OpenGL context.
         * 
         * By default it is set to the first window of the pipe in the
         * window's constructor. The shared context window is used during
         * initialization to setup the OpenGL context and ObjectManager.
         */
        void setSharedContextWindow( Window* sharedContextWindow )
            { _sharedContextWindow = sharedContextWindow; }

        /** @return the window with which this window shares the GL context */
        const Window* getSharedContextWindow() const
            { return _sharedContextWindow; }

        /** @return the window with which this window shares the GL context */
        Window* getSharedContextWindow() { return _sharedContextWindow; }

        /** @return the window's object manager instance. */
        ObjectManager* getObjectManager() { return _objectManager; }

        /** @return the window's object manager instance. */
        const ObjectManager* getObjectManager() const { return _objectManager; }

        /** @return the small bitmap font used for overlays. */
        EQ_EXPORT const Font* getSmallFont();

        /** @return the medium bitmap font used for overlays. */
        EQ_EXPORT const Font* getMediumFont();

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

        /** @return information about the current drawable. */
        const DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }

        /**
         * @return the OpenGL texture format corresponding to the window's color
         *         drawable configuration
         */
        EQ_EXPORT uint32_t getColorFormat();
        //@}

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in window.cpp
        /** 
         * Window attributes.
         *
         * Most of these attributes are used by the OSWindow implementation to
         * configure the window during configInit(). An OSWindow implementation
         * might not respect all attributes, e.g., IATTR_HINT_SWAPSYNC is not
         * implemented by the GLXWindow.
         */
        enum IAttribute
        {
            IATTR_HINT_STEREO,           //!< Active stereo
            IATTR_HINT_DOUBLEBUFFER,     //!< Front and back buffer
            IATTR_HINT_FULLSCREEN,       //!< Fullscreen drawable
            IATTR_HINT_DECORATION,       //!< Window decorations
            IATTR_HINT_SWAPSYNC,         //!< Swap sync on vertical retrace
            IATTR_HINT_DRAWABLE,         //!< Window, pbuffer or FBO
            IATTR_HINT_STATISTICS,       //!< Statistics gathering hint
            IATTR_HINT_SCREENSAVER,      //!< Screensaver (de)activation (WGL)
            IATTR_PLANES_COLOR,          //!< No of per-component color planes
            IATTR_PLANES_ALPHA,          //!< No of alpha planes
            IATTR_PLANES_DEPTH,          //!< No of z-buffer planes
            IATTR_PLANES_STENCIL,        //!< No of stencil planes
            IATTR_PLANES_ACCUM,          //!< No of accumulation buffer planes
            IATTR_PLANES_ACCUM_ALPHA,    //!< No of alpha accum buffer planes
            IATTR_PLANES_SAMPLES,        //!< No of multisample (AA) planes
            IATTR_FILL1,                 //!< Reserved for future extensions
            IATTR_FILL2,                 //!< Reserved for future extensions
            IATTR_ALL
        };

        /** Set a window attribute. */
        EQ_EXPORT void setIAttribute( const IAttribute attr,
                                      const int32_t value );

        /** @return the value of a window attribute. */
        EQ_EXPORT int32_t  getIAttribute( const IAttribute attr ) const;
        /** @return the name of a window attribute. */
        EQ_EXPORT static const std::string& getIAttributeString(
                                                        const IAttribute attr );
        //@}

        /** @name Actions */
        //@{
        /** 
         * Flush outstanding rendering requests.
         *
         * Called at the end of each frame from frameFinish() to ensure timely
         * execution of pending rendering requests.
         */
        virtual void flush() const { glFlush(); }

        /** 
         * Finish outstanding rendering requests.
         *
         * Called before a software swap barrier to ensure that the window will
         * swap directly after the barrier is left.
         */
        virtual void finish() const { glFinish(); }

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

        /** Render the current framerate as on overlay on the window. */
        EQ_EXPORT virtual void drawFPS();
        //@}

        /**  @name OSWindow interface */
        //@{
        /**
         * Set the OS-specific window.
         * 
         * The OSWindow implements the window-system-dependent part, e.g., the
         * drawable creation. This window forwards certain calls, e.g.,
         * swapBuffers(), to the OSWindow. The os-specific window has to be
         * initialized.
         */
        EQ_EXPORT void setOSWindow( OSWindow* window );

        /** @return the OS-specific window implementation. */
        const OSWindow* getOSWindow() const { return _osWindow; }
        /** @return the OS-specific window implementation. */
        OSWindow*       getOSWindow()       { return _osWindow; }

        /** @return the OS-specific pipe implementation. */
        const OSPipe* getOSPipe() const;
        /** @return the OS-specific pipe implementation. */
        OSPipe*       getOSPipe(); 
        //@}

        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when called from the configInit() method.
         *
         * @param message the error message.
         */
        EQ_EXPORT void setErrorMessage( const std::string& message );

        /** @return the current error message. */
        EQ_EXPORT const std::string& getErrorMessage() const;
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

    protected:
        friend class Pipe;

        /** @internal */
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
         * Signal the release of the local synchronization to the parent.
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

        /** De-initialize the OpenGL state for this window. */
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
        DrawableConfig _drawableConfig;

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
        float _lastTime;

        /** averaged FPS value, to prevent FPS counter flickering */
        float _avgFPS;

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

        /** Add a channel's rendering context to the current frame's list */
        void _addRenderContext( const RenderContext& context );

        bool _setPixelViewport( const PixelViewport& pvp );
        void _setViewport( const Viewport& vp );

        /** Set up object manager during initialization. */
        void _setupObjectManager();
        /** Release object manager. */
        void _releaseObjectManager();

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
}

#endif // EQ_WINDOW_H

