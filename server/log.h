
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_LOG_H
#define EQSERVER_LOG_H

#include <eq/client/log.h>

namespace eq
{
namespace server
{
    enum LogTopics
    {
        LOG_LB       = LOG_SERVER << 0,   // 8192
        LOG_VIEW     = LOG_SERVER << 1    // 16384
    };
}
}
#endif // EQSERVER_LOG_H
