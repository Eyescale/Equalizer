
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_LOG_H
#define EQNET_LOG_H

#include <eq/base/log.h>

namespace eqNet
{
    enum LogTopics
    {
        LOG_OBJECTS = eqBase::LOG_CUSTOM, // 16
        LOG_BARRIER = 0x20,               // 32
        LOG_CUSTOM  = 0x100               // 256
    };
}
#endif // EQNET_LOG_H
