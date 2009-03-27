
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
#  define EQABORT( info ) {                                         \
        EQERROR << "##### Abort: " << info << " #####" << std::endl \
                << eq::base::forceFlush; }

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
#  define EQABORT( info ) {                                             \
        EQERROR << "Abort: " << info << std::endl << eq::base::forceFlush; \
        eq::base::abortDebug(); }

#endif // DEBUG

#define EQSAFECAST( to, in ) static_cast< to >( in );   \
    EQASSERT( in == 0 || dynamic_cast< to >( static_cast< to >( in )))


#endif //EQBASE_DEBUG_H
