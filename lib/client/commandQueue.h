
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

#ifndef EQ_COMMANDQUEUE_H
#define EQ_COMMANDQUEUE_H

#include <eq/net/commandQueue.h>    // base class
#include <eq/client/windowSystem.h> // enum

namespace eq
{
    class MessagePump;

    /**
     * Augments an net::CommandQueue to pump system-specific events where
     * required by the underlying window/operating system.
     * @internal
     */
    class CommandQueue : public net::CommandQueue
    {
    public:
        CommandQueue();
        virtual ~CommandQueue();

        /** @sa net::CommandQueue::push(). */
        virtual void push( net::Command& packet );

        /** @sa net::CommandQueue::pushFront(). */
        virtual void pushFront( net::Command& packet );

        /** @sa net::CommandQueue::wakeup(). */
        virtual void wakeup();

        /** @sa net::CommandQueue::pop(). */
        virtual net::Command* pop();

        /** @sa net::CommandQueue::tryPop(). */
        virtual net::Command* tryPop();

        /** @sa net::CommandQueue::flush(). */
        virtual void flush();

        void setMessagePump( MessagePump* pump ) { _messagePump = pump; }
        MessagePump* getMessagePump() { return _messagePump; }

    private:
        MessagePump* _messagePump;
    };
}

#endif //EQ_COMMANDQUEUE_H
