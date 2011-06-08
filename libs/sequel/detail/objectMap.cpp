
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "objectMap.h"

#include "config.h"

#include <eq/sequel/objectFactory.h>
#include <co/base/scopedMutex.h>

namespace seq
{
namespace detail
{
typedef co::base::ScopedMutex< co::base::SpinLock> ScopedMutex;

ObjectMap::ObjectMap( ObjectFactory& factory )
        : _factory( factory )
{}

ObjectMap::~ObjectMap()
{
    eq::Config* config = _factory.getConfig();

    for( co::ObjectsCIter i = _masters.begin(); i != _masters.end(); ++i )
    {
        Object* object = *i;
        _map.erase( object->getID( ));
        config->deregisterObject( object );
    }
    _masters.clear();

    for( MapCIter i = _map.begin(); i != _map.end(); ++i )
    {
        const Entry& entry = i->second;
        if( !entry.instance )
            continue;

        config->unmapObject( entry.instance );
        _factory.destroyObject( entry.instance, entry.type );
    }
    _map.clear();
}

ObjectMap::Entry::Entry(const uint128_t& v, co::Object* i, const uint32_t t)
        : version( v )
        , instance( i )
        , type( t )
{}

uint32_t ObjectMap::commitNB( const uint32_t incarnation )
{
    _commitMasters( incarnation );
    return Serializable::commitNB( incarnation );
}

bool ObjectMap::isDirty() const
{
    if( Serializable::isDirty( ))
        return true;

    ScopedMutex mutex( _mutex );
    for( co::ObjectsCIter i = _masters.begin(); i != _masters.end(); ++i )
        if( (*i)->isDirty( ))
            return true;
    return false;
}

void ObjectMap::_commitMasters( const uint32_t incarnation )
{
    ScopedMutex mutex( _mutex );
    std::vector< uint32_t > requests;
    std::vector< co::Object* > objects;

    for( co::ObjectsCIter i = _masters.begin(); i != _masters.end(); ++i )
    {
        co::Object* object = *i;
        if( !object->isDirty( ))
            continue;

        requests.push_back( object->commitNB( incarnation ));
        objects.push_back( object );
    }

    const size_t size = requests.size();
    for( size_t i = 0; i < size; ++i )
    {
        const co::ObjectVersion ov( objects[i]->getID(),
                                    objects[i]->commitSync( requests[i] ));
        Entry& entry = _map[ ov.identifier ];
        if( entry.version == ov.version )
            continue;

        entry.version = ov.version;
        _changed.push_back( ov );
    }
    if( !_changed.empty( ))
        setDirty( DIRTY_CHANGED );
}

void ObjectMap::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    Serializable::serialize( os, dirtyBits );
    ScopedMutex mutex( _mutex );
    if( dirtyBits == DIRTY_ALL )
    {
        for( MapCIter i = _map.begin(); i != _map.end(); ++i )
            os << i->first << i->second.version << i->second.type;
        os << co::ObjectVersion::NONE;
        return;
    }

    if( dirtyBits & DIRTY_ADDED )
    {
        os << _added;
        for( std::vector< uint128_t >::const_iterator i = _added.begin();
             i != _added.end(); ++i )
        {
            const Entry& entry = _map[ *i ];
            os << entry.version << entry.type;
        }
        _added.clear();
    }
    if( dirtyBits & DIRTY_CHANGED )
    {
        os << _changed;
        _changed.clear();
    }

    if( dirtyBits & DIRTY_INITDATA )
        os << _initData;
    if( dirtyBits & DIRTY_FRAMEDATA )
        os << _frameData;
}

void ObjectMap::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    Serializable::deserialize( is, dirtyBits );
    ScopedMutex mutex( _mutex );
    if( dirtyBits == DIRTY_ALL )
    {
        EQASSERT( _map.empty( ));
        
        co::ObjectVersion ov;
        is >> ov;
        while( ov != co::ObjectVersion::NONE )
        {
            EQASSERT( _map.find( ov.identifier ) == _map.end( ));
            Entry& entry = _map[ ov.identifier ];
            entry.version = ov.version;
            is >> entry.type >> ov;
        }
        return;
    }

    if( dirtyBits & DIRTY_ADDED )
    {
        std::vector< uint128_t > added;
        is >> added;

        for( std::vector< uint128_t >::const_iterator i = added.begin();
             i != added.end(); ++i )
        {
            EQASSERT( _map.find( *i ) == _map.end( ));
            Entry& entry = _map[ *i ];
            is >> entry.version >> entry.type;
        }
    }
    if( dirtyBits & DIRTY_CHANGED )
    {
        co::ObjectVersions changed;
        is >> changed;

        for( co::ObjectVersionsCIter i = changed.begin(); i!=changed.end(); ++i)
        {
            const co::ObjectVersion& ov = *i;
            EQASSERT( _map.find( ov.identifier ) != _map.end( ));

            Entry& entry = _map[ ov.identifier ];
            entry.version = ov.version;
            EQASSERT( !entry.instance || entry.instance->isAttached( ));

            if( entry.instance && !entry.instance->isMaster( ))
                entry.instance->sync( ov.version );
        }
    }

    if( dirtyBits & DIRTY_INITDATA )
        is >> _initData;
    if( dirtyBits & DIRTY_FRAMEDATA )
        is >> _frameData;
}

bool ObjectMap::register_( co::Object* object, const uint32_t type )
{
    EQASSERT( object );
    if( !object || !_factory.getConfig()->registerObject( object ))
        return false;

    const Entry entry( object->getVersion(), object, type );
    ScopedMutex mutex( _mutex );
    EQASSERT( _map.find( object->getID( )) == _map.end( ));

    _map[ object->getID() ] = entry;
    _masters.push_back( object );
    _added.push_back( object->getID( ));
    setDirty( DIRTY_ADDED );
    return true;
}

co::Object* ObjectMap::get( const uint128_t& identifier, co::Object* instance )
{
    if( identifier == uint128_t::ZERO )
        return 0;

    ScopedMutex mutex( _mutex );
    MapIter i = _map.find( identifier );
    EQASSERT( i != _map.end( ));
    if( i == _map.end( ))
    {
        EQWARN << "Object mapping failed, no master registered" << std::endl;
        return 0;
    }

    Entry& entry = i->second;
    if( entry.instance )
    {
        EQASSERTINFO( !instance || entry.instance == instance,
                      entry.instance << " != " << instance )
        if( !instance || entry.instance == instance )
            return entry.instance;

        EQWARN << "Object mapping failed, different instance registered"
               << std::endl;
        return 0;
    }
    EQASSERT( entry.type != OBJECTTYPE_NONE );

    co::Object* object = instance ? 
                         instance : _factory.createObject( entry.type );
    EQASSERT( object );
    if( !object )
        return 0;

    if( !_factory.getConfig()->mapObject( object, identifier, entry.version ))
    {
        if( !instance )
            _factory.destroyObject( object, entry.type );
        return 0;
    }

    EQASSERT( object->getVersion() == entry.version );
    entry.instance = object;
    return object;
}

void ObjectMap::setInitData( co::Object* object )
{
    const uint128_t identifier = object ? object->getID() : uint128_t::ZERO;
    if( _initData == identifier )
        return;
    
    _initData = identifier;
    setDirty( DIRTY_INITDATA );
}

void ObjectMap::setFrameData( co::Object* object )
{
    const uint128_t identifier = object ? object->getID() : uint128_t::ZERO;
    if( _frameData == identifier )
        return;
    
    _frameData = identifier;
    setDirty( DIRTY_FRAMEDATA );
}

}
}
