
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_NODE_H
#define EQS_NODE_H

#include "config.h"

#include <eq/base/sema.h>
#include <eq/net/barrier.h>
#include <eq/net/node.h>

#include <vector>

namespace eqs
{
    class Pipe;
    class Server;

    typedef std::vector<Pipe*>::const_iterator PipeIter;

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
        uint32_t nPipes() const { return _pipes.size(); }

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
        void startInit( const uint32_t initID);

        /** 
         * Synchronize the initialisation of the node.
         * 
         * @return <code>true</code> if the node was initialised successfully,
         *         <code>false</code> if not.
         */
        bool syncInit();
        
        /** 
         * Starts exiting this node.
         */
        void startExit();

        /** 
         * Synchronize the exit of the node.
         * 
         * @return <code>true</code> if the node exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncExit();
        
        /** 
         * Send the node the command to stop its execution.
         */
        void stop();

        /** 
         * Trigger the rendering of a new frame for this node.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void update( const uint32_t frameID );

        /**
         * Finish one frame.
         *
         * @param frame the number of the frame to complete.
         */
        void syncUpdate( const uint32_t frame ) const;

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

        /** @sa eqNet::Object::send */
        bool send( eqNet::ObjectPacket& packet )
            { return eqNet::Object::send( _node, packet ); }

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

    private:
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

        /** The request identifier for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        /** The cached barriers. */
        std::vector<eqNet::Barrier*> _barrierCache;

        /** common code for all constructors */
        void _construct();

        /* Command handler functions. */
        eqNet::CommandResult _cmdInitReply( eqNet::Command& command );
        eqNet::CommandResult _cmdExitReply( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Node* node );
};
#endif // EQS_NODE_H
