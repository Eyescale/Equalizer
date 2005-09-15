
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
        /** 
         * Returns the local node instance.

         * @return the local node instance.
         */
        static Node* getLocalNode() { return _localNode; }

    protected:
        virtual void handleCommand( eqNet::Node* node, 
                                    const eqNet::NodePacket* packet );

    private:
        static Node* _localNode;
    };
}

#endif // EQ_NODE_H

