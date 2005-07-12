
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DYNAMIC_NETWORK_H
#define EQNET_DYNAMIC_NETWORK_H

#include "networkPriv.h"

#include <iostream> 

namespace eqNet
{
    namespace priv
    {
        /** A Network base class for dynamic-based networks. */
        class DynamicNetwork : public Network
        {
        public:
            DynamicNetwork( const uint id, Session* session ) 
                    : Network( id, session ) {}
            ~DynamicNetwork(){ exit(); }

            virtual bool init();
            virtual void exit();

        protected:
        };
    }
}

#endif //EQNET_DYNAMIC_NETWORK_H
