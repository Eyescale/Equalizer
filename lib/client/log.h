
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_LOG_H
#define EQ_LOG_H

#include <eq/net/log.h>

namespace eq
{
    enum LogTopics
    {
        LOG_ASSEMBLY = eq::net::LOG_CUSTOM,  // 256
        LOG_TASKS    = 0x200,              // 512
        LOG_EVENTS   = 0x400,              // 1024
        LOG_STATS    = 0x800,              // 2048
        LOG_SERVER   = 0x1000,             // 4096
        LOG_CUSTOM   = 0x10000             // 65536
    };
}
#endif // EQ_LOG_H
