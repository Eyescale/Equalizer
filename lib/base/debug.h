
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

#ifndef EQBASE_DEBUG_H
#define EQBASE_DEBUG_H

#include <eq/base/defines.h>
#include <eq/base/log.h>
#include <typeinfo>

#ifndef WIN32
#  include <cxxabi.h>
#endif

// assertions
// #define EQ_RELEASE_ASSERT

namespace eq
{
namespace base
{
/**
 * Used to trap into an infinite loop to allow debugging of assertions
 * @internal
 */
EQ_EXPORT void abort();

/**
 * Check the consistency of the heap and abort on error (Win32 only).
 * @internal
 */
EQ_EXPORT void checkHeap();

/** 
 * Print a textual description of the current system error.
 *
 * The current system error is OS-specific, e.g., errno or GetLastError().
 * @version 1.0
 */
EQ_EXPORT std::ostream& sysError( std::ostream& os );

/** 
 * Print the current call stack.
 *
 * May not be implemented on all platforms.
 * @version 1.0
 */
EQ_EXPORT std::ostream& backtrace( std::ostream& os );

/** Print the RTTI name of the given class. @version 1.0 */
template< class T > inline std::string className( T* object )
{
#ifdef WIN32
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
                    << eq::base::forceFlush;                            \
        eq::base::checkHeap();                                          \
    }
#    define EQASSERTINFO(x, info)                                       \
    {                                                                   \
        if( !(x) )                                                      \
            EQERROR << "##### Assert: " << #x << " [" << info << "] #####" \
                    << std::endl << eq::base::forceFlush;               \
        eq::base::checkHeap();                                          \
    }
#    define EQCHECK(x) { const bool eqOk = x; EQASSERTINFO( eqOk, #x ) }
#  else
#    define EQASSERT(x)
#    define EQASSERTINFO(x, info)
#    define EQCHECK(x) { x; }
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

#else // NDEBUG

#  define EQASSERT(x)                                                   \
    {                                                                   \
        if( !(x) )                                                      \
        {                                                               \
            EQERROR << "Assert: " << #x << " ";                         \
            eq::base::abort();                                          \
        }                                                               \
        eq::base::checkHeap();                                          \
    } 
#  define EQASSERTINFO(x, info)                                         \
    {                                                                   \
        if( !(x) )                                                      \
        {                                                               \
            EQERROR << "Assert: " << #x << " [" << info << "] ";        \
            eq::base::abort();                                          \
        }                                                               \
        eq::base::checkHeap();                                          \
    }

#  define EQUNIMPLEMENTED                                               \
    { EQERROR << "Unimplemented code in " << eq::base::className( this ) \
              << " ";                                                   \
        eq::base::abort(); }
#  define EQUNREACHABLE                                                 \
    { EQERROR << "Unreachable code in " << eq::base::className( this )  \
              << " ";                                                   \
        eq::base::abort(); }
#  define EQDONTCALL                                                    \
    { EQERROR << "Code is not supposed to be called in this context, type " \
              << eq::base::className( this ) << " " ;                   \
        eq::base::abort(); }

#  define EQCHECK(x) { const bool eqOk = x; EQASSERTINFO( eqOk, #x ) }
#  define EQABORT( info ) {                                             \
        EQERROR << "Abort: " << " ";                                    \
        eq::base::abort(); }

#endif // NDEBUG

#define EQSAFECAST( to, in ) static_cast< to >( in );   \
    EQASSERT( in == 0 || dynamic_cast< to >( static_cast< to >( in )))


#endif //EQBASE_DEBUG_H
