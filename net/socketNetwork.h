
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SOCKET_NETWORK_H
#define EQNET_SOCKET_NETWORK_H

#include "connectionNetwork.h"

namespace eqNet
{
    namespace priv
    {
        class SocketNetwork : public ConnectionNetwork
        {
        public:
            SocketNetwork(const uint id) : ConnectionNetwork(id){}

            virtual bool start();
            virtual void stop();
            virtual bool startNode(const uint nodeID);
        };
    }
}

#endif //EQNET_SOCKET_NETWORK_H
