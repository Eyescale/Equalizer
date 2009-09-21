
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "atomic.h"

#include <errno.h>

namespace eq
{
namespace base
{

#ifndef NDEBUG
EQ_EXPORT void abort()
{
    // if EQ_ABORT_WAIT is set, spin forever to allow identifying and debugging
    // crashed nodes.
    if( getenv( "EQ_ABORT_WAIT" ))
        while( true ) ;

    ::abort();
}

EQ_EXPORT void checkHeap()
{
#ifdef WIN32
    static mtLong count( 0 );
    if( ( ++count % 1000 ) == 0 && _heapchk() != _HEAPOK )
    {
        EQERROR << disableFlush << "Abort: heap corruption detected"<< std::endl
                << "    Set breakpoint in " << __FILE__ << ':' << __LINE__ + 1 
                << " to debug" << std::endl << enableFlush;
    }
#else
#endif
}
#endif // DEBUG

EQ_EXPORT std::ostream& sysError( std::ostream& os )
{
#ifdef WIN32
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
