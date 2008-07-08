
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDQUEUE_H
#define EQNET_COMMANDQUEUE_H

#include <eq/net/commandCache.h>

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
    class EQ_EXPORT CommandQueue : public base::NonCopyable
    {
    public:
        CommandQueue();
        virtual ~CommandQueue();

        /** 
         * Push a command to the queue.
         * 
         * @param packet the command packet.
         */
        virtual void push( Command& packet );

        /** Push a command to the front of the queue. */
        virtual void pushFront( Command& packet );

        /** Wake up the command queue, pop() will return 0. */
        virtual void wakeup() { _commands.push( 0 ); }

        /** 
         * Pop a command from the queue.
         *
         * The returned packet has to be released after usage.
         * 
         * @return the next command in the queue.
         */
        virtual Command* pop();

        /** 
         * Try to pop a command from the queue.
         *
         * The returned packet has to be released after usage.
         * 
         * @return the next command in the queue, or 0 if no command is queued.
         */
        virtual Command* tryPop();

        /** Release a command obtained by pop() or tryPop(). */
        void release( Command* command );

        /** 
         * Peek the command at the end of the queue.
         *
         * @return the last command in the queue, or 0 if no command is queued.
         */
        Command* back() const;

        /** 
         * @return <code>true</code> if the command queue is empty,
         *         <code>false</code> if not. 
         */
        bool empty() const { return _commands.empty(); }

        /** Flush all cached commands. */
        void flush();

        /** @return the size of the queue. */
        size_t size() const { return _commands.size(); }

        CHECK_THREAD_DECLARE( _thread );
    private:
        /** Thread-safe command queue. */
        base::MTQueue< Command >  _commands;
        
        /** The free command cache. */
        CommandCache              _commandCache;
        base::Lock                _commandCacheLock;
    };
}
}

#endif //EQNET_COMMANDQUEUE_H
