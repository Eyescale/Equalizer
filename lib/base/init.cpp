
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "log.h"
#include "thread.h"

namespace eq
{
namespace base
{

EQ_EXPORT bool init()
{
    Thread::pinCurrentThread();
    return true;
}

EQ_EXPORT bool exit()
{
    eq::base::Thread::removeAllListeners();
    eq::base::Log::exit();
    return true;
}

}
}

