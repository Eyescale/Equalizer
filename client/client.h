
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

    protected:
        /** 
         * @sa eqNet::Node::handlePacket
         */
        virtual void handlePacket( eqNet::Node* node, 
                                   const eqNet::Packet* packet );

    private:
        void _handleCommand( const eqNet::Packet* packet );

        /** The command handler function table. */
        void (eq::Client::*_cmdHandler[CMD_NODE_ALL])
            ( const eqNet::Packet* packet );

        void _cmdUnknown( const eqNet::Packet* packet );
        //void _cmdChooseConfigReply( const eqNet::Packet* packet );
    };
}

#endif // EQ_CLIENT_H
