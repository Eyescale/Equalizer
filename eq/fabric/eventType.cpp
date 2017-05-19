
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

#include "eventType.h"
#include "types.h"
#include <iostream>

namespace eq
{
namespace fabric
{
namespace
{
/** String representation of event types. */
class EventTypeNames
{
public:
    EventTypeNames()
    {
        _names[EVENT_WINDOW_EXPOSE] = "window expose";
        _names[EVENT_WINDOW_RESIZE] = "window resize";
        _names[EVENT_WINDOW_CLOSE] = "window close";
        _names[EVENT_WINDOW_HIDE] = "window show";
        _names[EVENT_WINDOW_SHOW] = "window hide";
        _names[EVENT_WINDOW_SCREENSAVER] = "window screensaver";
        _names[EVENT_CHANNEL_POINTER_MOTION] = "pointer motion";
        _names[EVENT_CHANNEL_POINTER_BUTTON_PRESS] = "pointer button press";
        _names[EVENT_CHANNEL_POINTER_BUTTON_RELEASE] = "pointer button release";
        _names[EVENT_CHANNEL_POINTER_WHEEL] = "pointer wheel";
        _names[EVENT_WINDOW_POINTER_WHEEL] = "pointer wheel";
        _names[EVENT_WINDOW_POINTER_MOTION] = "window pointer motion";
        _names[EVENT_WINDOW_POINTER_BUTTON_PRESS] =
            "window pointer button press";
        _names[EVENT_WINDOW_POINTER_BUTTON_RELEASE] =
            "window pointer button release";
        _names[EVENT_KEY_PRESS] = "key press";
        _names[EVENT_KEY_RELEASE] = "key release";
        _names[EVENT_CHANNEL_RESIZE] = "channel resize";
        _names[EVENT_STATISTIC] = "statistic";
        _names[EVENT_VIEW_RESIZE] = "view resize";
        _names[EVENT_EXIT] = "exit";
        _names[EVENT_MAGELLAN_AXIS] = "magellan axis";
        _names[EVENT_MAGELLAN_BUTTON] = "magellan button";
        _names[EVENT_NODE_TIMEOUT] = "node timed out";
        _names[EVENT_OBSERVER_MOTION] = "observer motion";
        _names[EVENT_UNKNOWN] = "unknown";
        _names[EVENT_USER] = "user-specific";
    }

    const std::string& operator[](const EventType type) const
    {
        return _names[type];
    }

private:
    std::string _names[EVENT_ALL];
};
static EventTypeNames _eventTypeNames;
}

namespace eventTypes
{
std::ostream& operator<<(std::ostream& os, const EventType& type)
{
    if (type >= EVENT_ALL)
        return os << "user event (" << unsigned(type) << ')';
    return os << _eventTypeNames[type] << " (" << unsigned(type) << ')';
}
}
}
}
