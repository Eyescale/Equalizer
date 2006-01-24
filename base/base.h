
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#include <eq/base/defines.h>

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

// assertions
#ifdef NDEBUG

#  define ASSERT(x) if( !(x) ) \
        ERROR << "##### Assert: " << #x << " #####" << std::endl;
#  define ASSERTINFO(x, info) if( !(x) )                                \
        ERROR << "##### Assert: " << #x << " [" << info << "] #####"    \
              << std::endl;
#  define UNIMPLEMENTED { ERROR << "Unimplemented code" << std::endl; }

#else

#  define ASSERT(x) if( !(x) ) \
    { ERROR << "Assert: " << #x << std::endl; ::abort(); }
#  define ASSERTINFO(x, info) if( !(x) )                                \
    {                                                                   \
        ERROR << "Assert: " << #x << " [" << info << "]" << std::endl;  \
        ::abort();                                           \
    }
#  define UNIMPLEMENTED                                         \
    { ERROR << "Unimplemented code" << std::endl; ::abort(); }

#endif

#ifdef CHECK_THREADSAFETY
#  define CHECK_THREAD                                      \
    if( !_threadID )                                        \
        _threadID = pthread_self();                         \
    ASSERTINFO( pthread_equal( _threadID, pthread_self( )), \
                "Called from two threads" );
#else
#  define CHECK_THREAD
#endif
    
#endif //EQBASE_BASE_H
