
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_NODE_H
#define EQS_NODE_H

#include "config.h"

#include <eq/net/barrier.h>
#include <eq/net/bufferConnection.h>
#include <eq/net/node.h>

#include <vector>

namespace eqs
{
    class Pipe;
    class Server;

    /**
     * The node.
     */
    class Node : public eqNet::Object
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
            { return _config ? _config->getServer() : NULL; }

        eqBase::RefPtr<eqNet::Node> getNode() const { return _node; }
        void setNode( eqBase::RefPtr<eqNet::Node> node ) { _node = node; }

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

        /** 
         * Returns the number of pipes on this node.
         * 
         * @return the number of pipes on this node. 
         */
        uint32_t nPipes() const { return static_cast<uint32_t>(_pipes.size()); }

        /** 
         * Gets a pipe.
         * 
         * @param index the pipe's index. 
         * @return the pipe.
         */
        Pipe* getPipe( const uint32_t index ) const { return _pipes[index]; }

        /** 
         * Gets the vector of pipes
         * 
         * @return the pipe vector.
         */
        const std::vector<Pipe*>& getPipes() const { return _pipes; }

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
        bool isUsed() const { return (_used!=0); }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }
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
        void startFrame( const uint32_t frameID, const uint32_t frameNumber );

        /**
         * Trigger the finish rendering of a frame.
         *
         * @param frame the number of the frame to complete.
         */
        void finishFrame( const uint32_t frame )
            { _sendFrameTasks( frame, _node->getConnection( )); }

        /** 
         * Synchronize the completion of the rendering of a frame.
         * 
         * @param frame 
         */
        void syncFrame( const uint32_t frame )
            { _finishedFrame.waitGE( frame ); }
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
        eqNet::Barrier* getBarrier();

        /** 
         * Release a barrier server by this node.
         * 
         * @param barrier the barrier.
         */
        void releaseBarrier( eqNet::Barrier* barrier );
        //*}

        void send( eqNet::SessionPacket& packet ) 
            { 
                packet.sessionID = getConfig()->getID(); 
                _bufferedTasks.send( packet );
            }
        void send( eqNet::SessionPacket& packet, const std::string& string ) 
            {
                packet.sessionID = getConfig()->getID(); 
                _bufferedTasks.send( packet, string );
            }
        template< typename T >
        void send( eqNet::ObjectPacket &packet, const std::vector<T>& data )
            {
                packet.sessionID = getConfig()->getID(); 
                _bufferedTasks.send( packet, data );
            }

        /** 
         * Adds a new description how this node can be reached.
         * 
         * @param cd the connection description.
         */
        void addConnectionDescription( 
            eqBase::RefPtr<eqNet::ConnectionDescription> cd)
            { _connectionDescriptions.push_back( cd ); }
        
        /** 
         * Removes a connection description.
         * 
         * @param index the index of the connection description.
         */
        void removeConnectionDescription( const uint32_t index );

        /** 
         * Returns the number of stored connection descriptions. 
         * 
         * @return the number of stored connection descriptions. 
         */
        uint32_t nConnectionDescriptions() const 
            { return _connectionDescriptions.size(); }

        /** 
         * Returns a connection description.
         * 
         * @param index the index of the connection description.
         * @return the connection description.
         */
        eqBase::RefPtr<eqNet::ConnectionDescription> getConnectionDescription(
            const uint32_t index ) const
            { return _connectionDescriptions[index]; }

        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

    protected:
        virtual ~Node();

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

    private:
        /** The pipe's name */
        std::string _name;

        /** The parent config. */
        Config* _config;
        friend class Config;

        /** The vector of pipes belonging to this node. */
        std::vector<Pipe*> _pipes;

        /** The reason for the last error. */
        std::string        _error;

        /** Number of entitities actively using this node. */
        uint32_t _used;

        /** The network node on which this Equalizer node is running. */
        eqBase::RefPtr<eqNet::Node> _node;

        /** The list of descriptions on how this node is reachable. */
        std::vector< eqBase::RefPtr<eqNet::ConnectionDescription> >
            _connectionDescriptions;

        /** The number of the last finished frame. */
        eqBase::Monitor<uint32_t> _finishedFrame;

        enum State
        {
            STATE_STOPPED = 0,  // next: INITIALIZING
            STATE_INITIALIZING, // next: INIT_FAILED or RUNNING
            STATE_INIT_FAILED,  // next: STOPPING
            STATE_RUNNING,      // next: STOPPING
            STATE_STOPPING,     // next: STOP_FAILED or STOPPED
            STATE_STOP_FAILED,  // next: STOPPED
        };

        /** The current state for state change synchronization. */
        eqBase::Monitor< State > _state;
            
        /** The cached barriers. */
        std::vector<eqNet::Barrier*> _barriers;

        /** Task packets for the current operation. */
        eqNet::BufferConnection _bufferedTasks;

        /** Stored tasks for a given frame. */
        struct FrameTasks
        {
            uint32_t                frame;
            eqNet::BufferConnection tasks;
        };
        /** The list of store frame tasks. */
        std::vector<FrameTasks> _frameTasks;

        /** common code for all constructors */
        void _construct();

        /** flush cached barriers. */
        void _flushBarriers();

        void _storeFrameTasks( const uint32_t frame, 
                               eqNet::BufferConnection& tasks );
        void _sendFrameTasks( const uint32_t frame, 
                              eqBase::RefPtr<eqNet::Connection> connection );

        void _send( eqNet::ObjectPacket& packet ) 
            { packet.objectID = getID(); send( packet ); }
        void _send( eqNet::ObjectPacket& packet, const std::string& string ) 
            { packet.objectID = getID(); send( packet, string ); }

        /* Command handler functions. */
        eqNet::CommandResult _cmdConfigInitReply( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigExitReply( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameFinishReply( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Node* node );
};
#endif // EQS_NODE_H
