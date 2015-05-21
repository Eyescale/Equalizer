
/* Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_AGL_WINDOWEVENT_H
#define EQ_AGL_WINDOWEVENT_H

#include <eq/defines.h>
#ifdef AGL

#include <eq/fabric/event.h> // base class
#include <eq/types.h>

namespace eq
{
namespace agl
{
    /** A window-system event with the native Carbon event, used for AGL. */
    class WindowEvent : public Event
    {
    public:
        /** The native event. @version 1.0 */
        EventRef carbonEventRef;
    };
}
}
#endif // AGL
#endif // EQ_AGL_WINDOWEVENT_H
