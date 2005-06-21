
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_PRIV_H
#define EQNET_NODE_PRIV_H

#include <eq/base/base.h>

#include <eq/net/message.h>

namespace eqNet
{
    namespace priv
    {
        class Node
        {
        public:
            /** 
             * Constructs a new Node.
             * 
             * @param id the identifier of the node.
             */
            Node(const uint id);

        protected:
            
            /** The identifier of this Node. */
            uint _id;
            
        private:
            static uint _nextNodeID;
        };
    }
}

#endif // EQNET_NODE_PRIV_H
