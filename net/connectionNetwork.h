
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_NETWORK_H
#define EQNET_CONNECTION_NETWORK_H

#include "networkPriv.h"

#include <iostream> 

namespace eqNet
{
    namespace priv
    {
        class Connection;

        /** A 'network' base class for connection-based networks. */
        class ConnectionNetwork : public Network
        {
        public:
            ConnectionNetwork(const uint id) : Network(id){}
            ~ConnectionNetwork(){ exit(); }

            virtual bool init();
            virtual void exit();

            /** 
             * Forces a node into started mode, used during server launch
             * 
             * @param nodeID the node identifier.
             * @param connection the open connection to the node.
             */
            void setStarted( const uint nodeID, Connection* connection );

        protected:
            /** The list of active connections, indexed per node. */
            IDHash<Connection*> _connections;
        };
    }
}

#endif //EQNET_CONNECTION_NETWORK_H
