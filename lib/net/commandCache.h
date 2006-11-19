
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDCACHE_H
#define EQNET_COMMANDCACHE_H

#include <list>

namespace eqNet
{
    class Command;
    class Node;
    
    /**
     * A CommandCache handles the creation of command for the CommandQueue and
     * the node reschedule queue.
     *
     * The packets are copied and can therefore be retained in the queues.
     */
    class CommandCache
    {
    public:
        CommandCache();
        ~CommandCache();

        /** 
         * Create a new command.
         *
         * @param node the node sending the packet.
         * @param packet the command packet.
         * @return the command.
         */
        Command *alloc( Command& packet );

        /** 
         * Release a command.
         *
         * @param command the command.
         */
        void release( Command* command );

    private:
        /** The free command cache. */
        std::list<Command*>       _freeCommands;
    };
};

#endif //EQNET_COMMANDCACHE_H
