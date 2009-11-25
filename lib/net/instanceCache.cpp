
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

#include "instanceCache.h"

#include "command.h"
#include "objectInstanceDataIStream.h"
#include "objectVersion.h"

#include <eq/base/debug.h>

namespace eq
{
namespace net
{
#ifdef EQ_INSTRUMENT_CACHE
namespace
{
base::mtLong nRead;
base::mtLong nReadHit;
base::mtLong nWrite;
base::mtLong nWriteHit;
base::mtLong nUsedRelease;
base::mtLong nUnusedRelease;
}
#endif

const InstanceCache::Data InstanceCache::Data::NONE;

InstanceCache::InstanceCache( const long maxSize )
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
  : masterInstanceID( EQ_ID_INVALID )
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

    base::ScopedMutex mutex( _items );
    ItemHash::const_iterator i = _items->find( rev.id );
    if( i == _items->end( ))
    {
        Item& item = _items.data[ rev.id ];
        item.data.masterInstanceID = instanceID;
    }

    Item& item = _items.data[ rev.id ] ;
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
        item.data.versions.push_back( new ObjectInstanceDataIStream ); 
    }
    else if( item.data.versions.back()->getPendingVersion() == rev.version )
    {
        if( item.data.versions.back()->isReady( ))
            return false; // Already have stream
        // else append data to stream
    }
    else
    {
        EQASSERT( item.data.versions.back()->isReady( ));

        const ObjectDataIStream* previous = item.data.versions.back();
        const uint32_t previousVersion = previous->getPendingVersion();

        if( previousVersion > rev.version )
            return false;

        if( previousVersion + 1 != rev.version ) // hole
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
        item.data.versions.push_back( new ObjectInstanceDataIStream ); 
    }

    EQASSERT( !item.data.versions.empty( ));
    ObjectDataIStream* stream = item.data.versions.back();

    stream->addDataPacket( command );
    
    if( stream->isReady( ))
    {
        EQASSERT( stream->getDataSize() > 0 );
        _size += stream->getDataSize();
    }

    _releaseItems( 1 );
    _releaseItems( 0 );

#ifdef EQ_INSTRUMENT_CACHE
    if( _items->find( rev.id ) != _items->end( ))
        ++nWriteHit;
#endif
    return true;
}


const InstanceCache::Data& InstanceCache::operator[]( const uint32_t id )
{
#ifdef EQ_INSTRUMENT_CACHE
    ++nRead;
#endif

    base::ScopedMutex mutex( _items );
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


bool InstanceCache::release( const uint32_t id, const uint32_t count )
{
    base::ScopedMutex mutex( _items );
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

bool InstanceCache::erase( const uint32_t id )
{
    base::ScopedMutex mutex( _items );
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

void InstanceCache::_releaseStreams( InstanceCache::Item& item )
{
    EQASSERT( !item.data.versions.empty( ));

    while( !item.data.versions.empty( ))
    {
        const ObjectDataIStream* stream = item.data.versions.back();
        item.data.versions.pop_back();

        EQASSERT( stream->isReady( ));
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
    _size -= stream->getDataSize();
    delete stream;
}            

void InstanceCache::_releaseItems( const uint32_t minUsage )
{
    if( _size <= _maxSize )
        return;

    CHECK_THREAD_LOCK_BEGIN( _thread );

    std::vector< uint32_t > keys;
    const long target = static_cast< long >(
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

        for( std::vector< uint32_t >::const_iterator i = keys.begin();
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

    for( std::vector< uint32_t >::const_iterator i = keys.begin();
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

    CHECK_THREAD_LOCK_END( _thread );
}

std::ostream& operator << ( std::ostream& os,
                            const InstanceCache& instanceCache )
{
    os << "InstanceCache " << instanceCache.getSize() << " bytes"
#ifdef EQ_INSTRUMENT_CACHE
       << ", " << nReadHit << " of " << nRead << " reads, " << nWriteHit
       << " of " << nWrite << " writes, " << nUsedRelease << " used and " 
       << nUnusedRelease << " unused releases"
#endif
        ;
    return os;
}

}
}
