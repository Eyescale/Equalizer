
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#ifndef COBASE_DEBUG_H
#define COBASE_DEBUG_H

#include <co/base/defines.h>
#include <co/base/log.h>
#include <typeinfo>

#ifndef _WIN32
#  include <cxxabi.h>
#  include <stdlib.h>
#endif

// assertions
// #define EQ_RELEASE_ASSERT

namespace co
{
namespace base
{
/**
 * Used to trap into an infinite loop to allow debugging of assertions
 * @internal
 */
COBASE_API void abort();

/**
 * Check the consistency of the heap and abort on error (Win32 only).
 * @internal
 */
COBASE_API void checkHeap();

/** 
 * Print a textual description of the current system error.
 *
 * The current system error is OS-specific, e.g., errno or GetLastError().
 * @version 1.0
 */
COBASE_API std::ostream& sysError( std::ostream& os );

/** 
 * Print the current call stack.
 *
 * May not be implemented on all platforms.
 * @version 1.0
 */
COBASE_API std::ostream& backtrace( std::ostream& os );

/** Print the RTTI name of the given class. @version 1.0 */
template< class T > inline std::string className( T* object )
{
#ifdef _WIN32
    return std::string( typeid( *object ).name( ));
#else
    int status;
    const char* mangled = typeid( *object ).name();
    char* name = abi::__cxa_demangle( mangled, 0, 0, &status );
    std::string result = name;
    if( status != 0 )
        result = mangled;
    if( name )
        free( name );
    return result;
#endif
}

}
}

#ifdef NDEBUG

#  ifdef EQ_RELEASE_ASSERT
#    define EQASSERT(x)                                                 \
    {                                                                   \
        if( !(x) )                                                      \
            EQERROR << "##### Assert: " << #x << " #####" << std::endl  \
                    << co::base::forceFlush;                            \
        co::base::checkHeap();                                          \
    }
#    define EQASSERTINFO(x, info)                                       \
    {                                                                   \
        if( !(x) )                                                      \
            EQERROR << "##### Assert: " << #x << " [" << info << "] #####" \
                    << std::endl << co::base::forceFlush;               \
        co::base::checkHeap();                                          \
    }
#    define EQCHECK(x) { const bool eqOk = x; EQASSERTINFO( eqOk, #x ) }
#  else
#    define EQASSERT(x)
#    define EQASSERTINFO(x, info)
#    define EQCHECK(x) { x; }
#  endif

#  define EQUNIMPLEMENTED { EQERROR << "Unimplemented code" << std::endl \
                                    << co::base::forceFlush; }
#  define EQUNREACHABLE   { EQERROR << "Unreachable code" << std::endl  \
                                    << co::base::forceFlush; }
#  define EQDONTCALL                                                    \
    { EQERROR << "Code is not supposed to be called in this context"    \
              << std::endl << co::base::forceFlush; }
#  define EQABORT( info ) {                                         \
        EQERROR << "##### Abort: " << info << " #####" << std::endl \
                << co::base::forceFlush; }

#else // NDEBUG

#  define EQASSERT(x)                                                   \
    {                                                                   \
        if( !(x) )                                                      \
        {                                                               \
            EQERROR << "Assert: " << #x << " ";                         \
            co::base::abort();                                          \
        }                                                               \
        co::base::checkHeap();                                          \
    } 
#  define EQASSERTINFO(x, info)                                         \
    {                                                                   \
        if( !(x) )                                                      \
        {                                                               \
            EQERROR << "Assert: " << #x << " [" << info << "] ";        \
            co::base::abort();                                          \
        }                                                               \
        co::base::checkHeap();                                          \
    }

#  define EQUNIMPLEMENTED                                               \
    { EQERROR << "Unimplemented code in " << co::base::className( this ) \
              << " ";                                                   \
        co::base::abort(); }
#  define EQUNREACHABLE                                                 \
    { EQERROR << "Unreachable code in " << co::base::className( this )  \
              << " ";                                                   \
        co::base::abort(); }
#  define EQDONTCALL                                                    \
    { EQERROR << "Code is not supposed to be called in this context, type " \
              << co::base::className( this ) << " " ;                   \
        co::base::abort(); }

#  define EQCHECK(x) { const bool eqOk = x; EQASSERTINFO( eqOk, #x ) }
#  define EQABORT( info ) {                                             \
        EQERROR << "Abort: " << info << " ";                            \
        co::base::abort(); }

#endif // NDEBUG

#define EQSAFECAST( to, in ) static_cast< to >( in );   \
    EQASSERT( in == 0 || dynamic_cast< to >( static_cast< to >( in )))


#endif //COBASE_DEBUG_H
