
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "log.h"
#include "thread.h"

namespace eqBase
{

EQ_EXPORT bool init()
{
    return true;
}

EQ_EXPORT bool exit()
{
    eqBase::Thread::removeAllListeners();
    eqBase::Log::exit();
    return true;
}
}
