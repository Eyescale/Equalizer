
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#include <eq/base/defines.h>

#include "log.h"

#include <stdio.h>
#include <stdint.h>
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

#define EQ_DUMP_CORE {((char*)0)[0] = 'a';}

// assertions
#ifdef NDEBUG

#  define EQASSERT(x) if( !(x) ) \
        EQERROR << "##### Assert: " << #x << " #####" << std::endl \
                << eqBase::forceFlush;
#  define EQASSERTINFO(x, info) if( !(x) )                                \
        EQERROR << "##### Assert: " << #x << " [" << info << "] #####"    \
              << std::endl << eqBase::forceFlush;
#  define EQUNIMPLEMENTED { EQERROR << "Unimplemented code for "        \
                                    << typeid(*this).name() << std::endl \
                                    << eqBase::forceFlush; }
#  define EQUNREACHABLE   { EQERROR << "Unreachable code for "          \
                                    << typeid(*this).name() << std::endl \
                                    << eqBase::forceFlush; }

#else

#  define EQASSERT(x) if( !(x) ) \
    { EQERROR << "Assert: " << #x << std::endl << eqBase::forceFlush; \
        EQ_DUMP_CORE; ::abort(); }
#  define EQASSERTINFO(x, info) if( !(x) )                              \
    {                                                                   \
        EQERROR << "Assert: " << #x << " [" << info << "]" << std::endl \
                << eqBase::forceFlush;                                  \
        EQ_DUMP_CORE; ::abort();                                        \
    }
#  define EQUNIMPLEMENTED                                               \
    { EQERROR << "Unimplemented code in " << typeid(*this).name() \
              << std::endl << eqBase::forceFlush;                  \
        ::abort(); EQ_DUMP_CORE; }
#  define EQUNREACHABLE                                                 \
    { EQERROR << "Unreachable code in " << typeid(*this).name() \
              << std::endl << eqBase::forceFlush;                \
        ::abort(); EQ_DUMP_CORE; }

#endif

// thread-safety checks
// These checks are for development purposes, to check that certain objects are
// properly used within the framework. Leaving them enabled during application
// developement may cause false positives, e.g. when threadsafety is ensured
// outside of the objects by the application.

#ifdef CHECK_THREADSAFETY
#  define CHECK_THREAD_DECLARE( NAME )                    \
    struct NAME ## Struct                                 \
    {                                                     \
        NAME ## Struct () : id( 0 ), extMutex( false ) {} \
        mutable pthread_t id;                             \
        bool extMutex;                                    \
    } NAME;
#  define CHECK_THREAD( NAME )                                          \
    {                                                                   \
        if( NAME.id==0 )                                                \
        {                                                               \
            NAME.id = pthread_self();                                   \
            EQINFO << "Functions for " << #NAME                         \
                   << " locked to this thread" << std::endl;            \
        }                                                               \
        if( !NAME.extMutex && !pthread_equal( NAME.id, pthread_self( ))) \
        {                                                               \
            EQERROR << "Threadsafety check for " << #NAME               \
                    << " failed on object of class "                    \
                    << typeid(*this).name() << endl;                    \
            EQASSERTINFO( 0, "Non-threadsave code called from two threads" ); \
        }                                                               \
    }

#  define CHECK_NOT_THREAD( NAME )                                      \
    {                                                                   \
        if( !NAME.extMutex && NAME.id )                                 \
        {                                                               \
            if( pthread_equal( NAME.id, pthread_self( )))               \
            {                                                           \
                EQERROR << "Threadsafety check for not " << #NAME       \
                        << " failed on object of class "                \
                        << typeid(*this).name() << endl;                \
                EQASSERTINFO( 0, "Code called from wrong thread" );     \
            }                                                           \
        }                                                               \
    }
#else
#  define CHECK_THREAD_DECLARE( NAME )
#  define CHECK_THREAD( NAME )
#  define CHECK_NOT_THREAD( NAME )
#endif
    
#endif //EQBASE_BASE_H
