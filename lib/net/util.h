
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_UTIL_H
#define EQNET_UTIL_H

#include <eq/base/base.h>

#ifdef WIN32
#  define MAXHOSTNAMELEN 255
#else
#  include <sys/param.h>
#endif

namespace eqNet
{
    namespace priv
    {
        /** 
         * Utility functions for the networking layer.
         */
        class Util
        {
        public:
            /** 
             * Parses a TCP/IP address into a hostname and a port.
             * 
             * @param address the TCP/IP address.
             * @param hostname caller-allocated output parameter for the
             *                 hostname.
             * @param port output parameter for the port.
             */
            static void parseAddress( const char* address, 
                                      char hostname[MAXHOSTNAMELEN], 
                                      uint16_t &port );
        };
    }
}

#endif // EQNET_UTIL_H

