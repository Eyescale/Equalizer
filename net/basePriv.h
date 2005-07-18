
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
             * The default handler for handling packets.
             * 
             * @param connection the connection which received the packet.
             * @param packet the packet.
             */
            void _handleUnknown( Connection* connection,
                                 const Packet* packet );
        };
    }
}

#endif // EQNET_BASE_H
