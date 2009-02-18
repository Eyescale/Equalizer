
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_LOG_H
#define EQ_LOG_H

#include <eq/net/log.h>

namespace eq
{
    enum LogTopics
    {
        LOG_ASSEMBLY = net::LOG_CUSTOM << 0,   // 256
        LOG_TASKS    = net::LOG_CUSTOM << 1,   // 512
        LOG_EVENTS   = net::LOG_CUSTOM << 2,   // 1024
        LOG_STATS    = net::LOG_CUSTOM << 3,   // 2048
        LOG_SERVER   = net::LOG_CUSTOM << 4,   // 4096
        LOG_CUSTOM   = 0x10000                 // 65536
    };
}
#endif // EQ_LOG_H
