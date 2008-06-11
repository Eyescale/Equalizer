
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDCACHE_H
#define EQNET_COMMANDCACHE_H

#include <eq/base/base.h>
#include <vector>

namespace eqNet
{
    class Command;
    
    /**
     * A CommandCache handles the creation of commands for the CommandQueue and
     * the node reschedule queue.
     *
     * The packets are copied and can therefore be retained in the queues.
     */
    class EQ_EXPORT CommandCache
    {
    public:
        CommandCache();
        ~CommandCache();

        /** 
         * Create a new command.
         *
         * @param command the input command.
         * @return the command.
         */
        Command *alloc( Command& command );

        /** 
         * Release a command.
         *
         * @param command the command.
         */
        void release( Command* command );

        /** Flush all released commands. */
        void flush();

        bool empty() const { return _freeCommands.empty(); }
    private:
#pragma warning(push)
#pragma warning(disable: 4251)
        /** The free command cache. */
        std::vector< Command* >       _freeCommands;
#pragma warning(pop)
    };
}

#endif //EQNET_COMMANDCACHE_H
