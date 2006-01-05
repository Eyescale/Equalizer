
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#ifndef EXCLUDE_DEFINES // defined during dependency generation
#  include "defines.h"
#endif

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// type definitions
#ifdef sgi
typedef int socklen_t;
#endif

// defines
#define EQ_UNDEFINED_UINT32   (0xffffffff)
#define EQ_TIMEOUT_INDEFINITE (0)
#define DUMP_CORE             {((char*)0)[0] = 'c';}

// assertions
#ifdef NDEBUG

#  define ASSERT(x) if( !(x) ) \
        ERROR << "##### Assert: " << #x << " #####" << std::endl;
#  define ASSERTINFO(x, info) if( !(x) )                                \
        ERROR << "##### Assert: " << #x << " [" << info << "] #####"    \
              << std::endl;

#else

#  define ASSERT(x) if( !(x) ) \
    { ERROR << "Assert: " << #x << std::endl; DUMP_CORE; ::abort(); }
#  define ASSERTINFO(x, info) if( !(x) )                                \
    {                                                                   \
        ERROR << "Assert: " << #x << " [" << info << "]" << std::endl;  \
        DUMP_CORE; ::abort();                                           \
    }

#endif

#endif //EQBASE_BASE_H
