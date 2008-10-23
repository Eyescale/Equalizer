
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_SLEEP_H
#define EQBASE_SLEEP_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    void sleep( const uint32_t milliSeconds )
    {
#ifdef WIN32_VC
        ::Sleep( milliSeconds );
#else
        ::usleep( milliSeconds * 1000 );
#endif
    }
}
}
#endif  // EQBASE_SLEEP_H
