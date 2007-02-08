
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_DEBUG_H
#define EQBASE_DEBUG_H

#include <eq/base/log.h>

// assertions
#ifdef NDEBUG

#  define EQASSERT(x) { if( !(x) )                                 \
        EQERROR << "##### Assert: " << #x << " #####" << std::endl \
                << eqBase::forceFlush; }
#  define EQASSERTINFO(x, info) { if( !(x) )                            \
            EQERROR << "##### Assert: " << #x << " [" << info << "] #####" \
                    << std::endl << eqBase::forceFlush; }
#  define EQUNIMPLEMENTED { EQERROR << "Unimplemented code" << std::endl \
                                    << eqBase::forceFlush; }
#  define EQUNREACHABLE   { EQERROR << "Unreachable code" << std::endl  \
                                    << eqBase::forceFlush; }
#  define EQDONTCALL                                                    \
    { EQERROR << "Code is not supposed to be called in this context"    \
              << std::endl << eqBase::forceFlush; }

#else // DEBUG
#  define EQASSERT(x) { if( !(x) )                                      \
    { EQERROR << "Assert: " << #x << std::endl << eqBase::forceFlush;   \
      ::abort(); }}
#  define EQASSERTINFO(x, info) { if( !(x) )                            \
        {                                                               \
            EQERROR << "Assert: " << #x << " [" << info << "]" << std::endl \
                    << eqBase::forceFlush;                              \
            ::abort();                                                  \
        }}
#  define EQUNIMPLEMENTED                                               \
    { EQERROR << "Unimplemented code in " << typeid(*this).name()       \
              << std::endl << eqBase::forceFlush;                       \
        ::abort(); }
#  define EQUNREACHABLE                                          \
    { EQERROR << "Unreachable code in " << typeid(*this).name()  \
              << std::endl << eqBase::forceFlush;                \
        ::abort(); }
#  define EQDONTCALL                                                    \
    { EQERROR << "Code is not supposed to be called in this context, type " \
              << typeid(*this).name() << std::endl << eqBase::forceFlush; \
        ::abort(); }

#endif
#endif //EQBASE_DEBUG_H
