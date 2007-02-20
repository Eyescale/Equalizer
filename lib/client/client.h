
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include <eq/client/commands.h>

#include <eq/base/monitor.h>
#include <eq/net/node.h>

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

        /** @name Referenced by node threads. */
        //*{
        void refUsed() { ++_used; }
        void unrefUsed() { --_used; }
        //*}

        /** @sa eqNet::Node::runClient */
        virtual bool runClient( const std::string& clientArgs );

    protected:
        /** @sa eqNet::Node::clientLoop */
        virtual void clientLoop();

        /** @sa eqNet::Node::handleCommand */
        virtual eqNet::CommandResult handleCommand( eqNet::Command& command );

        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode( const CreateReason
                                                        reason );
        
        /** @sa eqNet::Node::createSession */
        virtual eqNet::Session* createSession();

    private:
        eqBase::Monitor<uint32_t> _used;
    };
}

#endif // EQ_CLIENT_H
