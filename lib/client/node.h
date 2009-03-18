
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum

#include <eq/net/barrier.h>
#include <eq/net/objectVersion.h>
#include <eq/base/base.h>

namespace eq
{
    class CommandQueue;
    class FrameData;
    class NodeVisitor;

    /**
     * A Node represents a single computer in the cluster.
     *
     * Each node is executed in a seperate process.
     */
    class EQ_EXPORT Node : public net::Object
    {
    public:
        /** Constructs a new node. */
        Node( Config* parent );

        /** Destructs the node. */
        virtual ~Node();

        /** 
         * Returns the config of this node.
         * 
         * @return the config of this node. 
         */
        Config*       getConfig()       { return _config; }
        const Config* getConfig() const { return _config; }

        ClientPtr getClient();
        ServerPtr getServer();

        const PipeVector& getPipes() const { return _pipes; }

        const std::string& getName() const { return _name; }

        /** 
         * Return the set of tasks this nodes's channels might execute in the
         * worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @warning Not finalized, might change in the future.
         * @return the tasks.
         */
        uint32_t getTasks() const { return _tasks; }

        CommandQueue* getNodeThreadQueue();

        /** 
         * Traverse this node and all children using a node visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( NodeVisitor& visitor );

        /** 
         * Get a network barrier. 
         * 
         * @param id the barrier identifier.
         * @param version the barrier version.
         * @return the barrier.
         */
        net::Barrier* getBarrier( const uint32_t id, const uint32_t version );

        /** 
         * Get a frame data instance.
         * 
         * @param dataVersion the frame data identifier and version.
         * @return the frame.
         */
        FrameData* getFrameData( const net::ObjectVersion& dataVersion );

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

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array initialization in node.cpp
        /** Node attributes. */
        enum IAttribute
        {
            IATTR_THREAD_MODEL,           //!< Threading model
            IATTR_HINT_STATISTICS,        //!< Statistics gathering mode
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };

        void setIAttribute( const IAttribute attr, const int32_t value )
            { _iAttributes[attr] = value; }
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
        //*}

#ifdef EQ_ASYNC_TRANSMIT
        class TransmitThread : public base::Thread
        {
        public:
            TransmitThread( Node* parent ) : _node( parent ) {}
            virtual ~TransmitThread() {}

            void send( FrameData* data, net::NodePtr node, 
                       const uint32_t frameNumber );
            
        protected:
            virtual void* run();

        private:
            struct Task
            {
                Task( FrameData* d, net::NodePtr n, const uint32_t f ) 
                        : data( d ), node( n ), frameNumber( f ) {}

                FrameData*   data;
                net::NodePtr node;
                uint32_t     frameNumber;
            };

            base::MTQueue< Task > _tasks;
            Node* const           _node;
        };

        TransmitThread transmitter;
#endif

    protected:
        friend class Config;

        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );

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
         * call startFrame(). Immediately releases local synchronization if the
         * thread model is async.
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @sa startFrame(), Config::beginFrame()
         */
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

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
         * Called once per frame after the last draw operation. Waits for the
         * pipes to release the local synchonization and releases the node's
         * local synchronization if the thread model is draw_sync (the default).
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame finished with draw.
         * @sa Pipe::waitFrameLocal(), releaseFrameLocal()
         */
        virtual void frameDrawFinish( const uint32_t frameID, 
                                      const uint32_t frameNumber );

        /** 
         * Finish all rendering tasks.
         * 
         * Called once per frame after all frame tasks.  Waits for the pipes to
         * release the local synchonization and releases the node's local
         * synchronization if the thread model is local_sync.
         *
         * Note that frameFinish is called after the latency is exhausted and
         * synchronizes pipe thread execution.
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame finished with draw.
         * @sa Pipe::waitFrameLocal(), releaseFrameLocal()
         */
        virtual void frameTasksFinish( const uint32_t frameID, 
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

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** Pipe children. */
        PipeVector             _pipes;

        /** The reason for the last error. */
        std::string            _error;

        /** The configInit/configExit state. */
        base::Monitor<bool> _initialized;

        /** The number of the last started frame. */
        base::Monitor<uint32_t> _currentFrame;

        /** The number of the last locally released frame. */
        uint32_t                  _unlockedFrame;

        /** The number of the last finished frame. */
        uint32_t                  _finishedFrame;

        typedef stde::hash_map< uint32_t, net::Barrier* > BarrierHash;
        /** All barriers mapped by the node. */
        BarrierHash _barriers;
        base::Lock  _barriersMutex;

        typedef stde::hash_map< uint32_t, FrameData* > FrameDataHash;
        /** All frame datas used by the node during rendering. */
        FrameDataHash _frameDatas;
        base::Lock    _frameDatasMutex;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        friend class Pipe;
        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );
        Pipe* _findPipe( const uint32_t id );

        void _finishFrame( const uint32_t frameNumber ) const;
        void _frameFinish( const uint32_t frameID, const uint32_t frameNumber );

        void _flushObjects();

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /** The command functions. */
        net::CommandResult _cmdCreatePipe( net::Command& command );
        net::CommandResult _cmdDestroyPipe( net::Command& command );
        net::CommandResult _cmdConfigInit( net::Command& command );
        net::CommandResult _cmdConfigExit( net::Command& command );
        net::CommandResult _cmdFrameStart( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
        net::CommandResult _cmdFrameDrawFinish( net::Command& command );
        net::CommandResult _cmdFrameTasksFinish( net::Command& command );

        CHECK_THREAD_DECLARE( _nodeThread );
    };
}

#endif // EQ_NODE_H

