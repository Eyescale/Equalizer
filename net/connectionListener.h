
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_LISTENER_H
#define EQNET_CONNECTION_LISTENER_H

#include <eq/base/base.h>

namespace eqNet
{
    namespace priv
    {
        class Connection;
        class ConnectionNetwork;
        
        /**
         * The connection listener receives connection events and forwards them
         * to the network.
         */
        class ConnectionListener
        {
        public:
            ConnectionListener( ConnectionNetwork* network, const uint nodeID )
                    : _network(network), _nodeID(nodeID) {}

            void notifyData(Connection* connection);

            uint getNodeID(){ return _nodeID; }
        private:
            ConnectionNetwork* _network;
            uint               _nodeID;
        };
    }
}

#endif // EQNET_CONNECTION_LISTENER_H
