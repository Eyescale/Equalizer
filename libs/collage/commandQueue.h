
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <co/base/lock.h>
#include <co/base/mtQueue.h>
#include <co/base/nonCopyable.h>
#include <co/base/thread.h>

namespace co
{
    class Command;

    /**
     * A CommandQueue is a thread-safe queue for command packets.
     */
    class CommandQueue : public co::base::NonCopyable
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
        virtual void wakeup() { _commands.push( static_cast< Command* >( 0 )); }

        /** 
         * Pop a command from the queue.
         *
         * The returned command has to be released after usage.
         * 
         * @return the next command in the queue.
         */
        CO_API virtual Command* pop();

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
        bool isEmpty() const { return _commands.isEmpty(); }

        /** Flush all cached commands. */
        CO_API virtual void flush();

        /** @return the size of the queue. */
        size_t getSize() const { return _commands.getSize(); }

        EQ_TS_VAR( _thread );
    private:
        /** Thread-safe command queue. */
        co::base::MTQueue< Command* >  _commands;
    };
}

#endif //CO_COMMANDQUEUE_H
