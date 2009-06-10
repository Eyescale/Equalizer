
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

#ifndef EQ_PIPE_H
#define EQ_PIPE_H

#ifdef EQUALIZER_EXPORTS
   // We need to instantiate a Monitor< State > when compiling the library,
   // but we don't want to have <pthread.h> for a normal build, hence this hack
#  include <pthread.h>
#endif
#include <eq/base/monitor.h>

#include <eq/client/eye.h>            // Eye enum
#include <eq/client/pixelViewport.h>  // member
#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum
#include <eq/client/windowSystem.h>   // WGLEWContext

#include <eq/net/object.h>
#include <eq/net/objectVersion.h>

#include <eq/base/refPtr.h>
#include <eq/base/lock.h>
#include <eq/base/thread.h>

namespace eq
{
    class CommandQueue;
    class OSPipe;
    class PipeVisitor;

    /**
     * A Pipe represents a graphics card (GPU) on a Node.
     *
     * All Pipe, Window and Channel task methods are executed in a separate
     * eq::base::Thread, in parallel with all other pipes in the system, unless
     * the pipe is non-threaded, in which case the tasks are executed on the
     * Node's main thread.
     */
    class Pipe : public net::Object
    {
    public:
        /** Constructs a new pipe. */
        EQ_EXPORT Pipe( Node* parent );

        /** Destructs the pipe. */
        EQ_EXPORT virtual ~Pipe();

        /** @name Data Access. */
        //@{
        EQ_EXPORT net::CommandQueue* getPipeThreadQueue();
        Node*       getNode()       { return _node; }
        const Node* getNode() const { return _node; }

        EQ_EXPORT Config* getConfig();
        EQ_EXPORT const Config* getConfig() const;

        EQ_EXPORT ClientPtr getClient();
        EQ_EXPORT ServerPtr getServer();

        const WindowVector& getWindows() const { return _windows; }

        const std::string& getName() const { return _name; }

        /** 
         * Return the set of tasks this pipe's channels might execute in the
         * worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @warning Not finalized, might change in the future.
         * @return the tasks.
         */
        uint32_t getTasks() const { return _tasks; }

        bool isThreaded() const { return ( _thread != 0 ); }
        uint32_t getCurrentFrame()  const { return _currentFrame; }
        EQ_EXPORT uint32_t getFinishedFrame() const;

        /** 
         * Traverse this pipe and all children using a pipe visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQ_EXPORT VisitorResult accept( PipeVisitor& visitor );

        /**
         * Set the pipes's pixel viewport.
         *
         *  Used from _osPipe calls
         *
         * @param pvp the viewport in pixels.
         */
         void setPixelViewport( const eq::PixelViewport& pvp ){ _pvp = pvp; }

        /** 
         * @return the pipe's pixel viewport
         */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /**
         * Returns the port number of this pipe.
         * 
         * The port number identifies the X server for systems using the
         * X11/GLX window system. It currently has no meaning on other systems.
         *
         * @return the port number of this pipe, or
         *         <code>EQ_UNDEFINED_UINT32</code>.
         */
        uint32_t getPort() const { return _port; }

        /** 
         * Returns the device number of this pipe.
         * 
         * The device number identifies the X screen for systems using the
         * X11/GLX window system, or the number of the virtual screen for the
         * AGL window system. On Windows systems it identifies the graphics
         * adapter. Normally the device identifies a GPU.
         *
         * @return the device number of this pipe, or 
         *         <code>EQ_UNDEFINED_UINT32</code>.
         */
        uint32_t getDevice() const { return _device; }

        /** 
         * Return the window system used by this pipe. 
         * 
         * The return value is quaranteed to be constant for an initialized
         * pipe, that is, the window system is determined using
         * selectWindowSystem() before the pipe init method is executed.
         * 
         * @return the window system used by this pipe.
         */
        WindowSystem getWindowSystem() const { return _windowSystem; }

        /** @return the time in ms elapsed since the frame started. */
        EQ_EXPORT int64_t getFrameTime() const;

        /** @return the generic WGL function table for the pipe. */
        EQ_EXPORT WGLEWContext* wglewGetContext();
        //@}

        /**
         * @name Operations
         */
        //@{
        /** 
         * Get an assembly frame.
         * 
         * @param frameVersion the frame's identifier and version.
         * @param eye the current eye pass.
         * @return the frame.
         * @internal
         */
        Frame* getFrame( const net::ObjectVersion& frameVersion, 
                         const Eye eye );

        /** @internal Clear the frame cache and delete all frames. */
        void flushFrames();

        /** @return if the window is made current */
        bool isCurrent( const Window* window ) const;

        /** Set the window as current window. */
        void setCurrent( const Window* window ) const;

        /** @internal @return the view for the given identifier and version. */
        View* getView( const net::ObjectVersion& viewVersion );
        //@}

        /** Wait for the pipe to be exited. */
        EQ_EXPORT void waitExited() const;
        EQ_EXPORT bool isRunning() const;
        
        /** 
         * Wait for a frame to be finished.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrame()
         */
        EQ_EXPORT void waitFrameFinished( const uint32_t frameNumber ) const;

        /** 
         * Wait for a frame to be released locally.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrameLocal()
         */
        EQ_EXPORT void waitFrameLocal( const uint32_t frameNumber ) const;

        /** Start the pipe thread. */
        void startThread();

        /** Wait for the pipe thread to exit. */
        void joinThread();

        /** 
         * @name Interface to and from the OSPipe, the window-system specific 
         *       pieces for a pipe.
         */
        //@{
        /**
         * Set the OS-specific pipe.
         * 
         * The OS-specific pipe implements the window-system-dependent part.
         * The os-specific pipe has to be initialized.
         */
        void setOSPipe( OSPipe* pipe )  { _osPipe = pipe; }

        const OSPipe* getOSPipe() const { return _osPipe; }
              OSPipe* getOSPipe()       { return _osPipe; }

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
        void setErrorMessage( const std::string& message ) { _error = message; }
        //@}

    protected:
        friend class Node;

        /** @name Actions */
        //@{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         */
        EQ_EXPORT void startFrame( const uint32_t frameNumber );

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         */
        EQ_EXPORT void releaseFrame( const uint32_t frameNumber );

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * The synchronization is released only for this frame, not for
         * previous, possible yet unreleased frames.
         * 
         * The synchronization is released only for this frame, not for
         * previous, possible yet unreleased frames.
         * 
         * @param frameNumber the frame to release.
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
         * Tests wether a particular windowing system is supported by this pipe
         * and all its windows.
         * 
         * @param system the window system to test.
         * @return <code>true</code> if the window system is supported,
         *         <code>false</code> if not.
         */
        EQ_EXPORT virtual bool supportsWindowSystem( const WindowSystem system )
                                   const;

        /** 
         * Return the window system to be used by this pipe.
         * 
         * This function determines which of the supported windowing systems is
         * used by this pipe instance. 
         * 
         * @return the window system currently used by this pipe.
         */
        EQ_EXPORT virtual WindowSystem selectWindowSystem() const;

        /** 
         * Initialises this pipe.
         * 
         * @param initID the init identifier.
         */
        EQ_EXPORT virtual bool configInit( const uint32_t initID );

        /** 
         * Exit this pipe.
         */
        EQ_EXPORT virtual bool configExit();

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of pipe-specific data, for example updating the rendering
         * engine. Waits for the node to start the frame, unless the thread
         * model is async. If the thread model is async, the local
         * synchronization is released immediately.
         *
         * This method has to call startFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @sa Config::beginFrame(), Node::startFrame(), 
         *     Node::waitFrameStarted()
         */
        EQ_EXPORT virtual void frameStart( const uint32_t frameID, 
                                           const uint32_t frameNumber );

        /**
         * Finish rendering a frame.
         *
         * Called once at the end of each frame, to do per-frame updates of
         * pipe-specific data, for example updating the rendering
         * engine. Releases the local synchronization if the thread model is
         * local_sync. Always releases the global synchronization for this pipe.
         *
         * This method has to call releaseFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finish.
         */
        EQ_EXPORT virtual void frameFinish( const uint32_t frameID, 
                                            const uint32_t frameNumber );

        /** 
         * Finish drawing.
         * 
         * Called once per frame after the last draw operation. Releases the
         * local synchronization if the thread model is draw_sync (the default).
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finished with draw.
         */
        EQ_EXPORT virtual void frameDrawFinish( const uint32_t frameID, 
                                                const uint32_t frameNumber );

        /** @name Configuration. */
        //@{
        /** 
         * Enable or disable automatic or external OS event dispatch for the
         * pipe thread.
         *
         * @return true if Equalizer shall dispatch OS events, false if the
         *         application dispatches OS events.
         * @sa Event handling documentation on website.
         */
        virtual bool useMessagePump() { return true; }
        //@}

        /** @sa net::Object::attachToSession. */
        EQ_EXPORT virtual void attachToSession( const uint32_t id, 
                                                const uint32_t instanceID, 
                                                net::Session* session );

    private:
        //-------------------- Members --------------------
        /** The reason for the last error. */
        std::string _error;

        /** Window-system specific functions class */
        OSPipe *_osPipe;

        /** The parent node. */
        Node* const    _node;

        /** The name. */
        std::string    _name;

        /** The windows of this pipe. */
        WindowVector   _windows;

        /** The current window system. */
        WindowSystem _windowSystem;

        /** The size (and location) of the pipe. */
        PixelViewport _pvp;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The display (GLX) or ignored (Win32, AGL). */
        uint32_t _port;

        /** The screen (GLX), GPU (Win32) or virtual screen (AGL). */
        uint32_t _device;

        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZING,
            STATE_RUNNING
        };
        /** The configInit/configExit state. */
        base::Monitor< State > _state;

        /** The last started frame. */
        uint32_t _currentFrame;

        /** The number of the last finished frame. */
        base::Monitor< uint32_t > _finishedFrame;

        /** The number of the last locally unlocked frame. */
        base::Monitor<uint32_t> _unlockedFrame;

        /** The running per-frame statistic clocks. */
        std::deque< int64_t > _frameTimes;
        base::Lock            _frameTimeMutex;

        /** The base time for the currently active frame. */
        int64_t _frameTime;

        /** The time spent waiting since the last frame start. */
        int64_t _waitTime;

        typedef stde::hash_map< uint32_t, Frame* > FrameHash;
        /** All assembly frames used by the pipe during rendering. */
        FrameHash _frames;

        typedef stde::hash_map< uint32_t, View* > ViewHash;
        /** All views used by the pipe's channels during rendering. */
        ViewHash _views;

        /** The pipe thread. */
        class PipeThread : public base::Thread
        {
        public:
            PipeThread( Pipe* pipe ) 
                    : _pipe( pipe )
                {}
            
            virtual void* run(){ return _pipe->_runThread(); }

        private:
            Pipe* _pipe;
        };
        PipeThread* _thread;

        /** The receiver->pipe thread command queue. */
        eq::CommandQueue*   _pipeThreadQueue;

        /** The last window made current. */
        const mutable Window* _currentWindow;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        //-------------------- Methods --------------------
        void* _runThread();
        void _setupCommandQueue();

        friend class Window;
        void _addWindow( Window* window );
        void _removeWindow( Window* window );
        Window* _findWindow( const uint32_t id );

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /** @internal Release the views not used for some revisions. */
        void _releaseViews();

        /** @internal Clear the view cache and release all views. */
        void _flushViews();

        /* The command functions. */
        net::CommandResult _cmdCreateWindow( net::Command& command );
        net::CommandResult _cmdDestroyWindow( net::Command& command );
        net::CommandResult _cmdConfigInit( net::Command& command );
        net::CommandResult _cmdConfigExit( net::Command& command );
        net::CommandResult _cmdFrameStartClock( net::Command& command );
        net::CommandResult _cmdFrameStart( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
        net::CommandResult _cmdFrameDrawFinish( net::Command& command );

        CHECK_THREAD_DECLARE( _pipeThread );
    };
}

#endif // EQ_PIPE_H

