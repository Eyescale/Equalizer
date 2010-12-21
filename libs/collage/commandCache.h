
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_COMMANDCACHE_H
#define CO_COMMANDCACHE_H

#include <co/types.h>

#include <co/api.h>
#include <co/base/thread.h> // thread-safety checks

#include <vector>

namespace co
{
    class Command;
    
    /**
     * A command cache handles the reuse of allocated packets for a node.
     *
     * Commands are retained and released whenever they are not directly
     * processed, e.g., when pushed to another thread using a CommandQueue.
     */
    class CommandCache
    {
    public:
        CO_API CommandCache();
        CO_API ~CommandCache();

        /** @return a new command. */
        CO_API Command& alloc( NodePtr node, LocalNodePtr localNode,
                                  const uint64_t size );

        /** @return a clone of a command. */
        CO_API Command& clone( Command& from );

        /** Flush all allocated commands. */
        void flush();

    private:
        enum Cache
        {
            CACHE_SMALL,
            CACHE_BIG,
            CACHE_ALL
        };

        /** The caches. */
        Commands _cache[ CACHE_ALL ];

        /** The total size of each cache */
        size_t _size[ CACHE_ALL ];

        /** Last lookup position in each cache. */
        size_t _position[ CACHE_ALL ];

        void _compact( const Cache which );
        Command& _newCommand( const Cache which );

        EQ_TS_VAR( _thread );
    };
}

#endif //CO_COMMANDCACHE_H
