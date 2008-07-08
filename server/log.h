
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
        LOG_LB       = eq::LOG_SERVER  // 4096
    };
}
}
#endif // EQSERVER_LOG_H
