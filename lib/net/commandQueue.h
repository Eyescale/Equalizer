
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

#ifndef EQNET_COMMANDQUEUE_H
#define EQNET_COMMANDQUEUE_H

#include <eq/base/lock.h>
#include <eq/base/mtQueue.h>
#include <eq/base/nonCopyable.h>
#include <eq/base/thread.h>

namespace eq
{
namespace net
{
    class Command;

    /**
     * A CommandQueue is a thread-safe queue for command packets.
     */
    class CommandQueue : public base::NonCopyable
    {
    public:
        EQ_EXPORT CommandQueue();
        EQ_EXPORT virtual ~CommandQueue();

        /** 
         * Push a command to the queue.
         * 
         * @param packet the command packet.
         */
        EQ_EXPORT virtual void push( Command& packet );

        /** Push a command to the front of the queue. */
        EQ_EXPORT virtual void pushFront( Command& packet );

        /** Wake up the command queue, pop() will return 0. */
        virtual void wakeup() { _commands.push( static_cast< Command* >( 0 )); }

        /** 
         * Pop a command from the queue.
         *
         * The returned command has to be released after usage.
         * 
         * @return the next command in the queue.
         */
        EQ_EXPORT virtual Command* pop();

        /** 
         * Try to pop a command from the queue.
         *
         * The returned command has to be released after usage.
         * 
         * @return the next command in the queue, or 0 if no command is queued.
         */
        EQ_EXPORT virtual Command* tryPop();

        /** 
         * @return <code>true</code> if the command queue is empty,
         *         <code>false</code> if not. 
         */
        bool isEmpty() const { return _commands.isEmpty(); }

        /** Flush all cached commands. */
        virtual void flush();

        /** @return the size of the queue. */
        size_t getSize() const { return _commands.getSize(); }

        EQ_TS_VAR( _thread );
    private:
        /** Thread-safe command queue. */
        base::MTQueue< Command* >  _commands;
    };
}
}

#endif //EQNET_COMMANDQUEUE_H
