

/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef uint64
typedef uint64_t uint64;
#endif // uint64

#ifdef sgi
typedef int socklen_t;
#endif

// TODO: make debug printing more flexible
#define WARN printf

#endif //EQBASE_BASE_H
