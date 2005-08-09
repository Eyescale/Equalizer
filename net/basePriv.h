
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_PRIV_H
#define EQNET_BASE_PRIV_H

namespace eqNet
{
    namespace priv
    {
        class  Connection;
        struct Packet;

        /** The internal base class for all networked objects. */
        class Base
        {
        protected:
            /** 
             * The default handler for handling commands.
             * 
             * @param packet the packet.
             */
            void _cmdUnknown( Packet* packet );
        };
    }
}

#endif // EQNET_BASE_H
