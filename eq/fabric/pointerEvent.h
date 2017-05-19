
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

#ifndef EQFABRIC_POINTEREVENT_H
#define EQFABRIC_POINTEREVENT_H

#include <eq/fabric/event.h>
#include <eq/fabric/renderContext.h> // member

namespace eq
{
namespace fabric
{
/** Event for a pointer (mouse) motion or click. */
struct PointerEvent : public Event
{
    PointerEvent()
        : x(0)
        , y(0)
        , dx(0)
        , dy(0)
        , buttons(0)
        , button(0)
        , xAxis(0)
        , yAxis(0)
    {
    }

    int32_t x;             //!< X position relative to entity
    int32_t y;             //!< Y position relative to entity (0 is on top)
    int32_t dx;            //!< X position change since last event
    int32_t dy;            //!< Y position change since last event
    uint32_t buttons;      //!< current state of all buttons
    uint32_t button;       //!< fired button
    KeyModifier modifiers; //!< state of modifier keys
    float xAxis;           //!< x wheel rotation in clicks
    float yAxis;           //!< y wheel rotation in clicks
    RenderContext context; //!< The last rendering context at position
};

/** Print the pointer event to the given output stream. @version 1.0 */
inline std::ostream& operator<<(std::ostream& os, const PointerEvent& event)
{
    os << static_cast<const Event&>(event) << " [" << event.x << "], ["
       << event.y << "] d(" << event.dx << ", " << event.dy << ')' << " wheel "
       << '[' << event.xAxis << ", " << event.yAxis << "] buttons ";

    if (event.buttons == PTR_BUTTON_NONE)
        os << "none";
    if (event.buttons & PTR_BUTTON1)
        os << "1";
    if (event.buttons & PTR_BUTTON2)
        os << "2";
    if (event.buttons & PTR_BUTTON3)
        os << "3";
    if (event.buttons & PTR_BUTTON4)
        os << "4";
    if (event.buttons & PTR_BUTTON5)
        os << "5";

    os << event.modifiers << " fired ";
    if (event.button == PTR_BUTTON_NONE)
        os << "none";
    if (event.button & PTR_BUTTON1)
        os << "1";
    if (event.button & PTR_BUTTON2)
        os << "2";
    if (event.button & PTR_BUTTON3)
        os << "3";
    if (event.button & PTR_BUTTON4)
        os << "4";
    if (event.button & PTR_BUTTON5)
        os << "5";

    return os << " context " << event.context;
}
}
}

#endif // EQFABRIC_EVENT_H
