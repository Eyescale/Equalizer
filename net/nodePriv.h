
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_PRIV_H
#define EQNET_NODE_PRIV_H

#include <eq/base/base.h>

#include "idHash.h"
#include "networkPriv.h"
#include "node.h"
#include "base.h"

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
                    const uint toNodeID = toNode->getID();
                    if( !_nodeNetwork.containsKey( toNodeID ))
                        _nodeNetwork[toNodeID] = _findBestNetwork( toNode );
                    _nodeNetwork[toNodeID]->send( toNode, packet );
                }
                     
            bool isInNetwork( Network* network ){ return true; } // XXX
        protected:
            /** The list of networks in which this node is running. */
            std::vector<Network*> _networks;

            /** Maps a remote node the best network to reach it. */
            IDHash<Network*> _nodeNetwork;

            Network* _findBestNetwork( Node* toNode );

            friend inline std::ostream& operator << 
                (std::ostream& os, Node* node);
        };

        inline std::ostream& operator << ( std::ostream& os, Node* node )
        {
            os << "    Node " << node->getID() << "(" << (void*)node << ")" 
               << std::endl;
            
            return os;
        }
    }
}

#endif // EQNET_NODE_PRIV_H
