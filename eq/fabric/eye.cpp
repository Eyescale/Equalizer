
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "eye.h"

#include <lunchbox/bitOperation.h>
#include <lunchbox/debug.h> // for LBABORT

namespace eq
{
namespace fabric
{
std::ostream& operator<<(std::ostream& os, const Eye& eye)
{
    switch (eye)
    {
    case EYE_LEFT:
        os << "left eye";
        break;
    case EYE_RIGHT:
        os << "right eye";
        break;
    case EYE_CYCLOP:
        os << "cyclop eye";
        break;
    case EYE_UNDEFINED:
    default:
        LBABORT("Invalid eye value");
    }

    return os;
}
}
}

namespace lunchbox
{
template <>
EQFABRIC_API int32_t getIndexOfLastBit<eq::fabric::Eye>(eq::fabric::Eye eye)
{
    return getIndexOfLastBit(uint32_t(eye));
}
}
