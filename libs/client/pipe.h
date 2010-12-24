
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com> 
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

#include <eq/eye.h>            // Eye enum
#include <eq/os.h>             // WGLEWContext
#include <eq/types.h>
#include <eq/visitorResult.h>  // enum
#include <eq/windowSystem.h>   // enum

#include <eq/fabric/pipe.h>           // base class

#include <co/objectVersion.h>

#include <co/base/lock.h>
#include <co/base/refPtr.h>
#include <co/base/thread.h>

namespace eq
{
    /**
     * A Pipe represents a graphics card (GPU) on a Node.
     *
     * All Pipe, Window and Channel task methods are executed in a separate
     * co::base::Thread, in parallel with all other pipes in the system. An
     * exception are non-threaded pipes, which execute their tasks on the Node's
     * main thread.
     *
     * @sa fabric::Pipe
     */
    class Pipe : public fabric::Pipe< Node, Pipe, eq::Window, PipeVisitor >
    {
    public:
        /** Construct a new pipe. @version 1.0 */
        EQ_API Pipe( Node* parent );

        /** Destruct the pipe. @version 1.0 */
        EQ_API virtual ~Pipe();

        /** @name Data Access. */
        //@{
        EQ_API co::CommandQueue* getPipeThreadQueue(); //!< @internal
        co::CommandQueue* getMainThreadQueue(); //!< @internal
        co::CommandQueue* getCommandThreadQueue(); //!< @internal

        /** @return the parent configuration. @version 1.0 */
        EQ_API Config* getConfig();
        /** @return the parent configuration. @version 1.0 */
        EQ_API const Config* getConfig() const;

        /** @return the parent client node. @version 1.0 */
        EQ_API ClientPtr getClient();

        /** @return the parent server node. @version 1.0 */
        EQ_API ServerPtr getServer();

        /**
         * @return true if this pipe is running, false otherwise.
         * @version 1.0 
         */
        bool isRunning() const;

        /**
         * @return true if this pipe is stopped, false otherwise.
         * @version 1.0 
         */
        EQ_API bool isStopped() const;

        /**
         * Return the current frame number.
         *
         * To be called only from the pipe thread. Updated by startFrame().
         * @return the current frame number.
         * @version 1.0
         */ 
        uint32_t getCurrentFrame()  const { return _currentFrame; }
        EQ_API uint32_t getFinishedFrame() const; //!< @internal

        /**
         * Return the window system used by this pipe.
         * 
         * The return value is quaranteed to be constant for an initialized
         * pipe, that is, the window system is determined using
         * selectWindowSystem() before configInit() is executed.
         * 
         * @return the window system used by this pipe.
         * @version 1.0
         */
        WindowSystem getWindowSystem() const { return _windowSystem; }
        //@}

        /**
         * @name Operations
         */
        //@{
        /** 
         * @internal
         * Get an assembly frame.
         * 
         * @param frameVersion the frame's identifier and version.
         * @param eye the current eye pass.
         * @param output true if an output frame, false if input frame
         * @return the frame.
         */
        Frame* getFrame( const co::ObjectVersion& frameVersion, 
                         const Eye eye, const bool output );

        /** @internal Clear the frame cache and delete all frames. */
        void flushFrames();

        /** @internal @return if the window is made current */
        bool isCurrent( const Window* window ) const;

        /**
         * @internal
         * Set the window as current window.
         * @sa Window::makeCurrent()
         */
        void setCurrent( const Window* window ) const;

        /** @internal @return the view for the given identifier and version. */
        const View* getView( const co::ObjectVersion& viewVersion ) const;

        /** @internal @return the view for the given identifier and version. */
        View* getView( const co::ObjectVersion& viewVersion );
        //@}

        void waitExited() const; //!<  @internal Wait for the pipe to be exited
        void notifyMapped(); //!< @internal
        
        /**
         * @internal
         * Wait for a frame to be finished.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrame()
         */
        EQ_API void waitFrameFinished( const uint32_t frameNumber ) const;

        /**
         * @internal
         * Wait for a frame to be released locally.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrameLocal()
         */
        EQ_API void waitFrameLocal( const uint32_t frameNumber ) const;

        /** @internal Start the pipe thread. */
        void startThread();

        /** @internal Wait for the pipe thread to exit. */
        void joinThread();

        /** 
         * @name Interface to and from the SystemPipe, the window-system
         *       specific pieces for a pipe.
         */
        //@{
        /**
         * Set the system-specific pipe implementation.
         * 
         * The system-specific pipe implements the window-system-dependent part.
         * The os-specific pipe has to be initialized.
         * @version 1.0
         */
        void setSystemPipe( SystemPipe* pipe )  { _systemPipe = pipe; }

        /** @return the OS-specific pipe implementation. @version 1.0 */
        SystemPipe* getSystemPipe() { return _systemPipe; }

        /** @return the OS-specific pipe implementation. @version 1.0 */
        const SystemPipe* getSystemPipe() const { return _systemPipe; }
        //@}

        /**
         * @name Interface to and from the ComputeContext
         * @warning experimental - may not be supported in the future.         
         */
        //@{
        /** Set the compute-specific context. */
        void setComputeContext( ComputeContext* ctx ) { _computeContext = ctx; }

        /** @return the compute context. */
        const ComputeContext* getComputeContext() const
            { return _computeContext; }

        /** @return the compute context. */
        ComputeContext* getComputeContext() { return _computeContext; }
        //@}

        /** @name Configuration. */
        //@{
        /** 
         * Create a new MessagePump for this pipe.
         *
         * At most one message pump per execution thread is created. Each pipe
         * render thread creates one message pump for its window system. The
         * process main thread creates a message pump for AGL pipes and
         * non-threaded pipes. Applications which do their own message pumping
         * outside of Equalizer should return 0 here.
         *
         * @return the message pump, or 0.
         * @version 1.0
         */
        EQ_API virtual MessagePump* createMessagePump();

        /** @return the pipe's message pump, or 0. @version 1.0 */
        MessagePump* getMessagePump();
        //@}

    protected:
        /** @name Operations */
        //@{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         * @version 1.0
         */
        EQ_API void startFrame( const uint32_t frameNumber );

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         * @version 1.0
         */
        EQ_API void releaseFrame( const uint32_t frameNumber );

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         * @version 1.0
         */
        EQ_API void releaseFrameLocal( const uint32_t frameNumber );
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
         * @return true if the window system is supported, false if not.
         * @version 1.0
         */
        EQ_API virtual bool supportsWindowSystem( const WindowSystem system )
                                   const;

        /** 
         * Choose the window system to be used by this pipe.
         * 
         * This function determines which of the supported windowing systems is
         * used by this pipe instance. 
         * 
         * @return the window system currently used by this pipe.
         * @version 1.0
         */
        EQ_API virtual WindowSystem selectWindowSystem() const;

        /** 
         * Initialize this pipe.
         * 
         * @param initID the init identifier.
         * @version 1.0
         */
        EQ_API virtual bool configInit( const uint128_t& initID );

       /** 
         * Initialize the OS-specific pipe.
         *
         * @sa setSystemPipe()
         * @version 1.0
         */
        EQ_API virtual bool configInitSystemPipe( const uint128_t& initID );

        /** 
         * De-initialize this pipe.
         * @version 1.0
         */
        EQ_API virtual bool configExit();

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
         * @sa Config::startFrame(), Node::startFrame(),
         *     Node::waitFrameStarted()
         * @version 1.0
         */
        EQ_API virtual void frameStart( const uint128_t& frameID, 
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
         * @version 1.0
         */
        EQ_API virtual void frameFinish( const uint128_t& frameID, 
                                         const uint32_t frameNumber );

        /** 
         * Finish drawing.
         * 
         * Called once per frame after the last draw operation. Releases the
         * local synchronization if the thread model is draw_sync (the default).
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finished with draw.
         * @version 1.0
         */
        EQ_API virtual void frameDrawFinish( const uint128_t& frameID, 
                                             const uint32_t frameNumber );

        /** @internal */
        EQ_API virtual void attach( const UUID& id, const uint32_t instanceID );

    private:
        //-------------------- Members --------------------
        /** Window-system specific functions class */
        SystemPipe* _systemPipe;

        /** The current window system. */
        WindowSystem _windowSystem;

        enum State
        {
            STATE_MAPPED,
            STATE_INITIALIZING,
            STATE_RUNNING,
            STATE_STOPPED, // must come after running
            STATE_FAILED
        };
        /** The configInit/configExit state. */
       co::base::Monitor< State > _state;

        /** The last started frame. */
        uint32_t _currentFrame;

        /** The number of the last finished frame. */
       co::base::Monitor< uint32_t > _finishedFrame;

        /** The number of the last locally unlocked frame. */
       co::base::Monitor<uint32_t> _unlockedFrame;

        /** The running per-frame statistic clocks. */
        std::deque< int64_t > _frameTimes;
       co::base::Lock            _frameTimeMutex;

        /** The base time for the currently active frame. */
        int64_t _frameTime;

        /** The time spent waiting since the last frame start. */
        int64_t _waitTime;

        typedef stde::hash_map< uint128_t, Frame* > FrameHash;
        typedef stde::hash_map< uint128_t, FrameData* > FrameDataHash;

        /** All assembly frames used by the pipe during rendering. */
        FrameHash _frames;

        /** All output frame datas used by the pipe during rendering. */
        FrameDataHash _outputFrameDatas;

        typedef stde::hash_map< uint128_t, View* > ViewHash;
        /** All views used by the pipe's channels during rendering. */
        ViewHash _views;

        /** The pipe thread. */
        class PipeThread : public co::base::Thread
        {
        public:
            PipeThread( Pipe* pipe ) 
                    : _pipe( pipe )
                {}
            
            virtual void run(){ _pipe->_runThread(); }

        private:
            Pipe* _pipe;
        };
        PipeThread* _thread;

        /** The receiver->pipe thread command queue. */
        eq::CommandQueue*   _pipeThreadQueue;

        /** The last window made current. */
        const mutable Window* _currentWindow;

        /** GPU Computing context */
        ComputeContext *_computeContext;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        //-------------------- Methods --------------------
        void _runThread();
        void _setupCommandQueue();
        void _exitCommandQueue();

        friend class Window;

        /** @internal Release the views not used for some revisions. */
        void _releaseViews();

        /** @internal Clear the view cache and release all views. */
        void _flushViews();

        /* The command functions. */
        bool _cmdCreateWindow( co::Command& command );
        bool _cmdDestroyWindow( co::Command& command );
        bool _cmdConfigInit( co::Command& command );
        bool _cmdConfigExit( co::Command& command );
        bool _cmdFrameStartClock( co::Command& command );
        bool _cmdFrameStart( co::Command& command );
        bool _cmdFrameFinish( co::Command& command );
        bool _cmdFrameDrawFinish( co::Command& command );
        bool _cmdExitThread( co::Command& command );

        EQ_TS_VAR( _pipeThread );
    };
}

#endif // EQ_PIPE_H

