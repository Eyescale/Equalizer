
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NETWORK_PRIV_H
#define EQNET_NETWORK_PRIV_H

#include <eq/net/network.h>

#include "base.h"

namespace eqNet
{
    namespace priv
    {
        class Network : public eqNet::Network, public Base
        {
        public:
            /** 
             * Constructs a new Network.
             * 
             * @param id the identifier of the network.
             * @param protocol the network protocol.
             * @return the network.
             */
            static Network* create( const uint id,
                const eqNet::NetworkProtocol protocol );

        protected:
            Network(const uint id);

            /** The identifier of this Network. */
            uint _id;

        private:
        };
    }
}

#endif // EQNET_NETWORK_PRIV_H
