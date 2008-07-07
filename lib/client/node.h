
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include <eq/client/config.h>         // called in inline method

#include <eq/base/base.h>
#include <eq/net/barrier.h>

namespace eq
{
    class Config;
    class FrameData;
    class Pipe;
    class Server;

    typedef stde::hash_map< uint32_t, FrameData* > FrameDataCache;

    /**
     * A Node represents a single computer in the cluster.
     *
     * Each node is executed in a seperate process.
     */
    class EQ_EXPORT Node : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new node.
         */
        Node( Config* parent );

        /** 
         * Returns the config of this node.
         * 
         * @return the config of this node. 
         */
        Config* getConfig() const { return _config; }
        eq::base::RefPtr< Client > getClient() const
            { return (_config ? _config->getClient() : 0); }
        eq::base::RefPtr< Server > getServer() const
            { return (_config ? _config->getServer() : 0); }
        const PipeVector& getPipes() const { return _pipes; }

        const std::string& getName() const { return _name; }
        CommandQueue* getNodeThreadQueue()
            { return getClient()->getNodeThreadQueue(); }

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
         * @param dataVersion the frame data identifier and version.
         * @return the frame.
         */
        FrameData* getFrameData( const eqNet::ObjectVersion& dataVersion );

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

#ifdef EQ_TRANSMISSION_API
        /** @name Data Transmission. */
        //*{
        /** 
         * Blockingly receive data sent by Config::broadcastData().
         * 
         * @param size return value for the size of the data, can be 0.
         * @return the data pointer.
         */
        const void* receiveData( uint64_t* size );

        /** 
         * Non-blockingly receive data sent by Config::broadcastData().
         * 
         * @param size return value for the size of the data, can be 0.
         * @return the data pointer, or 0 if no data is pending.
         */
        const void* tryReceiveData( uint64_t* size );

        /** @return true if data is available, false if not. */
        bool  hasData() const;
        //*}
#endif

    protected:
        /**
         * Destructs the node.
         */
        virtual ~Node();
        friend class Config;

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

        /** 
         * Finish drawing.
         * 
         * Called once per frame after the last draw operation. The default
         * implementation waits for all pipe threads to release the local
         * synchronization in order to frame-synchronize the node and pipe
         * threads.
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame finished with draw.
         * @sa Pipe::waitFrameLocal(), releaseFrameLocal()
         */
        virtual void frameDrawFinish( const uint32_t frameID, 
                                      const uint32_t frameNumber );
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
        /** The parent config */
        Config* const          _config;

        /** The name. */
        std::string            _name;

        PipeVector             _pipes;

        /** The reason for the last error. */
        std::string            _error;

        /** The configInit/configExit state. */
        eq::base::Monitor<bool> _initialized;

        /** The number of the last started frame. */
        eq::base::Monitor<uint32_t> _currentFrame;

        /** The number of the last locally released frame. */
        uint32_t                  _unlockedFrame;

        /** The number of the last finished frame. */
        uint32_t                  _finishedFrame;

        /** All barriers mapped by the node. */
        eqNet::IDHash< eqNet::Barrier* > _barriers;
        eq::base::Lock                     _barriersMutex;

        /** All frame datas used by the node during rendering. */
        FrameDataCache _frameDatas;
        eq::base::Lock   _frameDatasMutex;

        /** The receiver->node data transmission queue. */
        CommandQueue           _dataQueue;

        friend class Pipe;
        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );
        Pipe* _findPipe( const uint32_t id );

        void _finishFrame( const uint32_t frameNumber ) const;
        void _frameFinish( const uint32_t frameID, const uint32_t frameNumber );

        void _flushObjects();

        /** The command functions. */
        eqNet::CommandResult _cmdCreatePipe( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyPipe( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigInit( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigExit( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameStart( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameFinish( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameDrawFinish( eqNet::Command& command );

        CHECK_THREAD_DECLARE( _nodeThread );
    };
}

#endif // EQ_NODE_H

