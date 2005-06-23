
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#include <eq/base/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// type definitions
#ifndef uint64
typedef uint64_t uint64;
#endif // uint64

#ifdef sgi
typedef int socklen_t;
#endif

#ifdef NDEBUG
#  define ASSERT(x) if( !(x) ) ERROR << "Assert: " << #x << endl;
#else
#  define ASSERT(x) if( !(x) ) \
    { ERROR << "Assert: " << #x << endl; exit(EXIT_FAILURE); }
#endif

#endif //EQBASE_BASE_H
