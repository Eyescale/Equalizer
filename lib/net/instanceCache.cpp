
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
}
#endif

template< typename K > InstanceCache< K >::InstanceCache( const long maxSize )
        : _maxSize( maxSize )
        , _size( 0 )
{}

template< typename K > InstanceCache< K >::~InstanceCache()
{
    for( typename Data::iterator i = _data.begin(); i != _data.end(); ++i )
    {
        Item& item = i->second;
        EQASSERT( item.stream );

        delete item.stream;
        item.stream = 0;
    }

    _data.clear();
    _size = 0;
}

template< typename K > InstanceCache< K >::Item::Item()
        : size( 0 )
        , stream( 0 )
        , used( 0 )
        , pinned( 0 )
{}

template< typename K > 
bool InstanceCache< K >::add( const K& key, Command& command, 
                              const bool fixed )
{
#ifdef EQ_INSTRUMENT_CACHE
    ++nWrite;
#endif

    base::ScopedMutex mutex( _mutex );
    typename Data::iterator i = _data.find( key );
    if( i == _data.end( ))
    {
        Item& item = _data[ key ] ;
        item.stream = new ObjectInstanceDataIStream;
        item.pinned = fixed ? 1 : 0;
        i = _data.find( key );
    }

    Item& item = i->second;;

    item.size += command.getPacket()->size;
    _size += command.getPacket()->size;
    item.stream->addDataPacket( command );
    if( fixed )
        item.pinned = 1;

    _releaseItems();

#ifdef EQ_INSTRUMENT_CACHE
    if( _data.find( key ) != _data.end( ))
        ++nWriteHit;
#endif
    return true;
}


template< typename K > 
DataIStream* InstanceCache< K >::operator[]( const K& key )
{
#ifdef EQ_INSTRUMENT_CACHE
    ++nRead;
#endif

    base::ScopedMutex mutex( _mutex );
    typename Data::iterator i = _data.find( key );
    if( i == _data.end( ))
        return 0;

    Item& item = i->second;
    EQASSERT( item.stream );
    ++item.pinned;

#ifdef EQ_INSTRUMENT_CACHE
    ++nReadHit;
#endif
    return item.stream;
}

template< typename K > 
bool InstanceCache< K >::pin( const K& key )
{
    return ( (*this)[ key ] != 0 );
}


template< typename K > 
bool InstanceCache< K >::unpin( const K& key )
{
    base::ScopedMutex mutex( _mutex );
    typename Data::iterator i = _data.find( key );
    if( i == _data.end( ))
        return false;

    Item& item = i->second;
    EQASSERT( item.stream );
    EQASSERT( item.pinned );

    --item.pinned;
    ++item.used;
    return true;
}

template< typename K > 
bool InstanceCache< K >::erase( const K& key )
{
    base::ScopedMutex mutex( _mutex );
    typename Data::iterator i = _data.find( key );
    if( i == _data.end( ))
        return false;

    Item& item = i->second;
    if( item.pinned != 0 )
    {
        EQINFO << "Refusing to erase pinned item" << std::endl;
        return false;
    }

    EQASSERT( item.stream );
    _size -= item.size;
    delete item.stream;
    item.stream = 0;

    _data.erase( i );
    return true;
}

template< typename K > 
void InstanceCache< K >::_releaseItems()
{
    if( _size <= _maxSize )
        return;

    CHECK_THREAD_LOCK_BEGIN( _thread );

    std::vector< K > releasedKeys;
    const long target = static_cast< long >(
                   static_cast< float >( _maxSize ) * 0.8f );

    // Release used items
    for( typename Data::iterator i = _data.begin(); 
         i != _data.end() && _size > target; ++i )
    {
        Item& item = i->second;
        if( item.pinned == 0 && item.used > 0 )
        {
            EQASSERT( item.stream );
            _size -= item.size;
            delete item.stream;
            item.stream = 0;

            releasedKeys.push_back( i->first );
        }
    }

    for( typename std::vector< K >::const_iterator i = releasedKeys.begin();
         i != releasedKeys.end(); ++i )
    {
        _data.erase( *i );
    }
    releasedKeys.clear();

    // Release unpinned items
    for( typename Data::iterator i = _data.begin(); 
         i != _data.end() && _size > target; ++i )
    {
        Item& item = i->second;
        if( item.pinned == 0 )
        {
            EQASSERT( item.stream );
            _size -= item.size;
            delete item.stream;
            item.stream = 0;

            releasedKeys.push_back( i->first );
        }
    }

    for( typename std::vector< K >::const_iterator i = releasedKeys.begin();
         i != releasedKeys.end(); ++i )
    {
        _data.erase( *i );
    }

    if( _size > target )
        EQWARN << "Overfull instance cache, too many pinned items" << std::endl;

    CHECK_THREAD_LOCK_END( _thread );
}

template< typename K >
std::ostream& operator << ( std::ostream& os,
                            const InstanceCache< K >& instanceCache )
{
    os << "InstanceCache " << instanceCache.getSize() << " bytes" 
#ifdef EQ_INSTRUMENT_CACHE
       << ", " << nReadHit << " of " << nRead << " reads, " << nWriteHit
       << " of " << nWrite << " writes"
#endif
        ;
    return os;
}

template class InstanceCache< ObjectVersion >;

}
}
