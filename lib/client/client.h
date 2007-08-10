
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include <eq/client/commandQueue.h> // member
#include <eq/client/nodeType.h>     // for TYPE_EQ_CLIENT enum
#include <eq/net/node.h>            // base class

namespace eq
{
    class Server;

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

    protected:
        /** @sa eqNet::Node::clientLoop */
        virtual void clientLoop();

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
        /** The receiver->node command queue. */
        eqNet::CommandQueue* _commandQueue;
        
        bool _running;

        /** @sa eqNet::Node::runClient */
        virtual bool runClient( const std::string& clientArgs );

        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode( const uint32_t type );
        
        /** @sa eqNet::Node::createSession */
        virtual eqNet::Session* createSession();

        /** @sa eqNet::Node::handleCommand */
        virtual eqNet::CommandResult handleCommand( eqNet::Command& command );

        /** @sa eqNet::Node::pushCommand */
        virtual bool pushCommand( eqNet::Command& command )
        { _commandQueue->push( command ); return true; }

        /** The command functions. */
        eqNet::CommandResult _reqExit( eqNet::Command& command );
    };
}

#endif // EQ_CLIENT_H
