
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_LOG_H
#define EQ_LOG_H

#include <eq/net/log.h>

namespace eq
{
    enum LogTopics
    {
        LOG_ASSEMBLY = eqNet::LOG_CUSTOM,  // 256
        LOG_TASKS    = 0x200,              // 512
        LOG_CUSTOM   = 0x1000              // 4096
    };
}
#endif // EQ_LOG_H
