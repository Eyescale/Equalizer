
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "uint128_t.h"
#include "debug.h" 

#include <cstdlib>      // for strtoull

namespace co
{
namespace base
{
/** Special identifier values */
const uint128_t uint128_t::ZERO;

uint128_t& uint128_t::operator = ( const std::string& from )
{
    char* next = 0;
#ifdef _MSC_VER
    _high = ::_strtoui64( from.c_str(), &next, 16 );
#else
    _high = ::strtoull( from.c_str(), &next, 16 );
#endif
    EQASSERT( next != from.c_str( ));
    if( *next == '\0' ) // short representation, high was 0
    {
        _low = _high;
        _high = 0;
    }
    else
    {
        EQASSERTINFO( *next == ':', from << ", " << next );
        ++next;
#ifdef _MSC_VER
        _low = ::_strtoui64( next, 0, 16 );
#else
        _low = ::strtoull( next, 0, 16 );
#endif
    }
    return *this;
}

}
}
