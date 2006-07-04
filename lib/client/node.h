
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include "commands.h"
#include "config.h"

#include <eq/base/gate.h>

namespace eq
{
    class Config;
    class Pipe;
    class Server;

    class Node : public eqNet::Object
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

        /** 
         * Gets a pipe.
         * 
         * @param index the pipe's index. 
         * @return the pipe.
         */
        Pipe* getPipe( const uint32_t index ) const
            { return _pipes[index]; }

    protected:
        /**
         * Destructs the node.
         */
        virtual ~Node();

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this node.
         * 
         * @param initID the init identifier.
         */
        virtual bool init( const uint32_t initID ){ return true; }

        /** 
         * Exit this node.
         */
        virtual bool exit(){ return true; }
        //@}

    private:
        friend class Config;
        Config*                _config;

        std::vector<Pipe*>     _pipes;

        /** The receiver->node thread request queue. */
        eqNet::RequestQueue    _requestQueue;

        /** The node thread. */
        class NodeThread : public eqBase::Thread
        {
        public:
            NodeThread( Node* node ) 
                    : eqBase::Thread( Thread::PTHREAD ),
                      _node( node ) {}
            virtual ssize_t run(){ return _node->_runThread(); }
        private:
            Node* _node;
        };
        NodeThread* _thread;

        ssize_t _runThread();

        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );

        /** 
         * Push a request from the receiver to the node thread to be handled
         * asynchronously.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        void _pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
            { _requestQueue.push( node, packet ); }

        /** The command functions. */
        eqNet::CommandResult _cmdCreatePipe( eqNet::Node* node,
                                             const eqNet::Packet* packet );
        eqNet::CommandResult _cmdDestroyPipe( eqNet::Node* node,
                                              const eqNet::Packet* packet );
        eqNet::CommandResult _cmdInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqExit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
    };
}

#endif // EQ_NODE_H

