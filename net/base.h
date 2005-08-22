
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_H
#define EQNET_BASE_H

#include <eq/base/base.h>

namespace eqNet
{
    class Connection;
    class Node;
    struct Packet;

    /** The base class for all networked objects. */
    class Base
    {
    public:
 
    protected:
        /** 
         * The default handler for handling commands.
         * 
         * @param node the originating node.
         * @param packet the packet.
         */
        void _cmdUnknown( Node* node, const Packet* packet );

    private:
    };
}

/** 
 * Entry function to run the node on the 'remote' side of a PipeConnection.
 * 
 * @param connection the connection.
 */
extern "C" void eqNet_Node_runServer( eqNet::Connection* connection );


#endif // EQNET_BASE_H
