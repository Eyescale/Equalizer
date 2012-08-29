
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQFABRIC_EYE_H
#define EQFABRIC_EYE_H

#include <eq/fabric/api.h>
#include <iostream>

namespace eq
{
namespace fabric
{
    /**
     * Eye pass bit mask for which is enabled.
     */
    enum Eye
    {
        EYE_UNDEFINED  = 0,
        EYE_CYCLOP     = 1, //!<  monoscopic 'middle' eye
        EYE_LEFT       = 2, //!< left eye
        EYE_RIGHT      = 4, //!< right eye
        EYE_LAST       = EYE_RIGHT, //!< the last eye
        NUM_EYES       = 3,  //!< @internal increase with each new enum
        EYES_STEREO    = EYE_LEFT | EYE_RIGHT, //!< left and right eye
        EYES_ALL       = 7 //!< all eyes
    };

    EQFABRIC_API std::ostream& operator << ( std::ostream& os, const Eye& eye );
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Eye& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}

#endif // EQFABRIC_EYE_H
