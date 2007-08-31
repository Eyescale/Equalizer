
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
#  define EQ_DLLEXPORT __declspec(dllexport) 
#  define EQ_DLLIMPORT __declspec(dllimport)
#  ifdef EQUALIZER_EXPORTS
#    define EQ_EXPORT EQ_DLLEXPORT
#    define EQ_STLEXTERN 
#  else
#    define EQ_EXPORT EQ_DLLIMPORT
#    define EQ_STLEXTERN extern
#  endif
   // Need to predefine server library exports for forward declaration of 
   // eqsStartLocalServer
#  ifdef EQUALIZERSERVERLIBRARY_EXPORTS
#    define EQS_EXPORT EQ_DLLEXPORT
#    define EQS_STLEXTERN 
#  else
#    define EQS_EXPORT EQ_DLLIMPORT
#    define EQS_STLEXTERN extern
#  endif
#else
#  define EQ_DLLEXPORT
#  define EQ_DLLIMPORT
#  define EQ_EXPORT
#  define EQS_DLLEXPORT
#  define EQS_DLLIMPORT
#  define EQS_EXPORT
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#ifndef WIN32_VC
#  include <stdint.h>
#  include <sys/param.h>  // for MIN/MAX
#endif

#include <eq/base/types.h>

// defines
#define EQ_UNDEFINED_UINT32   (0xffffffffu)
//#define EQ_UNDEFINED_INT32    (0x7fffffffu)
#define EQ_TIMEOUT_INDEFINITE 0
#define EQ_OBJECT_CAST( type, object )                 \
    static_cast<type>( object );                       \
    EQASSERTINFO( object && !dynamic_cast<type>( object ),  \
                  "Object is not of type " << #type ); 

#endif //EQBASE_BASE_H
