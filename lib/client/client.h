
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include <eq/client/commandQueue.h> // member
#include <eq/client/nodeType.h>     // for TYPE_EQ_CLIENT enum
#include <eq/client/types.h>        // basic types

#include <eq/net/command.h>         // member
#include <eq/net/node.h>            // base class

namespace eq
{
    class Server;

    /** 
     * The client represents a network node in the cluster.
     */
    class EQ_EXPORT Client : public net::Node
    {
    public:
        /** 
         * Constructs a new client.
         */
        Client();

        /**
         * Destructs the client.
         */
        virtual ~Client();

        /** 
         * Open and connect an Equalizer server to the local client.
         * 
         * @param server the server.
         * @return true if the server was connected, false if not.
         */
        bool connectServer( ServerPtr server );

        /** 
         * Disconnect and close the connection of an Equalizer server to the
         * local client.
         * 
         * @param server the server.
         * @return true if the server was disconnected, false if not.
         */
        bool disconnectServer( ServerPtr server );

        /** 
         * Get and process one command from the node command queue. Used
         * internally to run nonthreaded commands.
         */
        void processCommand();

        /** @sa net::Node::listen() */
        virtual bool listen();
        /** @sa net::Node::stopListening() */
        virtual bool stopListening();

        /** 
         * Set the window system for the client's message pump, used by
         * non-threaded pipes.
         * @internal
         */
        void setWindowSystem( const WindowSystem windowSystem );

        /** Return the command queue to the main node thread. */
        CommandQueue* getNodeThreadQueue() { return _nodeThreadQueue; }

    protected:
        /** @sa net::Node::clientLoop */
        virtual bool clientLoop();
        /** @sa net::Node::exitClient(). */
        virtual bool exitClient();

    private:
        /** The command->node command queue. */
        CommandQueue* _nodeThreadQueue;
        
        bool _running;

        /** @sa net::Node::createNode */
        virtual net::NodePtr createNode( const uint32_t type );
        
        /** @sa net::Node::dispatchCommand */
        virtual bool dispatchCommand( net::Command& command );

        /** @sa net::Node::invokeCommand */
        virtual net::CommandResult invokeCommand( net::Command& command );

        /** The command functions. */
        net::CommandResult _cmdExit( net::Command& command );
    };
}

#endif // EQ_CLIENT_H
