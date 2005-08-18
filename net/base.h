
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_H
#define EQNET_BASE_H

#include <eq/base/base.h>

namespace eqNet
{
    class Connection;
    class Node;
    struct Packet;

    /** The base class for all networked objects. */
    class Base
    {
    public:
 
    protected:
        /** 
         * The default handler for handling commands.
         * 
         * @param packet the packet.
         */
        void _cmdUnknown( Node* node, const Packet* packet );

    private:
    };
}

#endif // EQNET_BASE_H
