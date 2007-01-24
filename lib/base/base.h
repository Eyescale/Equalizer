
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#include <eq/base/defines.h>

#ifdef WIN32
#  define _USE_MATH_DEFINES
#  define _WIN32_WINNT 0x500
#  include <Winsock2.h>
#  include <Windows.h>
#  define MAX __max
#  define MIN __min
#  define EQ_DLLEXPORT __declspec(dllexport) 
#  define EQ_DLLIMPORT __declspec(dllimport)
#  ifdef EQUALIZER_EXPORTS
#    define EQ_EXPORT EQ_DLLEXPORT
#    define EQ_STLEXTERN 
#  else
#    define EQ_EXPORT EQ_DLLIMPORT
#    define EQ_STLEXTERN extern
#  endif
#else
#  define EQ_DLLEXPORT
#  define EQ_DLLIMPORT
#  define EQ_EXPORT
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#ifndef WIN32
#  include <cstdint>
#endif

// type definitions
#ifdef sgi
typedef int socklen_t;
#endif

#ifdef Darwin
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif

#ifdef WIN32
typedef UINT64     uint64_t;
typedef INT64      int64_t;
typedef UINT32     uint32_t;
typedef INT32      int32_t;
typedef UINT16     uint16_t;
typedef UINT8      uint8_t;
typedef int        socklen_t;
typedef SSIZE_T    ssize_t;
#endif

// defines
#define EQ_UNDEFINED_UINT32   (0xffffffff)
//#define EQ_UNDEFINED_INT32    (0x7fffffff)
#define EQ_TIMEOUT_INDEFINITE 0

#endif //EQBASE_BASE_H
