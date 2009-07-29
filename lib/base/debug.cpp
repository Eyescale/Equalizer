
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

#ifndef NDEBUG
namespace eq
{
namespace base
{

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
    if( _heapchk() != _HEAPOK )
    {
        EQERROR << disableFlush << "Abort: heap corruption detected" << std::endl
                << "    Set breakpoint in " << __FILE__ << ':' << __LINE__ + 1 
                << " to debug" << std::endl << enableFlush;
    }
#else
#endif
}

}
}
#endif
