
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/net/node.h>

namespace eq
{
    class Server : protected eqNet::Node
    {
        /** 
         * Opens the connection to an Equalizer server.
         * 
         * @param address the server's address.
         * @return <code>true</code> if the server was opened correctly,
         *         <code>false</code> otherwise.
         */
        bool open( std::string& address );
    };
}

#endif // EQNET_SERVER_H

