
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIPE_H
#define EQ_PIPE_H

#include <eq/client/commandQueue.h>   // member
#include <eq/client/node.h>           // used in inline methods
#include <eq/client/pixelViewport.h>  // member
#include <eq/client/statEvent.h>      // member
#include <eq/client/windowSystem.h>   // member

#include <eq/base/refPtr.h>
#include <eq/base/spinLock.h>
#include <eq/base/thread.h>
#include <eq/net/base.h>
#include <eq/net/object.h>

namespace eq
{
    class EventHandler;
    class Frame;
    class Window;

    class EQ_EXPORT Pipe : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new pipe.
         */
        Pipe( Node* parent );

        /** @name Data Access. */
        //*{
        Node* getNode() const { return _node; }
        Config* getConfig() const { return (_node ? _node->getConfig() : NULL);}
        eqBase::RefPtr< Client > getClient() const
            { return (_node ? _node->getClient() : NULL);}
        eqBase::RefPtr< Server > getServer() const
            { return (_node ? _node->getServer() : NULL);}
        const WindowVector& getWindows() const { return _windows; }

        const std::string& getName() const { return _name; }
        bool isThreaded() const { return ( _thread != 0 ); }

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
         * @return The string representation of this pipe's port and device
         *         setting, in the form used by XOpenDisplay().
         */
        std::string getXDisplayString();

        /** 
         * Return the window system used by this pipe. 
         * 
         * The return value is quaranteed to be constant for an initialised
         * pipe, that is, the window system is determined usign
         * selectWindowSystem() before the pipe init method is executed.
         * 
         * @return the window system used by this pipe.
         */
        WindowSystem getWindowSystem() const { return _windowSystem; }

        /** @return the X display connection for this pipe. */
        Display* getXDisplay() const { return _xDisplay; }

        /** @return the CG display ID for this pipe. */
        CGDirectDisplayID getCGDisplayID() const { return _cgDisplayID; }

        /** @return the time in ms elapsed since the frame started. */
        float getFrameTime() const { return _frameClock.getTimef(); }

        /** @return the pipe's event handler, or 0. */
        EventHandler* getEventHandler() { return _eventHandler; }
        //*}

        /**
         * @name Operations
         */
        //*{
        /**
         * Create a device context bound only to the display of this pipe.
         *
         * If the dc return parameter is set to 0 and the return value is true,
         * an affinitiy dc is not needed.
         *
         * @param affinityDC the affinity device context output parameter.
         * @param deleteProc the deleteDC function pointer output parameter.
         * @return the success status.
         */
        bool createAffinityDC( HDC& affinityDC, 
                               PFNWGLDELETEDCNVPROC& deleteProc );

        /** Add a new statistics event to the current frame. */
        void addStatEvent( StatEvent::Data& eventData );

        /**'
         * Push a command to the pipe thread to be handled from there.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        eqNet::CommandResult pushCommand( eqNet::Command& command );

        /** 
         * Get an assembly frame.
         * 
         * @param id the frame identifier.
         * @param version the frame's version.
         * @return the frame.
         */
        Frame* getFrame( const uint32_t id, const uint32_t version );
        //*}

        /** Wait for the pipe to be exited. */
        void waitExited() const { _initialized.waitEQ( false ); }
        bool isInitialized() const { return _initialized.get(); }
        
        /** 
         * Wait for a frame to be finished.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrame()
         */
        void waitFrameFinished( const uint32_t frameNumber ) const
            { _finishedFrame.waitGE( frameNumber ); }

        /** 
         * Wait for a frame to be released locally.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrameLocal()
         */
        void waitFrameLocal( const uint32_t frameNumber ) const
            { _unlockedFrame.waitGE( frameNumber ); }

        /** Wait for the pipe to exit. */
        void waitExit();

    protected:
        /**
         * Destructs the pipe.
         */
        virtual ~Pipe();
        friend class Node;

        /** @name Data Access. */
        //*{
        /** 
         * Set the X display connection for this pipe.
         * 
         * This function should only be called from configInit() or
         * configExit(). Updates the pixel viewport.
         *
         * @param display the X display connection for this pipe.
         */
        void setXDisplay( Display* display );

        /** 
         * Set the CG display ID for this pipe.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param id the CG display ID for this pipe.
         */
        void setCGDisplayID( CGDirectDisplayID id );
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
        void releaseFrame( const uint32_t frameNumber );

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         */
        void releaseFrameLocal( const uint32_t frameNumber );
        //*}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //*{
        /** 
         * Tests wether a particular windowing system is supported by this pipe
         * and all its windows.
         * 
         * @param system the window system to test.
         * @return <code>true</code> if the window system is supported,
         *         <code>false</code> if not.
         */
        virtual bool supportsWindowSystem( const WindowSystem system ) const;

        /** 
         * Return the window system to be used by this pipe.
         * 
         * This function determines which of the supported windowing systems is
         * used by this pipe instance. 
         * 
         * @return the window system currently used by this pipe.
         */
        virtual WindowSystem selectWindowSystem() const;

        /** 
         * Initialises this pipe.
         * 
         * @param initID the init identifier.
         */
        virtual bool configInit( const uint32_t initID );
        virtual bool configInitGLX();
        virtual bool configInitAGL();
        virtual bool configInitWGL();

        /** 
         * Exit this pipe.
         */
        virtual bool configExit();
        virtual void configExitGLX();
        virtual void configExitAGL();
        virtual void configExitWGL();

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of pipe-specific data, for example updating the rendering engine. The
         * default implementation waits for the node to start the frame. This
         * method has to call startFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @sa Config::beginFrame(), Node::startFrame(), 
         *     Node::waitFrameStarted()
         */
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

        /**
         * Finish rendering a frame.
         *
         * Called once at the end of each frame, to do per-frame updates of
         * pipe-specific data, for example updating the rendering engine.  This
         * method has to call releaseFrame().
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

        /**
         * Initialize the event handling for this pipe. 
         * 
         * This function initializes the necessary event handler for this pipe,
         * if required by the window system. Can be overriden by an empty
         * method to disable built-in event handling.
         * @sa EventHandler, useMessagePump()
         */
        virtual void initEventHandler();

        /**
         * De-initialize the event handling for this pipe. 
         */
        virtual void exitEventHandler();

        /** The current event handler, or 0. */
        EventHandler* _eventHandler;
        //*}

        /** @name Error information. */
        //*{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the configInit method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //*}

        /** @name Configuration. */
        //*{
        /** 
         * Enable or disable automatic or external OS event dispatch for the
         * pipe thread.
         *
         * @return true if Equalizer shall dispatch OS events, false if the
         *         application dispatches OS events.
         * @sa Event handling documentation on website.
         */
        virtual bool useMessagePump() { return true; }
        //*}

    private:
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

        /** The reason for the last error. */
        std::string            _error;

        /** Window-system specific display information. */
        union
        {
            Display*          _xDisplay;
            CGDirectDisplayID _cgDisplayID;
            char              _pipeFill[16];
        };

        /** The display (GLX) or ignored (Win32, AGL). */
        uint32_t _port;

        /** The screen (GLX), GPU (Win32) or virtual screen (AGL). */
        uint32_t _device;
        
        /** The configInit/configExit state. */
        eqBase::Monitor<bool> _initialized;

        /** The number of the last finished frame. */
        eqBase::Monitor<uint32_t> _finishedFrame;

        /** The number of the last locally unlocked frame. */
        eqBase::Monitor<uint32_t> _unlockedFrame;

        /** The running per-frame statistic clocks. */
        std::deque<eqBase::Clock> _frameClocks;
		eqBase::SpinLock          _frameClockMutex;

        /** The clock for the currently active frame. */
        eqBase::Clock _frameClock;

        /** The statistics events gathered during the current frame. */
        std::vector<StatEvent::Data> _statEvents;

        /** All assembly frames used by the pipe during rendering. */
        eqNet::IDHash< Frame* > _frames;

        /** The pipe thread. */
        class PipeThread : public eqBase::Thread
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
        eq::CommandQueue*   _commandQueue;

        void* _runThread();
        void _setupCommandQueue();

        static int XErrorHandler( Display* display, XErrorEvent* event );

        friend class Window;
        void _addWindow( Window* window );
        void _removeWindow( Window* window );
        Window* _findWindow( const uint32_t id );

        void _flushFrames();

        /* The command functions. */
        eqNet::CommandResult _cmdCreateWindow( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyWindow( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigInit( eqNet::Command& command );
        eqNet::CommandResult _reqConfigInit( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigExit( eqNet::Command& command );
        eqNet::CommandResult _reqConfigExit( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameStart( eqNet::Command& command );
        eqNet::CommandResult _reqFrameStart( eqNet::Command& command );
        eqNet::CommandResult _reqFrameFinish( eqNet::Command& command );
        eqNet::CommandResult _reqFrameDrawFinish( eqNet::Command& command );
    };
}

#endif // EQ_PIPE_H

