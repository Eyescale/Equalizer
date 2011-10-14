
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_COMMANDQUEUE_H
#define EQ_COMMANDQUEUE_H

#include <eq/client/types.h>
#include <eq/client/windowSystem.h> // enum
#include <co/commandQueue.h>    // base class

namespace eq
{
    /**
     * @internal
     * Augments an co::CommandQueue to pump system-specific events where
     * required by the underlying window/operating system.
     */
    class CommandQueue : public co::CommandQueue
    {
    public:
        CommandQueue();
        virtual ~CommandQueue();

        /** @sa co::CommandQueue::push(). */
        virtual void push( co::Command& packet );

        /** @sa co::CommandQueue::pushFront(). */
        virtual void pushFront( co::Command& packet );

        /** @sa co::CommandQueue::wakeup(). */
        virtual void wakeup();

        /** @sa co::CommandQueue::pop(). */
        virtual co::Command* pop();

        /** @sa co::CommandQueue::tryPop(). */
        virtual co::Command* tryPop();

        /** @sa reset the time spent in pop() and return the previous value. */
        int64_t resetWaitTime()
            { const int64_t time = _waitTime; _waitTime = 0; return time; }

        void setMessagePump( MessagePump* pump ) { _messagePump = pump; }
        MessagePump* getMessagePump() { return _messagePump; }

    private:
        MessagePump* _messagePump;

        /** The time spent waiting in pop(). */
        int64_t _waitTime;
    };
}

#endif //EQ_COMMANDQUEUE_H
