
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

#ifndef EQFABRIC_BUTTONEVENT_H
#define EQFABRIC_BUTTONEVENT_H

#include <eq/fabric/event.h>

namespace eq
{
namespace fabric
{
/** Event for a (SpaceMouse) click. */
struct ButtonEvent : public Event
{
    ButtonEvent()
        : button(0)
        , buttons(0)
    {
    }

    uint32_t button;  //!< fired button
    uint32_t buttons; //!< current state of all buttons
};

/** Print the button event to the given output stream. @version 1.0 */
inline std::ostream& operator<<(std::ostream& os, const ButtonEvent& event)
{
    return os << static_cast<const Event&>(event) << " buttons "
              << event.buttons;
}
}
}

#endif // EQFABRIC_EVENT_H
