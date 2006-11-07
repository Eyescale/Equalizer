
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOG_H
#define EQS_LOG_H

#include <eq/client/log.h>

namespace eqs
{
    enum LogTopics
    {
        LOG_TASKS    = eq::LOG_CUSTOM, // 4096
        LOG_ASSEMBLY = 0x2000          // 8192
    };
}
#endif // EQS_LOG_H
