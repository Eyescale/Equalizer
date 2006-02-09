
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

#define DUMP_CORE {((char*)0)[0] = 'a';}

// assertions
#ifdef NDEBUG

#  define EQASSERT(x) if( !(x) ) \
        EQERROR << "##### Assert: " << #x << " #####" << std::endl;
#  define EQASSERTINFO(x, info) if( !(x) )                                \
        EQERROR << "##### Assert: " << #x << " [" << info << "] #####"    \
              << std::endl;
#  define EQUNIMPLEMENTED { EQERROR << "Unimplemented code" << std::endl; }

#else

#  define EQASSERT(x) if( !(x) ) \
    { EQERROR << "Assert: " << #x << std::endl; DUMP_CORE; ::abort(); }
#  define EQASSERTINFO(x, info) if( !(x) )                                \
    {                                                                   \
        EQERROR << "Assert: " << #x << " [" << info << "]" << std::endl;  \
        DUMP_CORE; ::abort();                                           \
    }
#  define EQUNIMPLEMENTED                                         \
    { EQERROR << "Unimplemented code" << std::endl; DUMP_CORE; ::abort(); }

#endif

#ifdef CHECK_THREADSAFETY
#  define CHECK_THREAD                                      \
    if( !_threadID )                                        \
        _threadID = pthread_self();                         \
    EQASSERTINFO( pthread_equal( _threadID, pthread_self( )), \
                "Non-threadsave function called from two threads" );
#else
#  define CHECK_THREAD
#endif
    
#endif //EQBASE_BASE_H
