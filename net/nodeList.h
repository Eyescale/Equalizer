
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
            NodeList( Node* localNode ) : _localNode(localNode) 
                { EQASSERT(localNode); }

            void send( Packet& packet ) const
                {
                    for( size_t i=0; i<size(); i++ )
                        _localNode->send( (*this)[i], packet );
                }

        private:
            Node* _localNode;
        };
    }
}

#endif // EQNET_NODE_LIST_H
