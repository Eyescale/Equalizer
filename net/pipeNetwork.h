
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_NETWORK_H
#define EQNET_PIPE_NETWORK_H

#include "networkInt.h"

namespace eqNet
{
    namespace internal
    {
        class PipeNetwork : public Network
        {
        public:
            PipeNetwork(const uint id) : Network(id){}
        };
    }
}

#endif //EQNET_PIPE_NETWORK_H
