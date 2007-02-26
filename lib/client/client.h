
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include <eq/net/commandQueue.h> // member
#include <eq/net/node.h>         // base class

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

    private:
        /** The receiver->node command queue. */
        eqNet::CommandQueue    _commandQueue;
        
        bool _running;

        /** @sa eqNet::Node::runClient */
        virtual bool runClient( const std::string& clientArgs );

        /** @sa eqNet::Node::clientLoop */
        virtual void clientLoop();

        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode( const CreateReason
                                                        reason );
        
        /** @sa eqNet::Node::createSession */
        virtual eqNet::Session* createSession();

        /** @sa eqNet::Node::handleCommand */
        virtual eqNet::CommandResult handleCommand( eqNet::Command& command );

        /** @sa eqNet::Node::pushCommand */
        virtual bool pushCommand( eqNet::Command& command )
        { _commandQueue.push( command ); return true; }

        /** The command functions. */
        eqNet::CommandResult _reqExit( eqNet::Command& command );
    };
}

#endif // EQ_CLIENT_H
