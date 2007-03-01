
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include <eq/client/config.h>         // called in inline method
#include <eq/client/statEvent.h>      // member

#include <eq/base/base.h>
#include <eq/net/barrier.h>

namespace eq
{
    class Config;
    class FrameData;
    class Pipe;
    class Server;

    typedef stde::hash_map< uint32_t, FrameData* > FrameDataCache;

    class EQ_EXPORT Node : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new node.
         */
        Node();

        /** 
         * Returns the config of this node.
         * 
         * @return the config of this node. 
         */
        Config* getConfig() const { return _config; }
        eqBase::RefPtr<eqNet::Node> getServer() const
            { return (_config ? _config->getServer() : NULL); }

        /** 
         * Gets a pipe.
         * 
         * @param index the pipe's index. 
         * @return the pipe.
         */
        Pipe* getPipe( const uint32_t index ) const
            { return _pipes[index]; }

        /** 
         * Get a network barrier. 
         * 
         * @param id the barrier identifier.
         * @param version the barrier version.
         * @return the barrier.
         */
        eqNet::Barrier* getBarrier( const uint32_t id, const uint32_t version );

        /** 
         * Get a frame data instance.
         * 
         * @param id the frame identifier.
         * @param version the frame's version.
         * @return the frame.
         */
        FrameData* getFrameData( const uint32_t id, const uint32_t version );

        /** 
         * Add stat events to a frame. Used by Pipe::releaseFrame().
         * 
         * @param frameNumber the frame number.
         * @param events the list of events.
         */
        void addStatEvents( const uint32_t frameNumber,
                            std::vector<StatEvent> events );

        /** Wait for the node to be initialized. */
        void waitInitialized() const { _initialized.waitEQ( true ); }

        /** 
         * Wait for a frame to be started.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrame()
         */
        void waitFrameStarted( const uint32_t frameNumber ) const
            { _currentFrame.waitGE( frameNumber ); }

    protected:
        /**
         * Destructs the node.
         */
        virtual ~Node();

        /** @name Actions */
        //*{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         */
        void startFrame( const uint32_t frameNumber ) 
            { _currentFrame = frameNumber; }

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         */
        void releaseFrame( const uint32_t frameNumber );
        //*}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //*{

        /** 
         * Initialises this node.
         * 
         * @param initID the init identifier.
         */
        virtual bool configInit( const uint32_t initID ){ return true; }

        /** 
         * Exit this node.
         */
        virtual bool configExit(){ return true; }

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to start the node's frame
         * and to do per-frame updates of node-specific data. This method has to
         * call startFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @sa startFrame(), Config::beginFrame()
         */
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber ) 
            { startFrame( frameNumber ); }

        /**
         * Finish rendering a frame.
         *
         * Called once at the end of each frame, to end the frame and to do
         * per-frame updates of node-specific data. This method has to call
         * releaseFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finish.
         * @sa endFrame(), Config::finishFrame()
         */
        virtual void frameFinish( const uint32_t frameID, 
                                  const uint32_t frameNumber ) 
            { releaseFrame( frameNumber ); }
        //*}

        /** @name Error information. */
        //*{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the init method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //*}
 
    private:
        friend class Config;
        Config*                _config;

        std::vector<Pipe*>     _pipes;

        /** The reason for the last error. */
        std::string            _error;

        /** The configInit/configExit state. */
        eqBase::Monitor<bool> _initialized;

        /** The number of the last started frame. */
        eqBase::Monitor<uint32_t> _currentFrame;

        /** All barriers mapped by the node. */
        eqNet::IDHash< eqNet::Barrier* > _barriers;
        eqBase::Lock                     _barriersMutex;

        /** All frame datas used by the node during rendering. */
        FrameDataCache _frameDatas;
        eqBase::Lock   _frameDatasMutex;

        struct FrameStatEvents
        {
            uint32_t               frameNumber;
            std::vector<StatEvent> events;
        };
        std::vector<FrameStatEvents> _statEvents;
        eqBase::SpinLock             _statEventsLock;

        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );
        Pipe* _findPipe( const uint32_t id );

        void _finishFrame( const uint32_t frameNumber );

        std::vector<StatEvent>& _getStatEvents( const uint32_t frameNumber );
        void _recycleStatEvents( const uint32_t frameNumber );

        void _flushObjects();

        /** The command functions. */
        eqNet::CommandResult _cmdCreatePipe( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyPipe( eqNet::Command& command );
        eqNet::CommandResult _reqConfigInit( eqNet::Command& command );
        eqNet::CommandResult _reqConfigExit( eqNet::Command& command );
        eqNet::CommandResult _reqFrameStart( eqNet::Command& command );
        eqNet::CommandResult _reqFrameFinish( eqNet::Command& command );

        CHECK_THREAD_DECLARE( _recvThread );
    };
}

#endif // EQ_NODE_H

