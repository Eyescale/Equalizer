
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

#ifndef EQFABRIC_SIZEEVENT_H
#define EQFABRIC_SIZEEVENT_H

#include <eq/fabric/event.h>

namespace eq
{
namespace fabric
{
/** Event for a size or position change on a Window, Channel or View. */
struct SizeEvent : public Event
{
    SizeEvent() : x(0), y(0), w(0), h(0), dw(0), dh(0) {}

    int32_t x; //!< new X position, relative to parent
    int32_t y; //!< new Y position, relative to parent
    int32_t w; //!< new width
    int32_t h; //!< new height
    float dw;  //!< view only: new width relative to initial width
    float dh;  //!< view only: new height relative to initial height
};

/** Print the resize event to the given output stream. @version 1.0 */
inline std::ostream& operator << ( std::ostream& os, const SizeEvent& event )
{
    return os << static_cast< const Event& >( event ) << ' ' << event.x << 'x'
              << event.y << '+' << event.w << '+' << event.h;
}
}
}

#endif // EQFABRIC_EVENT_H
