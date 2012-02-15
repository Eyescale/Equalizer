
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "debug.h"

#include "os.h" // must come before atomic.h, ordering issue
#include "atomic.h"

#include <errno.h>

#ifndef _WIN32
#  include <cxxabi.h>
#  include <execinfo.h>
#  include <stdlib.h>
#  include <string.h>
#  define EQ_BACKTRACE_DEPTH 256
#endif

namespace co
{
namespace base
{

void abort()
{
    EQERROR << ", in: " << backtrace << std::endl;

    // if EQ_ABORT_WAIT is set, spin forever to allow identifying and debugging
    // crashed nodes.
    if( getenv( "EQ_ABORT_WAIT" ))
        while( true ) ;

    ::abort();
}

void checkHeap()
{
#ifdef _MSC_VER
    static a_int32_t count( 0 );
    if( ( ++count % 10000 ) == 0 && _heapchk() != _HEAPOK )
    {
        EQERROR << disableFlush << "Abort: heap corruption detected"<< std::endl
                << "    Set breakpoint in " << __FILE__ << ':' << __LINE__ + 1 
                << " to debug" << std::endl << enableFlush;
    }
#else
#endif
}

std::ostream& backtrace( std::ostream& os )
{
#ifdef _WIN32
    os << "backtrace not implemented";
#else
    void* callstack[ EQ_BACKTRACE_DEPTH ];
    const int frames = ::backtrace( callstack, EQ_BACKTRACE_DEPTH );
    char** names = ::backtrace_symbols( callstack, frames );
    os << disableFlush << disableHeader << indent << std::endl;
    for( int i = 1; i < frames; ++i )
    {
        std::string name = names[ i ];
#  ifdef Linux
        const size_t symbolPos = name.find( "(_" );
#  else
        const size_t symbolPos = name.find( " _" );
#  endif
        if( symbolPos != std::string::npos )
            name = name.substr( symbolPos+1, name.length( ));

#  ifdef Linux
        const size_t spacePos = name.find( '+' );
#  else
        const size_t spacePos = name.find( ' ' );
#  endif
        if( spacePos != std::string::npos )
            name = name.substr( 0, spacePos );

        int status;
        char* demangled = abi::__cxa_demangle( name.c_str(), 0, 0, &status);

        if( symbolPos == std::string::npos || spacePos == std::string::npos )
            os << names[ i ] << std::endl;
        else
        {
            if( demangled )
            {
                os << demangled << std::endl;
                free( demangled );
            }
            else
                os << name << std::endl;
        }
    }
    os << exdent << enableHeader << enableFlush;
    ::free( names );
#endif

    return os;
}

std::string demangleTypeID( const char* mangled )
{
#ifdef _WIN32
    return std::string( mangled );
#else
    int status;
    char* name = abi::__cxa_demangle( mangled, 0, 0, &status );
    const std::string result = name;
    if( name )
        free( name );

    return (status==0) ? result : mangled;
#endif
}

std::ostream& sysError( std::ostream& os )
{
#ifdef _WIN32
    const DWORD error = GetLastError();
    char text[512] = "";
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, text, 511, 0 );
    const size_t length = strlen( text );
    if( length>2 && text[length-2] == '\r' )
        text[length-2] = '\0';

    os << text << " (" << error << ")";
#else
    os << strerror( errno ) << " (" << errno << ")";
#endif

    return os;
}

}
}
