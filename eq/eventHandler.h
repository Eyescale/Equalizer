
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_EVENTHANDLER_H
#define EQ_EVENTHANDLER_H

#include <eq/fabric/pointerEvent.h>         // member
#include <eq/api.h>
#include <eq/types.h>

namespace eq
{
/** Base class for window system-specific event handlers */
class EventHandler
{
protected:
    /** Construct a new event handler. @version 1.0 */
    EventHandler() : _lastEventType( EVENT_UNKNOWN ) {}

    /** Destruct the event handler. @version 1.0 */
    virtual ~EventHandler() {}

    /**
     * @internal
     * Compute the mouse move delta from the previous pointer event.
     */
    EQ_API void _computePointerDelta( EventType type, PointerEvent& event );

private:
    /** Previous (pointer) event type */
    EventType _lastEventType;

    /** The previous pointer event to compute mouse movement deltas. */
    PointerEvent _lastPointerEvent;
};
}

#endif // EQ_EVENTHANDLER_H
