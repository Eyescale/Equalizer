
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_MESSAGEPUMP_H
#define EQ_MESSAGEPUMP_H

#include <eq/base/base.h>

namespace eq
{
    /**
     * Defines an interface to process OS messages/events.
     */
    class MessagePump
    {
    public:
        MessagePump() {}
        virtual ~MessagePump() {}

        /** Wake up dispatchOne(). */
        virtual void postWakeup() = 0;

        /** Get and dispatch all pending system events, non-blocking. */
        virtual void dispatchAll() = 0;

        /** Get and dispatch one pending system event, blocking. */
        virtual void dispatchOne() = 0;
        
        /** Clean up, no more dispatch from thread. */
        virtual void dispatchDone(){}

    };
}

#endif //EQ_MESSAGEPUMP_H
