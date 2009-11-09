
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

#include <eq/net/types.h>

#include <eq/base/stdExt.h>    // member
#include <eq/base/thread.h>    // member

#include <iostream>

namespace eq
{
namespace net
{
    class Command;
    class DataIStream;
    class ObjectDataIStream;
    struct ObjectVersion;

    /**
     * A thread-safe cache for object instance data. @internal
     */
    class InstanceCache
    {
    public:
        /** Construct a new instance cache. */
        InstanceCache( const long maxSize = EQ_100MB );

        /** Destruct this instance cache. */
        ~InstanceCache();

        /** 
         * Add a new command to the instance cache.
         *
         * @param rev the object identifier and version.
         * @param command The command to add.
         * @param usage pre-set usage count.
         * @return true if the command was entered, false if not.
         */
        bool add( const ObjectVersion& rev, Command& command, 
                  const uint32_t usage = 0 );

        /**
         * Direct access to the cached instance data for the given object id.
         *
         * The instance data for the given object has to be released by the
         * caller, unless 0 has been returned. Not all returned data stream
         * might be ready.
         *
         * @param id the identifier of the object to look up.
         * @return the list of cached instance datas, or 0 if no data is cached
         *         for this object.
         */
        const InstanceDataDeque* operator[]( const uint32_t id );

        /** 
         * Release the retrieved instance data of the given object.
         *
         * @param id the identifier of the object to release.
         * @param count the number of access operations to release
         * @return true if the element was unpinned, false if it is not in the
         *         instance cache.
         */
        bool release( const uint32_t id, const uint32_t count = 1 );

        /** 
         * Erase all the data for the given object.
         *
         * The data does not have to be accessed, i.e., release has been called
         * for each previous access.
         *
         * @return true if the element was erased, false otherwise.
         */
        bool erase( const uint32_t id );

        /** @return the number of bytes used by the instance cache. */
        long getSize() const { return _size; }

    private:
        struct Item
        {
            Item();
            InstanceDataDeque streams;
            unsigned used;
            unsigned access;
        };

        typedef stde::hash_map< uint32_t, Item > Data;

        Data _data;
        base::Lock _mutex;

        const long _maxSize; //!< high-water mark to start releasing commands
        long _size;          //!< Current number of bytes stored

        void _releaseItems( const uint32_t minUsage );
        void _releaseStreams( InstanceCache::Item& item );
        void _releaseFirstStream( InstanceCache::Item& item );

        CHECK_THREAD_DECLARE( _thread );
    };

    std::ostream& operator << ( std::ostream&, const InstanceCache& );
}
}
#endif //EQNET_INSTANCECACHE_H

