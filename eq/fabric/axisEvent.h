
/* Copyright (c) 2016-2017, Stefan.Eilemann@epfl.ch
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

#ifndef EQFABRIC_AXISEVENT_H
#define EQFABRIC_AXISEVENT_H

#include <eq/fabric/event.h>

namespace eq
{
namespace fabric
{
/** Event for a (SpaceMouse) axis movement. */
struct AxisEvent : public Event
{
    AxisEvent() : xAxis(0), yAxis(0), zAxis(0),
                  xRotation(0), yRotation(0), zRotation(0) {}

    int32_t xAxis;         //!< X translation
    int32_t yAxis;         //!< Y translation
    int32_t zAxis;         //!< Z translation
    int32_t xRotation;     //!< X rotation
    int32_t yRotation;     //!< Y rotation
    int32_t zRotation;     //!< Z rotation
};

/** Print the axis event to the given output stream. @version 1.0 */
inline std::ostream& operator << ( std::ostream& os, const AxisEvent& event )
{
    return os << static_cast< const Event& >( event ) << " translation "
              << event.xAxis << ", " << event.yAxis << ", " << event.zAxis
              << " rotation " << event.xRotation << ", " << event.yRotation
              << ", " << event.zRotation;
}
}
}

#endif // EQFABRIC_EVENT_H
