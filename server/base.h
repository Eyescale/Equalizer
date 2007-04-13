
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_BASE_H
#define EQS_BASE_H

#include <eq/base/base.h>

#ifdef WIN32
#  ifdef EQUALIZERSERVERLIBRARY_EXPORTS
#    define EQS_EXPORT EQ_DLLEXPORT
#    define EQS_STLEXTERN 
#  else
#    define EQS_EXPORT EQ_DLLIMPORT
#    define EQS_STLEXTERN extern
#  endif
#else
#  define EQS_DLLEXPORT
#  define EQS_DLLIMPORT
#  define EQS_EXPORT
#endif

#endif // EQS_BASE_H
