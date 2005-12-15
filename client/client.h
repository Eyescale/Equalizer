
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
        /** The command functions. */
        //void _cmdChooseConfigReply( eqNet::Node* node,
        //                            const eqNet::Packet* packet );
    };
}

#endif // EQ_CLIENT_H
