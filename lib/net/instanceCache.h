
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_INSTANCECACHE_H
#define EQNET_INSTANCECACHE_H

#include <eq/base/atomic.h>    // member
#include <eq/base/rng.h>       // member
#include <eq/base/stdExt.h>    // member
#include <eq/base/thread.h>    // member

#include <iostream>

namespace eq
{
namespace net
{
    class Command;

    /**
     * A thread-safe cache for object instance data. @internal
     */
    template< typename K > class InstanceCache
    {
    public:
        /** Construct a new instance cache. */
        InstanceCache( const long maxSize = EQ_100MB );

        /** Destruct this instance cache. */
        ~InstanceCache();

        /** 
         * Add a new item to the instance cache.
         *
         * @param key The key to find the item.
         * @param command The command to add.
         * @param pin true if the item has to be added and pinned in the instance cache.
         * @return true if the item was entered, false if not.
         */
        bool add( const K& key, Command* const command, const bool pin );

        /**
         * Direct access to the element with the given key.
         *
         * The returned element is automatically pinned, and has to be unpinned
         * by the caller. If the element is not in the instance cache, 0 is returned.
         */
        Command* operator[]( const K& key );

        /** 
         * Lock the element of the given key in the instance cache, if it exist.
         * 
         * @param key The key of the element to pin.
         * @return true if the element was pinned, false if it is not in the
         *         instance cache. 
         */
        bool pin( const K& key );

        /** 
         * Unlock the element of the given key.
         * @return true if the element was unpinned, false if it is not in the
         *         instance cache.
         */
        bool unpin( const K& key );

        /** 
         * Erase the element of the given key.
         *
         * The element has to be unpinned.
         *
         * @return true if the element was erased, false otherwise.
         */
        bool erase( const K& key );

        /** @return the number of bytes used by the instance cache. */
        long getSize() const { return _size; }

    private:
        struct Item
        {
            Item();
            Command* command;
            unsigned used;
            unsigned pinned;
        };

        typedef stde::hash_map< K, Item > Data;

        Data _data;
        base::Lock _mutex;

        const long   _maxSize; //!< high-water mark to start releasing commands
        base::mtLong _size;    //!< Current number of bytes stored

        void _releaseItems();
        CHECK_THREAD_DECLARE( _thread );
    };

    template< typename K >
    std::ostream& operator << ( std::ostream& os,
                                const InstanceCache< K >& instanceCache );
}
}
#endif //EQNET_INSTANCECACHE_H

