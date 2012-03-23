
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_COMMANDQUEUE_H
#define CO_COMMANDQUEUE_H

#include <co/api.h>
#include <co/types.h>
#include <lunchbox/thread.h>

namespace co
{
namespace detail { class CommandQueue; }

    /** A CommandQueue is a thread-safe queue for command packets. */
    class CommandQueue : public lunchbox::NonCopyable
    {
    public:
        CO_API CommandQueue();
        CO_API virtual ~CommandQueue();

        /** 
         * Push a command to the queue.
         * 
         * @param packet the command packet.
         */
        CO_API virtual void push( Command& packet );

        /** Push a command to the front of the queue. */
        CO_API virtual void pushFront( Command& packet );

        /** Wake up the command queue, pop() will return 0. */
        CO_API virtual void wakeup();

        /** 
         * Pop a command from the queue.
         *
         * The returned command has to be released after usage.
         * 
         * @param timeout the time in ms to wait for the operation.
         * @return the next command in the queue.
         * @throws Exception on timeout.
         */
        CO_API virtual Command* pop( const uint32_t timeout =
                                     EQ_TIMEOUT_INDEFINITE );

        /** 
         * Try to pop a command from the queue.
         *
         * The returned command has to be released after usage.
         * 
         * @return the next command in the queue, or 0 if no command is queued.
         */
        CO_API virtual Command* tryPop();

        /** 
         * @return <code>true</code> if the command queue is empty,
         *         <code>false</code> if not. 
         */
        CO_API bool isEmpty() const;

        /** Flush all cached commands. */
        CO_API void flush();

        /** @return the size of the queue. */
        CO_API size_t getSize() const;

        EQ_TS_VAR( _thread );

    private:
        detail::CommandQueue* const _impl;
    };
}

#endif //CO_COMMANDQUEUE_H
