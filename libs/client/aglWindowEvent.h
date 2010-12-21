
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_AGLWINDOWEVENT_H
#define EQ_AGLWINDOWEVENT_H

#include <eq/event.h> // base class

namespace eq
{
    /** A window-system event with the native Carbon event, used for AGL. */
    class AGLWindowEvent : public Event
    {
    public:
        /** The native event. @version 1.0 */
        EventRef carbonEventRef;
    };
}

#endif // EQ_AGLWINDOWEVENT_H

