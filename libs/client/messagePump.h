
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/api.h>

namespace eq
{
    /** An interface to process system messages or events. */
    class MessagePump
    {
    public:
        /** Construct a new message pump. @version 1.0 */
        MessagePump() {}

        /** Destruct this message pump. @version 1.0 */
        virtual ~MessagePump() {}

        /** Unblock dispatchOne(). @version 1.0 */
        virtual void postWakeup() = 0;

        /** Dispatch all pending system events, does not block. @version 1.0 */
        virtual void dispatchAll() = 0;

        /**
         * Dispatch at least one pending system event, blocks potentially.
         * @version 1.0
         */
        virtual void dispatchOne() = 0;
    };
}

#endif //EQ_MESSAGEPUMP_H
