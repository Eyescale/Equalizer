
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_LOG_H
#define EQNET_LOG_H

#include <eq/base/log.h>

namespace eq
{
namespace net
{
    enum LogTopics
    {
        LOG_OBJECTS = base::LOG_CUSTOM << 0,  // 16
        LOG_BARRIER = base::LOG_CUSTOM << 1,  // 32
        LOG_WIRE    = base::LOG_CUSTOM << 2,  // 64
        LOG_NETPERF = base::LOG_CUSTOM << 3,  // 128
        LOG_CUSTOM  = base::LOG_CUSTOM << 4   // 256
    };
}
}
#endif // EQNET_LOG_H
