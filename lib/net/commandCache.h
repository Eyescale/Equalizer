
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQNET_COMMANDCACHE_H
#define EQNET_COMMANDCACHE_H

#include <eq/base/base.h>
#include <vector>

namespace eq
{
namespace net
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
}

#endif //EQNET_COMMANDCACHE_H
