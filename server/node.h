
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_NODE_H
#define EQS_NODE_H

#include <eq/base/sema.h>
#include <eq/net/barrier.h>
#include <eq/net/node.h>
#include <eq/net/object.h>

#include <vector>

namespace eqs
{
    class Config;
    class Pipe;

    /**
     * The node.
     */
    class Node : public eqNet::Node, public eqNet::Object
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

        /** 
         * Returns the parent config of this node.
         * 
         * @return the parent config of this node.
         */
        Config* getConfig() const { return _config; }

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
         * Finish the frame[current-latency].
         */
        void syncUpdate();

        /** 
         * Adjust the latency of this node.
         * 
         * If decrementing the latency, the function blocks until the
         * corresponding frames have been finished.
         *
         * @param delta the delta of the latency change.
         */
        void adjustLatency( const int delta );
        //*}

        /**
         * @name Barrier Cache
         *
         * Caches barriers for which this node is the master.
         */
        //*{
        /** 
         * Get a barrier of a given height.
         * 
         * @param height the height of the barrier.
         * @return the barrier.
         */
        eqNet::Barrier* getBarrier( const uint32_t height );

        /** 
         * Release a barrier server by this node.
         * 
         * @param barrier the barrier.
         */
        void releaseBarrier( eqNet::Barrier* );
        //*}

        /**
         * @name Attributes
         */
        //*{
        enum SAttribute
        {
            SATTR_LAUNCHCOMMAND,
            SATTR_ALL
        };

        enum IAttribute
        {
            IATTR_LAUNCHTIMEOUT,
            IATTR_ALL
        };

        void setSAttribute( const SAttribute attr, const std::string& value )
            { _sAttributes[attr] = value; }
        const std::string& getSAttribute( const SAttribute attr ) const
            { return _sAttributes[attr]; }

        void setIAttribute( const IAttribute attr, const int value )
            { _iAttributes[attr] = value; }
        int  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }
        //*}

    protected:
        /** @sa eqNet::Node::getProgramName */
        virtual const std::string& getProgramName();

    private:
        /** The vector of pipes belonging to this node. */
        std::vector<Pipe*> _pipes;

        /** Number of entitities actively using this node. */
        uint32_t _used;

        /** The parent config. */
        Config* _config;
        friend class Config;

        /** String attributes. */
        std::string _sAttributes[SATTR_ALL];

        /** Int attributes. */
        int _iAttributes[IATTR_ALL];

        /** The request identifier for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        /** Identifiers for cached barriers, sorted by height. */
        Sgi::hash_map<uint32_t, std::vector<eqNet::Barrier*> > _barrierCache;

        enum State
        {
            STATE_STOPPED   = eqNet::Node::STATE_STOPPED,
            STATE_LAUNCHED  = eqNet::Node::STATE_LAUNCHED,
            STATE_CONNECTED = eqNet::Node::STATE_CONNECTED,
            STATE_LISTENING = eqNet::Node::STATE_LISTENING,
            STATE_INITIALISING,
            STATE_INITIALISED
        };

        /** common code for all constructors */
        void _construct();

        void _sendInit( const uint32_t initID );
        void _sendExit();

        /* Command handler functions. */
        eqNet::CommandResult _cmdInitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdExitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
    };

    std::ostream& operator << ( std::ostream& os, const Node* node );
};
#endif // EQS_NODE_H
