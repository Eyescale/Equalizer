
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
//#define EQ_UNDEFINED_INT32    (0x7fffffff)
#define EQ_TIMEOUT_INDEFINITE (0)

// XXX move to client?
#define EQ_NONE (-1)


#define EQ_DUMP_CORE {((char*)0)[0] = 'a';}

// assertions
#ifdef NDEBUG

#  define EQASSERT(x) if( !(x) ) \
        EQERROR << "##### Assert: " << #x << " #####" << std::endl;
#  define EQASSERTINFO(x, info) if( !(x) )                                \
        EQERROR << "##### Assert: " << #x << " [" << info << "] #####"    \
              << std::endl;
#  define EQUNIMPLEMENTED { EQERROR << "Unimplemented code" << std::endl; }
#  define EQUNREACHABLE   { EQERROR << "Unreachable code" << std::endl; }

#else

#  define EQASSERT(x) if( !(x) ) \
    { EQERROR << "Assert: " << #x << std::endl; EQ_DUMP_CORE; ::abort(); }
#  define EQASSERTINFO(x, info) if( !(x) )                              \
    {                                                                   \
        EQERROR << "Assert: " << #x << " [" << info << "]" << std::endl; \
        EQ_DUMP_CORE; ::abort();                                        \
    }
#  define EQUNIMPLEMENTED                                               \
    { EQERROR << "Unimplemented code" << std::endl; ::abort(); EQ_DUMP_CORE; }
#  define EQUNREACHABLE                                                 \
    { EQERROR << "Unreachable code" << std::endl; ::abort(); EQ_DUMP_CORE; }

#endif

// thread-safety checks
#ifdef CHECK_THREADSAFETY
#  define CHECK_THREAD( THREADID )                                      \
    {                                                                   \
        if( THREADID==0 )                                               \
        {                                                               \
            THREADID = pthread_self();                                  \
            EQINFO << "Functions for " << #THREADID                     \
                   << " locked to this thread" << std::endl;            \
        }                                                               \
        if( !pthread_equal( THREADID, pthread_self( )))                 \
        {                                                               \
            EQERROR << "Threadsafety check for " << #THREADID           \
                    << " failed on object of class "                    \
                    << typeid(*this).name() << endl;                    \
            EQASSERTINFO( 0, "Non-threadsave code called from two threads" ); \
        }                                                               \
    }

#  define CHECK_NOT_THREAD( THREADID )                                  \
    {                                                                   \
        if( THREADID )                                                  \
        {                                                               \
            if( pthread_equal( THREADID, pthread_self( )))              \
            {                                                           \
                EQERROR << "Threadsafety check for not " << #THREADID   \
                        << " failed on object of class "                \
                        << typeid(*this).name() << endl;                \
                EQASSERTINFO( 0, "Code called from wrong thread" );     \
            }                                                           \
        }                                                               \
    }
#else
#  define CHECK_THREAD( THREADID )
#  define CHECK_NOT_THREAD( THREADID )
#endif
    
#endif //EQBASE_BASE_H
