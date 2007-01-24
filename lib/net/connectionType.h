
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTIONTYPE_H
#define EQNET_CONNECTIONTYPE_H

#include <eq/base/base.h>
#include <eq/base/referenced.h>

namespace eqNet
{
    /** The supported network protocols. */
    enum ConnectionType
    {
        CONNECTIONTYPE_TCPIP,   //!< TCP/IP networking.
        CONNECTIONTYPE_PIPE,    //!< pipe() based uni-directional connection
    };
}

#endif // EQNET_CONNECTIONTYPE_H
