
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include <eq/net/node.h>

namespace eq
{
    class Node : public eqNet::Node
    {
    public:
    protected:
        /** 
         * @sa eqNet::Node::handlePacket
         */
        virtual void handlePacket( eqNet::Node* node, 
                                   const eqNet::Packet* packet );

        /** 
         * @sa eqNet::Node::handleCommand
         */
        virtual void handleCommand( eqNet::Node* node, 
                                    const eqNet::NodePacket* packet );
    private:
    };
}

#endif // EQ_NODE_H

