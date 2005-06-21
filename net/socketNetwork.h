
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SOCKET_NETWORK_H
#define EQNET_SOCKET_NETWORK_H

#include "networkInt.h"

namespace eqNet
{
    namespace internal
    {
        class SocketNetwork : public Network
        {
        public:
            SocketNetwork(const uint id) : Network(id){}
        };
    }
}

#endif //EQNET_SOCKET_NETWORK_H
