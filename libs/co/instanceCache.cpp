
/* Copyright (c) 2009-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <co/base/scopedMutex.h>

namespace co
{
//#define EQ_INSTRUMENT_CACHE
#ifdef EQ_INSTRUMENT_CACHE
namespace
{
base::a_int32_t nRead;
base::a_int32_t nReadHit;
base::a_int32_t nWrite;
base::a_int32_t nWriteHit;
base::a_int32_t nWriteMiss;
base::a_int32_t nWriteReady;
base::a_int32_t nWriteOld;
base::a_int32_t nUsedRelease;
base::a_int32_t nUnusedRelease;
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
        : used( 0 )
        , access( 0 )
{}

bool InstanceCache::add( const ObjectVersion& rev, const uint32_t instanceID,
                         Command& command, const uint32_t usage )
{
#ifdef EQ_INSTRUMENT_CACHE
    ++nWrite;
#endif

    const NodeID nodeID = command.getNode()->getNodeID();

    base::ScopedMutex<> mutex( _items );
    ItemHash::const_iterator i = _items->find( rev.identifier );
    if( i == _items->end( ))
    {
        Item& item = _items.data[ rev.identifier ];
        item.data.masterInstanceID = instanceID;
        item.from = nodeID;
    }

    Item& item = _items.data[ rev.identifier ] ;
    if( item.data.masterInstanceID != instanceID || item.from != nodeID )
    {
        EQASSERT( !item.access ); // same master with different instance ID?!
        if( item.access != 0 ) // are accessed - don't add
            return false;
        // trash data from different master mapping
        _releaseStreams( item );
        item.data.masterInstanceID = instanceID;
        item.from = nodeID;
        item.used = usage;
    }
    else
        item.used = EQ_MAX( item.used, usage );

    if( item.data.versions.empty( ))
    {
        item.data.versions.push_back( new ObjectDataIStream ); 
        item.times.push_back( _clock.getTime64( ));
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
        const ObjectDataIStream* previous = item.data.versions.back();
        EQASSERT( previous->isReady( ));

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
        item.times.push_back( _clock.getTime64( ));
    }

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

void InstanceCache::remove( const NodeID& nodeID )
{
    std::vector< base::uint128_t > keys;

    base::ScopedMutex<> mutex( _items );
    for( ItemHash::iterator i = _items->begin(); i != _items->end(); ++i )
    {
        Item& item = i->second;
        if( item.from != nodeID )
            continue;

        EQASSERT( !item.access );
        if( item.access != 0 )
            continue;

        _releaseStreams( item );
        keys.push_back( i->first );
    }

    for( std::vector< base::uint128_t >::const_iterator i = keys.begin();
         i != keys.end(); ++i )
    {
        _items->erase( *i );
    }
}

const InstanceCache::Data& InstanceCache::operator[]( const base::UUID& id )
{
#ifdef EQ_INSTRUMENT_CACHE
    ++nRead;
#endif

    base::ScopedMutex<> mutex( _items );
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

bool InstanceCache::release( const base::UUID& id, const uint32_t count )
{
    base::ScopedMutex<> mutex( _items );
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

bool InstanceCache::erase( const base::UUID& id )
{
    base::ScopedMutex<> mutex( _items );
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

    std::vector< base::uint128_t > keys;

    base::ScopedMutex<> mutex( _items );
    for( ItemHash::iterator i = _items->begin(); i != _items->end(); ++i )
    {
        Item& item = i->second;
        if( item.access != 0 )
            continue;

        _releaseStreams( item, time );
        if( item.data.versions.empty( ))
            keys.push_back( i->first );
    }

    for( std::vector< base::uint128_t >::const_iterator i = keys.begin();
         i != keys.end(); ++i )
    {
        _items->erase( *i );
    }
}

void InstanceCache::_releaseStreams( InstanceCache::Item& item, 
                                     const int64_t minTime )
{
    EQASSERT( item.access == 0 );
    while( !item.data.versions.empty() && item.times.front() <= minTime &&
           item.data.versions.front()->isReady( ))
    {
        _releaseFirstStream( item );
    }
}

void InstanceCache::_releaseStreams( InstanceCache::Item& item )
{
    EQASSERT( item.access == 0 );
    EQASSERT( !item.data.versions.empty( ));

    while( !item.data.versions.empty( ))
    {
        ObjectDataIStream* stream = item.data.versions.back();
        item.data.versions.pop_back();
        _deleteStream( stream );
    }
    item.times.clear();
}            

void InstanceCache::_releaseFirstStream( InstanceCache::Item& item )
{
    EQASSERT( item.access == 0 );
    EQASSERT( !item.data.versions.empty( ));
    if( item.data.versions.empty( ))
        return;

    ObjectDataIStream* stream = item.data.versions.front();
    item.data.versions.pop_front();
    item.times.pop_front();
    _deleteStream( stream );
}            

void InstanceCache::_deleteStream( ObjectDataIStream* stream )
{
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

    std::vector< base::uint128_t > keys;
    const uint64_t target = uint64_t( float( _maxSize ) * 0.8f );

    // Release used items (first stream)
    bool streamsLeft = false;
    for( ItemHashIter i = _items->begin();
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

        for( std::vector< base::uint128_t >::const_iterator i = keys.begin();
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

    for( std::vector< base::uint128_t >::const_iterator i = keys.begin();
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
