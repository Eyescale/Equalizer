
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_PRIV_H
#define EQNET_NODE_PRIV_H

#include <eq/base/base.h>

#include "node.h"
#include "base.h"

namespace eqNet
{
    namespace priv
    {
        class Node : public Base, public eqNet::Node
        {
        public:
            /** 
             * Constructs a new Node.
             * 
             * @param id the identifier of the node.
             */
            Node(const uint id);

        protected:
        };
    }
}

#endif // EQNET_NODE_PRIV_H
