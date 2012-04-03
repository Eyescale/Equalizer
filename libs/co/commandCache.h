
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <lunchbox/thread.h> // thread-safety checks

namespace co
{
namespace detail { class CommandCache; }
    
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
        detail::CommandCache* const _impl;
        friend std::ostream& operator << ( std::ostream&, const CommandCache& );
        LB_TS_VAR( _thread );
    };

    std::ostream& operator << ( std::ostream&, const CommandCache& );
}

#endif //CO_COMMANDCACHE_H
