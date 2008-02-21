
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include <eq/client/commandQueue.h> // member
#include <eq/client/nodeType.h>     // for TYPE_EQ_CLIENT enum
#include <eq/net/command.h>         // member
#include <eq/net/node.h>            // base class

namespace eq
{
    class Server;

    /** 
     * The client represents a network node in the cluster.
     */
    class EQ_EXPORT Client : public eqNet::Node
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
        bool connectServer( eqBase::RefPtr<Server> server );

        /** 
         * Disconnect and close the connection of an Equalizer server to the
         * local client.
         * 
         * @param server the server.
         * @return true if the server was disconnected, false if not.
         */
        bool disconnectServer( eqBase::RefPtr<Server> server );

        /** 
         * Get and process one command from the node command queue. Used
         * internally to run nonthreaded commands.
         */
        void processCommand();

        /** @sa eqNet::Node::listen() */
        virtual bool listen();
        /** @sa eqNet::Node::stopListening() */
        virtual bool stopListening();

        /** 
         * Set the window system for the client's message pump, used by
         * non-threaded pipes.
         */
        void setWindowSystem( const WindowSystem windowSystem );

        /** Return the command queue to the main node thread. */
        CommandQueue* getNodeThreadQueue() { return _nodeThreadQueue; }

    protected:
        /** @sa eqNet::Node::clientLoop */
        virtual bool clientLoop();
        /** @sa eqNet::Node::exitClient(). */
        virtual bool exitClient();

        /** @name Configuration. */
        //*{
        /** 
         * Enable or disable automatic or external OS event dispatch for the
         * node thread.
         *
         * @return true if Equalizer shall dispatch OS events, false if the
         *         application dispatches OS events.
         * @sa Event handling documentation on website.
         */
        virtual bool useMessagePump() { return true; }
        //*}
    private:
        /** The command->node command queue. */
        CommandQueue* _nodeThreadQueue;
        
        bool _running;

        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode( const uint32_t type );
        
        /** @sa eqNet::Node::dispatchCommand */
        virtual bool dispatchCommand( eqNet::Command& command );

        /** @sa eqNet::Node::invokeCommand */
        virtual eqNet::CommandResult invokeCommand( eqNet::Command& command );

        /** The command functions. */
        eqNet::CommandResult _cmdExit( eqNet::Command& command );
    };
}

#endif // EQ_CLIENT_H
