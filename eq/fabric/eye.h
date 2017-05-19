
/* Copyright (c) 2007-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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
#include <lunchbox/bitOperation.h>

namespace eq
{
namespace fabric
{
/** Eye pass bit mask for which rendering is enabled. */
enum Eye
{
    EYE_CYCLOP_BIT = 0, //!< @internal
    EYE_LEFT_BIT = 1,   //!< @internal
    EYE_RIGHT_BIT = 2,  //!< @internal
    EYE_UNDEFINED = 0,
    EYE_CYCLOP = 1 << EYE_CYCLOP_BIT, //!<  monoscopic 'middle' eye
    EYE_LEFT = 1 << EYE_LEFT_BIT,     //!< left eye
    EYE_RIGHT = 1 << EYE_RIGHT_BIT,   //!< right eye
    EYE_LAST = EYE_RIGHT,             //!< the last eye
    NUM_EYES = 3,                     //!< @internal increase with each new enum
    EYES_STEREO = EYE_LEFT | EYE_RIGHT, //!< left and right eye
    EYES_ALL = 7                        //!< all eyes
};

EQFABRIC_API std::ostream& operator<<(std::ostream& os, const Eye& eye);
}
}

#endif // EQFABRIC_EYE_H
