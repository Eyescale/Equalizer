
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

#ifdef EQ_EXPORTS
   // We need to instantiate a Monitor< State > when compiling the library,
   // but we don't want to have <pthread.h> for a normal build, hence this hack
#  include <pthread.h>
#endif
#include <eq/base/monitor.h>

#include <eq/client/eye.h>            // Eye enum
#include <eq/client/os.h>             // WGLEWContext
#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum
#include <eq/client/windowSystem.h>   // enum

#include <eq/fabric/pipe.h>           // base class

#include <eq/net/objectVersion.h>

#include <eq/base/lock.h>
#include <eq/base/refPtr.h>
#include <eq/base/thread.h>

namespace eq
{
    class CommandQueue;
    class ComputeContext;
    class FrameData;
    class MessagePump;
    class OSPipe;

    /**
     * A Pipe represents a graphics card (GPU) on a Node.
     *
     * All Pipe, Window and Channel task methods are executed in a separate
     * eq::base::Thread, in parallel with all other pipes in the system, unless
     * the pipe is non-threaded, in which case the tasks are executed on the
     * Node's main thread.
     */
    class Pipe : public fabric::Pipe< Node, Pipe, eq::Window, PipeVisitor >
    {
    public:
        /** Constructs a new pipe. */
        EQ_EXPORT Pipe( Node* parent );

        /** Destructs the pipe. */
        EQ_EXPORT virtual ~Pipe();

        /** @name Data Access. */
        //@{
        EQ_EXPORT net::CommandQueue* getPipeThreadQueue(); //!< @internal
        net::CommandQueue* getMainThreadQueue(); //!< @internal

        EQ_EXPORT Config* getConfig();
        EQ_EXPORT const Config* getConfig() const;

        EQ_EXPORT ClientPtr getClient();
        EQ_EXPORT ServerPtr getServer();

        uint32_t getCurrentFrame()  const { return _currentFrame; }
        EQ_EXPORT uint32_t getFinishedFrame() const;

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
        Frame* getFrame( const net::ObjectVersion& frameVersion, 
                         const Eye eye, const bool output );

        /** @internal Clear the frame cache and delete all frames. */
        void flushFrames();

        /** @return if the window is made current */
        bool isCurrent( const Window* window ) const;

        /** Set the window as current window. */
        void setCurrent( const Window* window ) const;

        /** @internal @return the view for the given identifier and version. */
        const View* getView( const net::ObjectVersion& viewVersion ) const;

        /** @internal @return the view for the given identifier and version. */
        View* getView( const net::ObjectVersion& viewVersion );
        //@}

        /** Wait for the pipe to be exited. @internal */
        void waitExited() const;
        bool isRunning() const; //!< @internal
        void notifyMapped(); //!< @internal
        
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

        /** Start the pipe thread. @internal */
        void startThread();

        /** Wait for the pipe thread to exit. @internal */
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

        /** 
         * @name Interface to and from the ComputeContext
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
         * render thread creates one message pump for its window system, and the
         * process main thread creates a message pump for AGL pipes or if
         * non-threaded pipes are used. Applications which do their own message
         * pumping outside of Equalizer should return 0 here.
         *
         * @return the message pump, or 0.
         */
        EQ_EXPORT virtual MessagePump* createMessagePump();
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
         * Initialize this pipe.
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

        /** @sa net::Object::attachToSession. */
        EQ_EXPORT virtual void attachToSession( const uint32_t id, 
                                                const uint32_t instanceID, 
                                                net::Session* session );

    private:
        //-------------------- Members --------------------
        /** Window-system specific functions class */
        OSPipe *_osPipe;

        /** The current window system. */
        WindowSystem _windowSystem;

        enum State
        {
            STATE_STOPPED,
            STATE_MAPPED,
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
        typedef stde::hash_map< uint32_t, FrameData* > FrameDataHash;

        /** All assembly frames used by the pipe during rendering. */
        FrameHash _frames;

        /** All output frame datas used by the pipe during rendering. */
        FrameDataHash _outputFrameDatas;

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

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

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
        bool _cmdCreateWindow( net::Command& command );
        bool _cmdDestroyWindow( net::Command& command );
        bool _cmdConfigInit( net::Command& command );
        bool _cmdConfigExit( net::Command& command );
        bool _cmdFrameStartClock( net::Command& command );
        bool _cmdFrameStart( net::Command& command );
        bool _cmdFrameFinish( net::Command& command );
        bool _cmdFrameDrawFinish( net::Command& command );

        EQ_TS_VAR( _pipeThread );
    };
}

#endif // EQ_PIPE_H

