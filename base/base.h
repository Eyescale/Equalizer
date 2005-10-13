
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// type definitions
#ifndef ushort
typedef unsigned short ushort;
#endif // ushort

#ifndef uint32
typedef uint32_t uint32;
#endif // uint32

#ifndef uint64
typedef uint64_t uint64;
#endif // uint64

#ifdef sgi
typedef int socklen_t;
#endif

// defines
#define EQ_TIMEOUT_INDEFINITE 0

// assertions
#ifdef NDEBUG
#  define ASSERT(x) if( !(x) ) ERROR << "Assert: " << #x << endl;
#else
#  define ASSERT(x) if( !(x) ) \
    { ERROR << "Assert: " << #x << std::endl; ::abort(); }
#endif

#endif //EQBASE_BASE_H
