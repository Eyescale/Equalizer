
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_COMMANDCACHE_H
#define EQNET_COMMANDCACHE_H

#include <eq/net/types.h>

#include <eq/base/base.h>
#include <eq/base/thread.h> // thread-safety checks

#include <vector>

namespace eq
{
namespace net
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
        CommandCache();
        ~CommandCache();

        /** @return a new command. */
        Command& alloc( NodePtr node, NodePtr localNode, const uint64_t size );

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
        CommandVector _caches[ CACHE_ALL ];
        /** Last lookup position in each cache. */
        size_t _positions[ CACHE_ALL ];

        CHECK_THREAD_DECLARE( _thread );
    };
}
}

#endif //EQNET_COMMANDCACHE_H
