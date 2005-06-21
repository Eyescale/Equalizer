
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_NETWORK_H
#define EQNET_PIPE_NETWORK_H

#include "networkPriv.h"

namespace eqNet
{
    namespace priv
    {
        class PipeNetwork : public Network
        {
        public:
            PipeNetwork(const uint id) : Network(id){}

            virtual bool init();
            virtual bool start() ;
            virtual void stop();
            virtual bool startNode(const uint nodeID);
        };
    }
}

#endif //EQNET_PIPE_NETWORK_H
