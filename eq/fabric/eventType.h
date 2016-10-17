
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#ifndef EQFABRIC_EVENTTYPE_H
#define EQFABRIC_EVENTTYPE_H

#include <eq/fabric/api.h>
#include <lunchbox/types.h>

namespace eq
{
namespace fabric
{
namespace eventTypes
{
/** The type of an event. */
enum EventType // Also update string table in event.cpp
{
    // SizeEvent
    EVENT_WINDOW_RESIZE,        //!< A window has been resized
    EVENT_WINDOW_SHOW,          //!< A window is shown
    EVENT_CHANNEL_RESIZE,       //!< A channel has been resized
    EVENT_VIEW_RESIZE,          //!< A view has been resized

    // PointerEvent
    EVENT_CHANNEL_POINTER_MOTION = 10, //!< A pointer is moved over a Channel
    EVENT_CHANNEL_POINTER_BUTTON_PRESS, //!< Mouse button pressed in a Channel
    EVENT_CHANNEL_POINTER_BUTTON_RELEASE, //!< Mouse button release in a Channel
    EVENT_CHANNEL_POINTER_WHEEL, //!< Mouse wheel scroll over a Channel
    EVENT_WINDOW_POINTER_WHEEL, //!< Mouse wheel scroll over a Window
    EVENT_WINDOW_POINTER_MOTION, //!< A pointer is moved over a Window
    EVENT_WINDOW_POINTER_BUTTON_PRESS, //!< Mouse button is pressed in a Window
    EVENT_WINDOW_POINTER_BUTTON_RELEASE, //!< Mouse button release in a Window

    // KeyEvent
    EVENT_KEY_PRESS = 20, //!< Key pressed
    EVENT_KEY_RELEASE, //!< Key released

    EVENT_MAGELLAN_AXIS = 30,        //!< AxisEvent: SpaceMouse touched
    EVENT_MAGELLAN_BUTTON,      //!< ButtonEvent: SpaceMouse button pressed

    // Stateless Events
    EVENT_WINDOW_CLOSE = 40,         //!< A window has been closed
    EVENT_WINDOW_HIDE,          //!< A window is hidden
    EVENT_WINDOW_EXPOSE,    //!< A window is dirty
    EVENT_EXIT, //!< Exit request from application or due to runtime error

    EVENT_STATISTIC, //!< Statistic event

    /** Window pointer grabbed by system window */
    EVENT_WINDOW_POINTER_GRAB,
    /** Window pointer to be released by system window */
    EVENT_WINDOW_POINTER_UNGRAB,


    /**
     * Observer moved (head tracking update). Contains observer originator
     * identifier and 4x4 float tracking matrix.
     */
    EVENT_OBSERVER_MOTION,

    /**
     * Config error event. Contains the originator id, the error code and
     * 0-n Strings with additional information.
     */
    EVENT_CONFIG_ERROR = 50,
    EVENT_NODE_ERROR, //!< Node error event. @sa CONFIG_ERROR
    EVENT_PIPE_ERROR, //!< Pipe error event. @sa CONFIG_ERROR
    EVENT_WINDOW_ERROR, //!< Window error event. @sa CONFIG_ERROR
    EVENT_CHANNEL_ERROR, //!< Channel error event. @sa CONFIG_ERROR

    // todo
    EVENT_NODE_TIMEOUT,         //!< Node has timed out
    EVENT_WINDOW_SCREENSAVER,   //!< A window screensaver request (Win32 only)

    EVENT_UNKNOWN, //!< Event type not known by the event handler

    /** User-defined events have to be of this type or higher */
    EVENT_USER,
    EVENT_ALL // must be last
};

/** Print the event type to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const EventType& );

}
}
}

#endif // EQFABRIC_EVENTTYPE_H
