
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_SLEEP_H
#define EQBASE_SLEEP_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    void sleep( const uint32_t seconds )
    {
#ifdef WIN32_VC
        Sleep( seconds * 1000 );
#else
        ::sleep( seconds );
#endif
    }
}
}
#endif  // EQBASE_SLEEP_H
