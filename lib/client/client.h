
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include "commands.h"

#include <eq/net/node.h>

namespace eq
{
    class Client : public eqNet::Node
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
         * Push a request from the receiver to the app thread to be handled
         * asynchronously. 
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        void pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
            { _requestQueue.push( node, packet ); }

    protected:
        /** @sa eqNet::Node::clientLoop */
        virtual void clientLoop();

        /** 
         * @sa eqNet::Node::handlePacket
         */
        virtual eqNet::CommandResult handlePacket( eqNet::Node* node, 
                                                   const eqNet::Packet* packet);

        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode( const CreateReason
                                                        reason );
        
        /** @sa eqNet::Node::createSession */
        virtual eqNet::Session* createSession();

    private:
        /** The receiver->node thread request queue. */
        eqNet::RequestQueue    _requestQueue;
        bool                   _clientLoopRunning;

        /** The command functions. */
        eqNet::CommandResult _cmdStop( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqStop( eqNet::Node* node,
                                       const eqNet::Packet* packet );
    };
}

#endif // EQ_CLIENT_H
