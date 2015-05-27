
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_GLX_WINDOWEVENT_H
#define EQ_GLX_WINDOWEVENT_H

#include <eq/fabric/event.h>
#include <eq/types.h>
#include <eq/gl.h>

namespace eq
{
namespace glx
{
    /** A window-system event for a glx::WindowIF. */
    class EQ_API WindowEvent : public Event
    {
    public:
        /** Native event. @version 1.0 */
        XEvent xEvent;
    };
}
}
#endif // EQ_GLX_WINDOWEVENT_H
