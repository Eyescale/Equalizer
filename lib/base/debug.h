
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_DEBUG_H
#define EQBASE_DEBUG_H

#include <eq/base/defines.h>
#include <eq/base/log.h>

// assertions
#define EQ_NO_RELEASE_ASSERT

#ifdef NDEBUG
#  ifdef EQ_NO_RELEASE_ASSERT
#    define EQASSERT( x )
#    define EQASSERTINFO( x, info )
#    define EQCHECK(x) { x; }
#  else
#    define EQASSERT(x) { if( !(x) )                                      \
              EQERROR << "##### Assert: " << #x << " #####" << std::endl  \
                      << eq::base::forceFlush; }
#    define EQASSERTINFO(x, info) { if( !(x) )                            \
              EQERROR << "##### Assert: " << #x << " [" << info << "] #####" \
                      << std::endl << eq::base::forceFlush; }
#    define EQCHECK(x) { const bool eqResult = x; EQASSERTINFO( eqResult, #x ) }
#  endif
#  define EQUNIMPLEMENTED { EQERROR << "Unimplemented code" << std::endl \
                                    << eq::base::forceFlush; }
#  define EQUNREACHABLE   { EQERROR << "Unreachable code" << std::endl  \
                                    << eq::base::forceFlush; }
#  define EQDONTCALL                                                    \
    { EQERROR << "Code is not supposed to be called in this context"    \
              << std::endl << eq::base::forceFlush; }

#else // DEBUG
namespace eq
{
namespace base
{
    /** Used to trap into an infinite loop to allow debugging of assertions */
    EQ_EXPORT void abortDebug();
}
}

#  define EQASSERT(x) { if( !(x) )                                      \
        { EQERROR << "Assert: " << #x << std::endl << eq::base::forceFlush; \
      eq::base::abortDebug(); }}
#  define EQASSERTINFO(x, info) { if( !(x) )                            \
        {                                                               \
            EQERROR << "Assert: " << #x << " [" << info << "]" << std::endl \
                    << eq::base::forceFlush;                              \
            eq::base::abortDebug();                                       \
        }}
#  define EQUNIMPLEMENTED                                               \
    { EQERROR << "Unimplemented code in " << typeid(*this).name()       \
              << std::endl << eq::base::forceFlush;                       \
        eq::base::abortDebug(); }
#  define EQUNREACHABLE                                          \
    { EQERROR << "Unreachable code in " << typeid(*this).name()  \
              << std::endl << eq::base::forceFlush;                \
        eq::base::abortDebug(); }
#  define EQDONTCALL                                                    \
    { EQERROR << "Code is not supposed to be called in this context, type " \
              << typeid(*this).name() << std::endl << eq::base::forceFlush; \
        eq::base::abortDebug(); }

#  define EQCHECK(x) { const bool eqResult = x; EQASSERTINFO( eqResult, #x ) }

#endif // DEBUG



#endif //EQBASE_DEBUG_H
