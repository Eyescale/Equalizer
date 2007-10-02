
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDQUEUE_H
#define EQNET_COMMANDQUEUE_H

#include <eq/net/commandCache.h>

#include <eq/base/lock.h>
#include <eq/base/mtQueue.h>
#include <eq/base/thread.h>

namespace eqNet
{
    class Command;

    /**
     * A CommandQueue is a thread-safe queue for command packets.
     */
    class EQ_EXPORT CommandQueue
    {
    public:
        CommandQueue();
        virtual ~CommandQueue();

        /** 
         * Push a command to the queue.
         *
         * The command's command is incremented by one, which enables the usage
         * of the same code for handling the arriving packet and the queued
         * command, as they use different commands.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        virtual void push( Command& packet );

        /** 
         * Push a command to the front of the queue.
         *
         * The command's command is incremented by one, which enables the usage
         * of the same code for handling the arriving packet and the queued
         * command, as they use different commands.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        virtual void pushFront( Command& packet );

        /** 
         * Pop a command from the queue.
         *
         * The returned packet is valid until the next pop operation.
         * 
         * @return the next command in the queue.
         */
        virtual Command* pop();

        /** 
         * Try to pop a command from the queue.
         *
         * The returned packet is valid until the next pop operation.
         * 
         * @return the next command in the queue, or 0 if no command is queued.
         */
        Command* tryPop();

        
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

#pragma warning(push)
#pragma warning(disable: 4251)
        /** Thread-safe command queue. */
        eqBase::MTQueue<Command>  _commands;
#pragma warning(pop)
        
        /** The last popped command, to be released upon the next pop. */
        Command*                  _lastCommand;

        /** The free command cache. */
        CommandCache              _commandCache;
        eqBase::Lock              _commandCacheLock;
    };
}

#endif //EQNET_COMMANDQUEUE_H
