
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include <eq/fabric/window.h>         // base class

#include <eq/client/types.h>
#include <eq/client/visitorResult.h> // enum

#include <eq/util/bitmapFont.h>      // member
#include <eq/util/objectManager.h>   // member
#include <eq/fabric/channel.h>       // friend
#include <eq/fabric/renderContext.h> // member


/** @file client/window.h */

namespace eq
{
namespace fabric
{
    template< typename T, typename W > class Channel;
}
    class OSPipe;
    class OSWindow;
    struct Event;

    /**
     * A Window represents an on-screen or off-screen drawable.
     *
     * A drawable is a 2D rendering surface, typically attached to an OpenGL
     * context. A window uses an OSWindow implementation to manage the operating
     * system specific handling of window and context creation.
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
     *
     * @sa fabric::Window
     */
    class Window : public fabric::Window< Pipe, Window, Channel >
    {
    public:
        /** The per-window object manager. @version 1.0 */
        typedef util::ObjectManager< const void* > ObjectManager;

        /** Fonts used for overlays. @version 1.0 */
        typedef util::BitmapFont< const void* > Font;

        /** Construct a new window. @version 1.0 */
        EQ_EXPORT Window( Pipe* parent );

        /** Destruct the window. @version 1.0 */
        EQ_EXPORT virtual ~Window();

        /** @name Data Access */
        //@{
        EQ_EXPORT net::CommandQueue* getPipeThreadQueue(); //!< @internal

        /** @return the Node of this window. @version 1.0 */
        EQ_EXPORT const Node* getNode() const; 

        /** @return the Node of this window. @version 1.0 */
        EQ_EXPORT Node*       getNode();

        /** @return the Config of this window. @version 1.0 */
        EQ_EXPORT const Config* getConfig() const;

        /** @return the Config of this window. @version 1.0 */
        EQ_EXPORT Config*       getConfig();

        /** @return the Client of this window. @version 1.0 */
        EQ_EXPORT ClientPtr getClient();

        /** @return the Server of this window. @version 1.0 */
        EQ_EXPORT ServerPtr getServer();

        /**
         * @return true if this window is running, false otherwise.
         * @version 1.0 
         */
        bool isRunning() const { return (_state == STATE_RUNNING); }

        /** 
         * Get the last rendering context at the x, y position.
         *
         * If no render context is found on the given position, false is
         * returned and context is not modified.
         *
         * @return true if a render context was found, false otherwise.
         * @warning experimental - may not be supported in the future.         
         */
        EQ_EXPORT bool getRenderContext( const int32_t x, const int32_t y,
                                         RenderContext& context ) const;
        //@}

        /** @name OpenGL context handling and sharing */
        //@{
        /** 
         * Set the window with which this window shares the OpenGL context.
         * 
         * By default it is set to the first window of the pipe in the
         * window's constructor. The shared context window is used during
         * initialization to setup the OpenGL context and util::ObjectManager.
         * @version 1.0
         */
        void setSharedContextWindow( Window* sharedContextWindow )
            { _sharedContextWindow = sharedContextWindow; }

        /**
         * @return the window with which this window shares the GL context.
         * @version 1.0
         */
        const Window* getSharedContextWindow() const
            { return _sharedContextWindow; }

        /**
         * @return the window with which this window shares the GL context.
         * @version 1.0
         */
        Window* getSharedContextWindow() { return _sharedContextWindow; }

        /** @return the window's object manager instance. @version 1.0 */
        ObjectManager* getObjectManager() { return _objectManager; }

        /** @return the window's object manager instance. @version 1.0 */
        const ObjectManager* getObjectManager() const { return _objectManager; }

        /**
         * @return a small bitmap font used for overlays.
         * @warning experimental - may not be supported in the future.         
         */
        EQ_EXPORT const Font* getSmallFont();

        /**
         * @return a medium bitmap font used for overlays.
         * @warning experimental - may not be supported in the future.         
         */
        EQ_EXPORT const Font* getMediumFont();

        /** 
         * Get the GLEW context for this window.
         * 
         * The glew context is provided and initialized by the OSWindow, and
         * provides access to OpenGL extensions. This function does not follow
         * the Equalizer naming conventions, since GLEW uses a function of this
         * name to automatically resolve OpenGL function entry
         * points. Therefore, any OpenGL function support by the driver can be
         * directly called from any method of an initialized window.
         * 
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         * @version 1.0
         */
        EQ_EXPORT const GLEWContext* glewGetContext() const;

        /**
         * @internal
         * @return the OpenGL texture format corresponding to the window's color
         *         drawable configuration
         */
        EQ_EXPORT uint32_t getColorFormat() const;
        //@}

        /** @name Actions */
        //@{
        /** 
         * Flush outstanding rendering requests.
         *
         * Called at the end of each frame from frameFinish() to ensure timely
         * execution of pending rendering requests.
         * @version 1.0
         */
        virtual void flush() const { glFlush(); }

        /** 
         * Finish outstanding rendering requests.
         *
         * Called before a software swap barrier to ensure that the window will
         * swap directly after the barrier is left.
         * @version 1.0
         */
        virtual void finish() const { glFinish(); }

        /** Swap the front and back buffer of the window. @version 1.0 */
        EQ_EXPORT virtual void swapBuffers();

        /** Render the current framerate as on overlay. @version 1.0 */
        EQ_EXPORT virtual void drawFPS();

        /** @return the window's average framerate. @version 1.0 */
        float getFPS() const { return _avgFPS; }

        /**
         * @internal
         * Make the window's drawable and context current.
         *
         * GL drivers tend to be behave sub-optimally if two many makeCurrent
         * calls happen in a multi-threaded program. When caching is enabled,
         * this method will only call OSWindow::makeCurrent if it has not been
         * done before for this window on this pipe.
         */
        EQ_EXPORT virtual void makeCurrent( const bool cache = true ) const;

        /** @internal Bind the window's FBO, if it uses one. */
        EQ_EXPORT virtual void bindFrameBuffer() const;
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
         * @version 1.0
         */
        EQ_EXPORT void setOSWindow( OSWindow* window );

        /** @return the OS-specific window implementation. @version 1.0 */
        const OSWindow* getOSWindow() const { return _osWindow; }

        /** @return the OS-specific window implementation. @version 1.0 */
        OSWindow* getOSWindow() { return _osWindow; }

        /** @return the OS-specific pipe implementation. @version 1.0 */
        const OSPipe* getOSPipe() const;

        /** @return the OS-specific pipe implementation. @version 1.0 */
        OSPipe* getOSPipe(); 
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
         * @version 1.0
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
         * @version 1.0
         */
        EQ_EXPORT void startFrame( const uint32_t frameNumber );

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         * @version 1.0
         */
        EQ_EXPORT void releaseFrame( const uint32_t frameNumber );

        /** 
         * Signal the release of the local synchronization to the parent.
         * 
         * @param frameNumber the frame to release.
         * @version 1.0
         */
        EQ_EXPORT void releaseFrameLocal( const uint32_t frameNumber );
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
         * @version 1.0
         */
        EQ_EXPORT virtual bool configInit( const uint32_t initID );

        /** 
         * Initialize the OS-specific window.
         *
         * @sa setOSWindow()
         * @version 1.0
         */
        EQ_EXPORT virtual bool configInitOSWindow( const uint32_t initID );

        /** 
         * Initialize the OpenGL state for this window.
         * 
         * @param initID the init identifier.
         * @return <code>true</code> if the initialization was successful,
         *         <code>false</code> if not.
         * @version 1.0
         */
        EQ_EXPORT virtual bool configInitGL( const uint32_t initID );

        /** Exit this window. @version 1.0 */
        EQ_EXPORT virtual bool configExit();

        /** De-initialize the OS-specific window. @version 1.0 */
        EQ_EXPORT virtual bool configExitOSWindow();

        /** De-initialize the OpenGL state for this window. @version 1.0 */
        virtual bool configExitGL() { return true; }

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of window-specific data. This method has to call startFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @version 1.0
         */
        EQ_EXPORT virtual void frameStart( const uint32_t frameID, 
                                           const uint32_t frameNumber );

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
         * @version 1.0
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
         * @version 1.0
         */
        EQ_EXPORT virtual void frameDrawFinish( const uint32_t frameID,
                                                const uint32_t frameNumber );
        //@}

    private:
        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZING,
            STATE_RUNNING
        };

        /** The window sharing the OpenGL context. */
        Window* _sharedContextWindow;

        /** Window-system specific functions class */
        OSWindow* _osWindow;

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
            char dummy[32];
        };

        /** Add a channel's rendering context to the current frame's list */
        void _addRenderContext( const RenderContext& context );
        friend class Channel;
        
        /** Set up object manager during initialization. */
        void _setupObjectManager();
        /** Release object manager. */
        void _releaseObjectManager();

        /** Calculates per-window frame rate */
        void _updateFPS();

        /** Enter the given barrier. */
        void _enterBarrier( net::ObjectVersion barrier );

        /* The command functions. */
        bool _cmdCreateChannel( net::Command& command );
        bool _cmdDestroyChannel(net::Command& command );
        bool _cmdConfigInit( net::Command& command );
        bool _cmdConfigExit( net::Command& command );
        bool _cmdFrameStart( net::Command& command );
        bool _cmdFrameFinish( net::Command& command );
        bool _cmdThrottleFramerate( net::Command& command );
        bool _cmdFinish( net::Command& command );
        bool _cmdBarrier( net::Command& command );
        bool _cmdNVBarrier( net::Command& command );
        bool _cmdSwap( net::Command& command );
        bool _cmdFrameDrawFinish( net::Command& command );

        EQ_TS_VAR( _pipeThread );
    };
}

#endif // EQ_WINDOW_H

