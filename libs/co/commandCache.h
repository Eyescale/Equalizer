
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <co/base/atomic.h> // member
#include <co/base/thread.h> // thread-safety checks

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

        typedef std::vector< Command* > Data;
        typedef Data::const_iterator DataCIter;

        /** The caches. */
        Data _cache[ CACHE_ALL ];

        /** Last lookup position in each cache. */
        DataCIter _position[ CACHE_ALL ];

        /** The current number of free items in each cache */
        base::a_int32_t _free[ CACHE_ALL ];

        /** The maximum number of free items in each cache */
        int32_t _maxFree[ CACHE_ALL ];

        void _compact( const Cache which );
        Command& _newCommand( const Cache which );

        friend std::ostream& operator << ( std::ostream&, const CommandCache& );
        EQ_TS_VAR( _thread );
    };

    std::ostream& operator << ( std::ostream&, const CommandCache& );
}

#endif //CO_COMMANDCACHE_H
