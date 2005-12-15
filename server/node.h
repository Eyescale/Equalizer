
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_NODE_H
#define EQS_NODE_H

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
        uint nPipes() const { return _pipes.size(); }

        /** 
         * Gets a pipe.
         * 
         * @param index the pipe's index. 
         * @return the pipe.
         */
        Pipe* getPipe( const uint index ) const { return _pipes[index]; }

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
         */
        void startInit();

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
        //*}

    protected:
        /** @sa eqNet::Node::getProgramName */
        virtual const std::string& getProgramName();

    private:
        /** The vector of pipes belonging to this node. */
        std::vector<Pipe*> _pipes;

        /** Number of entitities actively using this node. */
        uint _used;

        /** The parent config. */
        Config* _config;
        friend class Config;

        /** The request identifier for pending asynchronous operations. */
        uint _pendingRequestID;

        enum State
        {
            STATE_STOPPED   = eqNet::Node::STATE_STOPPED,
            STATE_LAUNCHED  = eqNet::Node::STATE_LAUNCHED,
            STATE_CONNECTED = eqNet::Node::STATE_CONNECTED,
            STATE_LISTENING = eqNet::Node::STATE_LISTENING,
            STATE_INITIALISING,
            STATE_INITIALISED
        };


        void _sendInit();
        void _sendExit();

        void _cmdInitReply( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdExitReply( eqNet::Node* node, const eqNet::Packet* packet );
    };

    std::ostream& operator << ( std::ostream& os, const Node* node );
};
#endif // EQS_NODE_H
