
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_LIST_H
#define EQNET_NODE_LIST_H

#include "nodePriv.h"

#include <vector>

namespace eqNet
{
    namespace priv
    {
        struct Packet;

        /**
         * A list of nodes. 
         */
        class NodeList : public std::vector<Node*>
        {
        public:
            void send( Node* fromNode, Packet& packet ) const
                {
                    for( size_t i=0; i<size(); i++ )
                        fromNode->send( (*this)[i], packet );
                }
        };
    }
}

#endif // EQNET_NODE_LIST_H
