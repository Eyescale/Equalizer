
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

#ifndef EQFABRIC_FOCUSMODE_H
#define EQFABRIC_FOCUSMODE_H

#include <eq/fabric/api.h>
#include <eq/fabric/iAttribute.h>

namespace eq
{
namespace fabric
{
    /** The algorithm to use for observer focal distance calculation */
    enum FocusMode
    {
        FOCUSMODE_FIXED      = FIXED, //!< Focus on physical projection
        /** Focus distance relative and in -Z axis of origin. */
        FOCUSMODE_RELATIVE_TO_ORIGIN = RELATIVE_TO_ORIGIN,
        /** Focus distance relative and in direction of observer. */
        FOCUSMODE_RELATIVE_TO_OBSERVER = RELATIVE_TO_OBSERVER,
    };

    inline std::ostream& operator << ( std::ostream& os, const FocusMode& mode )
    {
        os << IAttribute( mode );
        return os;
    }
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::FocusMode& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}
#endif // EQFABRIC_FOCUSMODE_H
