
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include "commands.h"
#include "config.h"

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

        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the init method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //@}
 
    private:
        friend class Config;
        Config*                _config;

        std::vector<Pipe*>     _pipes;

        /** The reason for the last error. */
        std::string            _error;

        /** The receiver->node thread command queue. */
        eqNet::CommandQueue    _commandQueue;

        /** The node thread. */
        class NodeThread : public eqBase::Thread
        {
        public:
            NodeThread( Node* node ) 
                    : _node( node ) {}
            virtual void* run(){ return _node->_runThread(); }
        private:
            Node* _node;
        };
        NodeThread* _thread;

        void* _runThread();

        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );

        /** 
         * Push a command from the receiver to the node thread to be handled
         * asynchronously.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        eqNet::CommandResult _pushCommand( eqNet::Command& command )
            {_commandQueue.push( command ); return eqNet::COMMAND_HANDLED;}

        /** The command functions. */
        eqNet::CommandResult _cmdCreatePipe( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyPipe( eqNet::Command& command );
        eqNet::CommandResult _cmdInit( eqNet::Command& command );
        eqNet::CommandResult _reqInit( eqNet::Command& command );
        eqNet::CommandResult _reqExit( eqNet::Command& command );
    };
}

#endif // EQ_NODE_H

