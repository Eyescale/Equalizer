
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

#include "instanceCache.h"

#include "command.h"
#include "objectDataIStream.h"
#include "objectVersion.h"

#include <co/base/debug.h>

namespace co
{
//#define EQ_INSTRUMENT_CACHE
#ifdef EQ_INSTRUMENT_CACHE
namespace
{
co::base::a_int32_t nRead;
co::base::a_int32_t nReadHit;
co::base::a_int32_t nWrite;
co::base::a_int32_t nWriteHit;
co::base::a_int32_t nWriteMiss;
co::base::a_int32_t nWriteReady;
co::base::a_int32_t nWriteOld;
co::base::a_int32_t nUsedRelease;
co::base::a_int32_t nUnusedRelease;
}
#endif

const InstanceCache::Data InstanceCache::Data::NONE;

InstanceCache::InstanceCache( const uint64_t maxSize )
        : _maxSize( maxSize )
        , _size( 0 )
{}

InstanceCache::~InstanceCache()
{
    for( ItemHash::iterator i = _items->begin(); i != _items->end(); ++i )
    {
        Item& item = i->second;
        _releaseStreams( item );
    }

    _items->clear();
    _size = 0;
}

InstanceCache::Data::Data() 
        : masterInstanceID( EQ_INSTANCE_INVALID )
{}

bool InstanceCache::Data::operator != ( const InstanceCache::Data& rhs ) const
{
    return ( masterInstanceID != rhs.masterInstanceID ||
             versions != rhs.versions );
}

bool InstanceCache::Data::operator == ( const InstanceCache::Data& rhs ) const
{
    return ( masterInstanceID == rhs.masterInstanceID &&
             versions == rhs.versions );
}

InstanceCache::Item::Item()
        : time( 0 )
        , used( 0 )
        , access( 0 )
{}

bool InstanceCache::add( const ObjectVersion& rev, const uint32_t instanceID,
                         Command& command, const uint32_t usage )
{
#ifdef EQ_INSTRUMENT_CACHE
    ++nWrite;
#endif

    co::base::ScopedMutex<> mutex( _items );
    ItemHash::const_iterator i = _items->find( rev.identifier );
    if( i == _items->end( ))
    {
        Item& item = _items.data[ rev.identifier ];
        item.data.masterInstanceID = instanceID;
    }

    Item& item = _items.data[ rev.identifier ] ;
    if( item.data.masterInstanceID != instanceID )
    {
        // trash data from different master mapping
        _releaseStreams( item );
        item.data.masterInstanceID = instanceID;
        item.used = usage;
    }
    else
        item.used = EQ_MAX( item.used, usage );

    if( item.data.versions.empty( ))
    {
        item.data.versions.push_back( new ObjectDataIStream ); 
    }
    else if( item.data.versions.back()->getPendingVersion() == rev.version )
    {
        if( item.data.versions.back()->isReady( ))
        {
#ifdef EQ_INSTRUMENT_CACHE
            ++nWriteReady;
#endif
            return false; // Already have stream
        }
        // else append data to stream
    }
    else
    {
        EQASSERT( item.data.versions.back()->isReady( ));

        const ObjectDataIStream* previous = item.data.versions.back();
        const uint128_t previousVersion = previous->getPendingVersion();

        if( previousVersion > rev.version )
        {
#ifdef EQ_INSTRUMENT_CACHE
            ++nWriteOld;
#endif
            return false;
        }
        if( ( previousVersion + 1 ) != rev.version ) // hole
        {
            EQASSERT( previousVersion < rev.version );

            if( item.access != 0 ) // are accessed - don't add
                return false;

            _releaseStreams( item );
        }
        else
        {
            EQASSERT( previous->isReady( ));
        }
        item.data.versions.push_back( new ObjectDataIStream ); 
    }

    item.time = _clock.getTime64();

    EQASSERT( !item.data.versions.empty( ));
    ObjectDataIStream* stream = item.data.versions.back();

    stream->addDataPacket( command );
    
    if( stream->isReady( ))
        _size += stream->getDataSize();

    _releaseItems( 1 );
    _releaseItems( 0 );

#ifdef EQ_INSTRUMENT_CACHE
    if( _items->find( rev.identifier ) != _items->end( ))
        ++nWriteHit;
    else
        ++nWriteMiss;
#endif
    return true;
}


const InstanceCache::Data& InstanceCache::operator[]( const co::base::UUID& id )
{
#ifdef EQ_INSTRUMENT_CACHE
    ++nRead;
#endif

    co::base::ScopedMutex<> mutex( _items );
    ItemHash::iterator i = _items->find( id );
    if( i == _items->end( ))
        return Data::NONE;

    Item& item = i->second;
    EQASSERT( !item.data.versions.empty( ));
    ++item.access;
    ++item.used;

#ifdef EQ_INSTRUMENT_CACHE
    ++nReadHit;
#endif
    return item.data;
}


bool InstanceCache::release( const co::base::UUID& id, const uint32_t count )
{
    co::base::ScopedMutex<> mutex( _items );
    ItemHash::iterator i = _items->find( id );
    if( i == _items->end( ))
        return false;

    Item& item = i->second;
    EQASSERT( !item.data.versions.empty( ));
    EQASSERT( item.access >= count );

    item.access -= count;
    _releaseItems( 1 );
    return true;
}

bool InstanceCache::erase( const co::base::UUID& id )
{
    co::base::ScopedMutex<> mutex( _items );
    ItemHash::iterator i = _items->find( id );
    if( i == _items->end( ))
        return false;

    Item& item = i->second;
    if( item.access != 0 )
        return false;

    _releaseStreams( item );
    _items->erase( i );
    return true;
}

void InstanceCache::expire( const int64_t timeout )
{
    const int64_t time = _clock.getTime64() - timeout;
    if( time <= 0 )
        return;

    std::vector< co::base::uint128_t > keys;

    co::base::ScopedMutex<> mutex( _items );
    for( ItemHash::iterator i = _items->begin(); i != _items->end(); ++i )
    {
        Item& item = i->second;
        EQASSERT( !item.data.versions.empty( ));

        if( item.time < time )
        {
            if( item.access == 0 )
            {
                _releaseStreams( item );
                keys.push_back( i->first );
            }
            else
                EQWARN << "Expired item " << i->first << " still accessed"
                       << std::endl;
        }
    }

    for( std::vector< co::base::uint128_t >::const_iterator i = keys.begin();
         i != keys.end(); ++i )
    {
        Item& item = _items.data[ *i ];
        if( item.data.versions.empty( ))
            _items->erase( *i );
    }
}

void InstanceCache::_releaseStreams( InstanceCache::Item& item )
{
    EQASSERT( !item.data.versions.empty( ));

    while( !item.data.versions.empty( ))
    {
        const ObjectDataIStream* stream = item.data.versions.back();
        item.data.versions.pop_back();

        EQASSERT( stream->isReady( ));
        EQASSERT( _size >= stream->getDataSize( ));
        _size -= stream->getDataSize();
        delete stream;
    }
}            

void InstanceCache::_releaseFirstStream( InstanceCache::Item& item )
{
    EQASSERT( !item.data.versions.empty( ));

    if( item.data.versions.empty( ))
        return;

    const ObjectDataIStream* stream = item.data.versions.front();
    item.data.versions.pop_front();

    EQASSERT( stream->isReady( ));
    EQASSERT( _size >= stream->getDataSize( ));
    _size -= stream->getDataSize();
    delete stream;
}            

void InstanceCache::_releaseItems( const uint32_t minUsage )
{
    if( _size <= _maxSize )
        return;

    EQ_TS_SCOPED( _thread );

    std::vector< co::base::uint128_t > keys;
    const uint64_t target = static_cast< uint64_t >(
                   static_cast< float >( _maxSize ) * 0.8f );

    // Release used items (first stream)
    bool streamsLeft = false;
    for( ItemHash::iterator i = _items->begin();
         i != _items->end() && _size > target; ++i )
    {
        Item& item = i->second;
        EQASSERT( !item.data.versions.empty( ));

        if( item.access == 0 && item.used >= minUsage )
        {
            _releaseFirstStream( item );
            if( !item.data.versions.empty( ))
                streamsLeft = true;

            keys.push_back( i->first );
#ifdef EQ_INSTRUMENT_CACHE
            ++nUsedRelease;
#endif
        }
    }

    // release used items (second..n streams)
    while( streamsLeft && _size > target )
    {
        streamsLeft = false;

        for( std::vector< co::base::uint128_t >::const_iterator i = keys.begin();
             i != keys.end() && _size > target; ++i )
        {
            Item& item = _items.data[ *i ];

            if( !item.data.versions.empty() && item.access == 0 && 
                item.used >= minUsage )
            {
                _releaseFirstStream( item );
                if( !item.data.versions.empty( ))
                    streamsLeft = true;
#ifdef EQ_INSTRUMENT_CACHE
                ++nUsedRelease;
#endif
            }
        }
    }

    for( std::vector< co::base::uint128_t >::const_iterator i = keys.begin();
         i != keys.end(); ++i )
    {
        Item& item = _items.data[ *i ];
        if( item.data.versions.empty( ))
            _items->erase( *i );
    }

    if( _size > target && minUsage == 0 )
        EQWARN << "Overfull instance cache, too many pinned items, size "
               << _size << " target " << target << " max " << _maxSize
               << " " << _items->size() << " entries"
#ifdef EQ_INSTRUMENT_CACHE
               << ": " << *this
#endif
               << std::endl;
}

std::ostream& operator << ( std::ostream& os,
                            const InstanceCache& instanceCache )
{
    os << "InstanceCache " << instanceCache.getSize() / 1048576 << "/" 
       << instanceCache.getMaxSize() / 1048576 << " MB"
#ifdef EQ_INSTRUMENT_CACHE
       << ", " << nReadHit << "/" << nRead << " reads, " << nWriteHit
       << "/" << nWrite << " writes (" << nWriteMiss << " misses, " << nWriteOld
       << " old, " << nWriteReady << " dups) " << nUsedRelease << " used, "
       << nUnusedRelease << " unused releases"
#endif
        ;
    return os;
}

}
