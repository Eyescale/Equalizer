
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_PRIV_H
#define EQNET_NODE_PRIV_H

#include <eq/base/base.h>

#include "basePriv.h"
#include "idHash.h"
#include "networkPriv.h"
#include "node.h"

#include <algorithm>
#include <vector>

namespace eqNet
{
    namespace priv
    {
        class Packet;
        class Session;

        class Node : public Base, public eqNet::Node
        {
        public:
            /** 
             * Constructs a new Node.
             * 
             * @param id the identifier of the node.
             * @param session the parent session.
             */
            Node( const uint id );
            
            /** 
             * Sends a packet to a node using the best network.
             * 
             * @param toNode the receiver node.
             * @param packet the packet.
             */
            void send( Node* toNode, const Packet& packet )
                {
                    if( !_nodeNetwork[toNode])
                        _nodeNetwork[toNode] = _findBestNetwork( toNode );
                    _nodeNetwork[toNode]->send( toNode, packet );
                }

            void addNetwork( Network* network ) { _networks.push_back(network);}
            bool isInNetwork( Network* network )
                { 
                    return (std::find( _networks.begin(), _networks.end(), 
                                       network) != _networks.end() );
                }

        protected:
            /** The list of networks in which this node is running. */
            std::vector<Network*> _networks;

            /** Maps a remote node the best network to reach it. */
            eqBase::PtrHash<Node*, Network*> _nodeNetwork;

            Network* _findBestNetwork( Node* toNode );
        };

        inline std::ostream& operator << ( std::ostream& os, const Node* node )
        {
            if( node )
                os << "node " << node->getID() << "(" << (void*)node << ")";
            else
                os << "NULL node";

            return os;
        }
    }
}

#endif // EQNET_NODE_PRIV_H
