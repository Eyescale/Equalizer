
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

    /** A thread-safe queue for Command packets. */
    class CommandQueue : public lunchbox::NonCopyable
    {
    public:
        /** Construct a new command queue. @version 1.0 */
        CO_API CommandQueue();

        /** Destruct a new command queue. @version 1.0 */
        CO_API virtual ~CommandQueue();

        /** 
         * Push a command to the queue.
         * 
         * @param packet the command packet.
         * @version 1.0
         */
        CO_API virtual void push( CommandPtr packet );

        /** Push a command to the front of the queue. @version 1.0 */
        CO_API virtual void pushFront( CommandPtr packet );

        /** 
         * Pop a command from the queue.
         *
         * @param timeout the time in ms to wait for the operation.
         * @return the next command in the queue.
         * @throw Exception on timeout.
         * @version 1.0
         */
        CO_API virtual CommandPtr pop( const uint32_t timeout =
                                       LB_TIMEOUT_INDEFINITE );

        /** 
         * Try to pop a command from the queue.
         *
         * @return the next command in the queue, or 0 if no command is queued.
         * @version 1.0
         */
        CO_API virtual CommandPtr tryPop();

        /** 
         * @return <code>true</code> if the command queue is empty,
         *         <code>false</code> if not. 
         * @version 1.0
         */
        CO_API bool isEmpty() const;

        /** Flush all pending commands. @version 1.0 */
        CO_API void flush();

        /** @return the size of the queue. @version 1.0 */
        CO_API size_t getSize() const;

        LB_TS_VAR( _thread );

    private:
        detail::CommandQueue* const _impl;
    };
}

#endif //CO_COMMANDQUEUE_H
