
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_NODE_H
#define EQSERVER_NODE_H

#include "config.h"                // used in inline method
#include "connectionDescription.h" // used in inline method

#include <eq/net/barrier.h>
#include <eq/net/bufferConnection.h>
#include <eq/net/node.h>

#include <vector>

namespace eq
{
namespace server
{
    /**
     * The node.
     */
    class Node : public net::Object
    {
    public:
        /** 
         * Constructs a new Node.
         */
        Node();

        /** 
         * Constructs a new deep copy of a node.
         */
        Node( const Node& from );

        /** @name Data Access. */
        //*{
        Config* getConfig() const { return _config; }
        Server* getServer() const
            { return _config ? _config->getServer() : 0; }

        net::NodePtr getNode() const { return _node; }
        void setNode( net::NodePtr node ) { _node = node; }
        bool isApplicationNode() const
            { return (this == _config->getApplicationNode( )); }

        net::CommandQueue* getServerThreadQueue()
            { return _config->getServerThreadQueue(); }
        net::CommandQueue* getCommandThreadQueue()
            { return _config->getCommandThreadQueue(); }

        /** @return the number of the last finished frame. */
        uint32_t getFinishedFrame() const { return _finishedFrame; }

        /** 
         * Adds a new pipe to this node.
         * 
         * @param pipe the pipe.
         */
        void addPipe( Pipe* pipe );

        /** 
         * Removes a pipe from this node.
         * 
         * @param pipe the pipe
         * @return <code>true</code> if the pipe was removed, <code>false</code>
         *         otherwise.
         */
        bool removePipe( Pipe* pipe );

        /** @return vector of pipes */
        const PipeVector& getPipes() const { return _pipes; }

        /** 
         * Traverse this node and all children using a node visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( NodeVisitor* visitor );

        /** 
         * References this node as being actively used.
         */
        void refUsed(){ _used++; }

        /** 
         * Unreferences this node as being actively used.
         */
        void unrefUsed(){ _used--; }

        /** 
         * Returns if this node is actively used.
         *
         * @return <code>true</code> if this node is actively used,
         *         <code>false</code> if not.
         */
        bool isUsed() const { return( _used!=0 ); }

        /**
         * Increase  node activition count.
         */
        void activate(){ _active++; }

        /** 
         * Decrease  node activition count.
         */
        void deactivate(){ _active--; };

        /** 
         * Returns if this node is actively used.
         *
         * @return <code>true</code> if this node has activation,
         *         <code>false</code> if not.
         */
        bool isActive() const{ return( _active != 0 ); }

        /** Add additional tasks this node might potentially execute. */
        void addTasks( const uint32_t tasks ) { _tasks |= tasks; }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** The last drawing channel for this entity. */
        void setLastDrawPipe( const Pipe* pipe )
            { _lastDrawPipe = pipe; }
        const Pipe* getLastDrawPipe() const { return _lastDrawPipe;}
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start initializing this node.
         *
         * @param initID an identifier to be passed to all init methods.
         */
        void startConfigInit( const uint32_t initID);

        /** 
         * Synchronize the initialisation of the node.
         * 
         * @return <code>true</code> if the node was initialised successfully,
         *         <code>false</code> if not.
         */
        bool syncConfigInit();

        /** 
         * Starts exiting this node.
         */
        void startConfigExit();

        /** 
         * Synchronize the exit of the node.
         * 
         * @return <code>true</code> if the node exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncConfigExit();
        
        /** 
         * Trigger the rendering of a new frame for this node.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void update( const uint32_t frameID, const uint32_t frameNumber );

        /** 
         * Flush the processing of frames, including frameNumber.
         *
         * @param frameNumber the number of the frame.
         */
        void flushFrames( const uint32_t frameNumber );

        /** Synchronize the completion of the rendering of a frame. */
        void finishFrame( const uint32_t frame );

        /** Invoke non-threaded rendering synchronization. */
        void updateFrameFinishNT( const uint32_t currentFrame );
        //*}


        /** @name Attributes */
        //*{
        void setIAttribute( const eq::Node::IAttribute attr, 
                            const int32_t value )
            { _iAttributes[attr] = value; }
        int32_t  getIAttribute( const eq::Node::IAttribute attr ) const
            { return _iAttributes[attr]; }
        //*}

        /**
         * @name Barrier Cache
         *
         * Caches barriers for which this node is the master.
         */
        //*{
        /** 
         * Get a new barrier of height 0.
         * 
         * @return the barrier.
         */
        net::Barrier* getBarrier();

        /** 
         * Release a barrier server by this node.
         * 
         * @param barrier the barrier.
         */
        void releaseBarrier( net::Barrier* barrier );
        //*}

        void send( net::SessionPacket& packet ) 
            { 
                packet.sessionID = _config->getID(); 
                _bufferedTasks.send( packet );
            }
        void send( net::SessionPacket& packet, const std::string& string ) 
            {
                packet.sessionID = _config->getID(); 
                _bufferedTasks.send( packet, string );
            }
        template< typename T >
        void send( net::ObjectPacket &packet, const std::vector<T>& data )
            {
                packet.sessionID = _config->getID(); 
                _bufferedTasks.send( packet, data );
            }

        /** 
         * Adds a new description how this node can be reached.
         * 
         * @param desc the connection description.
         */
        void addConnectionDescription( ConnectionDescriptionPtr desc )
            { _connectionDescriptions.push_back( desc ); }
        
        /** 
         * Removes a connection description.
         * 
         * @param index the index of the connection description.
         */
        void removeConnectionDescription( const uint32_t index );

        /** @return the vector of connection descriptions. */
        const ConnectionDescriptionVector& getConnectionDescriptions()
            const { return _connectionDescriptions; }

        /** @name Error information. */
        //*{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //*}

    protected:
        virtual ~Node();

        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

        /** @sa net::Object::attachToSession. */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );
    private:
        /** The node name */
        std::string _name;

        /** Integer attributes. */
        int32_t _iAttributes[eq::Node::IATTR_ALL];

        /** The parent config. */
        Config* _config;
        friend class Config;

        /** The vector of pipes belonging to this node. */
        PipeVector _pipes;

        /** The reason for the last error. */
        std::string _error;

        /** Number of entities actively using this node. */
        uint32_t _used;

        /** Number of activations for this node. */
        uint32_t _active;

        /** The network node on which this Equalizer node is running. */
        net::NodePtr _node;

        /** The list of descriptions on how this node is reachable. */
        ConnectionDescriptionVector _connectionDescriptions;

        /** The frame identifiers non-finished frames. */
        std::map< uint32_t, uint32_t > _frameIDs;

        /** The number of the last flushed frame (frame finish packet sent). */
        uint32_t _flushedFrame;

        /** The number of the last finished frame. */
        uint32_t _finishedFrame;

        enum State
        {
            STATE_STOPPED = 0,  // next: INITIALIZING
            STATE_INITIALIZING, // next: INIT_FAILED or RUNNING
            STATE_INIT_FAILED,  // next: STOPPING
            STATE_RUNNING,      // next: STOPPING
            STATE_STOPPING,     // next: STOP_FAILED or STOPPED
            STATE_STOP_FAILED,  // next: STOPPED
        };

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The current state for state change synchronization. */
        base::Monitor< State > _state;
            
        /** The cached barriers. */
        std::vector<net::Barrier*> _barriers;

        /** Task packets for the current operation. */
        net::BufferConnection _bufferedTasks;

        /** The last draw pipe for this entity */
        const Pipe* _lastDrawPipe;

        /** common code for all constructors */
        void _construct();

        /** flush cached barriers. */
        void _flushBarriers();

        void _send( net::ObjectPacket& packet ) 
            { packet.objectID = getID(); send( packet ); }
        void _send( net::ObjectPacket& packet, const std::string& string ) 
            { packet.objectID = getID(); send( packet, string ); }

        /** Send the frame finish packet for the given frame number. */
        void _sendFrameFinish( const uint32_t frameNumber );

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /* Command handler functions. */
        net::CommandResult _cmdConfigInitReply( net::Command& command );
        net::CommandResult _cmdConfigExitReply( net::Command& command );
        net::CommandResult _cmdFrameFinishReply( net::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Node* node );
}
}
#endif // EQSERVER_NODE_H
