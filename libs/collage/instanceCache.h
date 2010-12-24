
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_INSTANCECACHE_H
#define CO_INSTANCECACHE_H

#include <co/api.h>
#include <co/types.h>

#include <co/base/lockable.h>  // member
#include <co/base/stdExt.h>    // member
#include <co/base/thread.h>    // member
#include <co/base/uuid.h>      // member

#include <iostream>

namespace co
{
    /** @internal A thread-safe cache for object instance data. */
    class InstanceCache
    {
    public:
        /** Construct a new instance cache. */
        CO_API InstanceCache( const uint64_t maxSize = EQ_100MB );

        /** Destruct this instance cache. */
        CO_API ~InstanceCache();

        /** 
         * Add a new command to the instance cache.
         *
         * @param rev the object identifier and version.
         * @param instanceID the master instance ID.
         * @param command The command to add.
         * @param usage pre-set usage count.
         * @return true if the command was entered, false if not.
         */
        CO_API bool add( const ObjectVersion& rev, const uint32_t instanceID, 
                  Command& command, const uint32_t usage = 0 );

        /** One cache entry */
        struct Data
        {
            Data();
            CO_API bool operator != ( const Data& rhs ) const;
            CO_API bool operator == ( const Data& rhs ) const;

            uint32_t masterInstanceID; //!< The instance ID of the master object
            ObjectDataIStreamDeque versions; //!< all cached data
            CO_API static const Data NONE; //!< '0' return value 
        };

        /**
         * Direct access to the cached instance data for the given object id.
         *
         * The instance data for the given object has to be released by the
         * caller, unless 0 has been returned. Not all returned data stream
         * might be ready.
         *
         * @param id the identifier of the object to look up.
         * @return the list of cached instance datas, or Data::NONE if no data
         *         is cached for this object.
         */

        CO_API const Data& operator[]( const co::base::UUID& id );

        /** 
         * Release the retrieved instance data of the given object.
         *
         * @param id the identifier of the object to release.
         * @param count the number of access operations to release
         * @return true if the element was unpinned, false if it is not in the
         *         instance cache.
         */
        CO_API bool release( const co::base::UUID& id, const uint32_t count = 1 );

        /** 
         * Erase all the data for the given object.
         *
         * The data does not have to be accessed, i.e., release has been called
         * for each previous access.
         *
         * @return true if the element was erased, false otherwise.
         */
        CO_API bool erase( const co::base::UUID& id );

        /** @return the number of bytes used by the instance cache. */
        uint64_t getSize() const { return _size; }

        /** @return the maximum number of bytes used by the instance cache. */
        uint64_t getMaxSize() const { return _maxSize; }

        /** Remove all items which are older than the given time. */
        void expire( const int64_t age );

        bool empty( ){ return _items->empty(); }
    private:
        struct Item
        {
            Item();
            Data data;
            int64_t time;
            unsigned used;
            unsigned access;
        };

        typedef stde::hash_map< co::base::uint128_t, Item > ItemHash;

        co::base::Lockable< ItemHash > _items;

        const uint64_t _maxSize; //!<high-water mark to start releasing commands
        uint64_t _size;          //!< Current number of bytes stored

        const co::base::Clock _clock;  //!< Clock for item expiration

        void _releaseItems( const uint32_t minUsage );
        void _releaseStreams( InstanceCache::Item& item );
        void _releaseFirstStream( InstanceCache::Item& item );

        EQ_TS_VAR( _thread );
    };

    CO_API std::ostream& operator << ( std::ostream&, const InstanceCache& );
}
#endif //CO_INSTANCECACHE_H

