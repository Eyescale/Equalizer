
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@googlemail.com>
 *               2012, Stefan Eilemann <eile@eyescale.ch>
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

#include <lunchbox/scopedMutex.h>

namespace co
{
namespace
{
struct Entry //!< One object map item
{
    Entry() : instance( 0 ), type( OBJECTTYPE_NONE ) {}
    Entry( const uint128_t& v, Object* i, const uint32_t t )
            : version( v ), instance( i ), type( t ) {}

    uint128_t version;    //!< The current version of the object
    Object* instance; //!< The object instance, if attached
    uint32_t type;        //!< The object class id
};

typedef stde::hash_map< uint128_t, Entry > Map;
typedef Map::iterator MapIter;
typedef Map::const_iterator MapCIter;
}

namespace detail
{
class ObjectMap
{
public:
    ObjectMap( LocalNodePtr ln, ObjectFactory& f )
            : localNode( ln ) , factory( f ) {}

    ~ObjectMap()
        {
            for( ObjectsCIter i = masters.begin(); i != masters.end(); ++i )
            {
                Object* object = *i;
                map.erase( object->getID( ));
                localNode->deregisterObject( object );
            }
            masters.clear();

            for( MapCIter i = map.begin(); i != map.end(); ++i )
            {
                const Entry& entry = i->second;
                if( !entry.instance )
                    continue;

                localNode->unmapObject( entry.instance );
                factory.destroyObject( entry.instance, entry.type );
            }
            map.clear();
        }

    LocalNodePtr localNode;
    ObjectFactory& factory; //!< The 'parent' user

    mutable lunchbox::SpinLock mutex;

    Map map; //!< the actual map
    Objects masters; //!< Master objects registered with this instance

    /** Added master objects since the last commit. */
    std::vector< uint128_t > added;

    /** Changed master objects since the last commit. */
    ObjectVersions changed;
};
}

ObjectMap::ObjectMap( LocalNodePtr localNode, ObjectFactory& factory )
        : _impl( new detail::ObjectMap( localNode, factory ))
{}

ObjectMap::~ObjectMap()
{
    delete _impl;
}

uint128_t ObjectMap::commit( const uint32_t incarnation )
{
    _commitMasters( incarnation );
    return Serializable::commit( incarnation );
}

bool ObjectMap::isDirty() const
{
    if( Serializable::isDirty( ))
        return true;

    lunchbox::ScopedFastRead mutex( _impl->mutex );
    for( ObjectsCIter i =_impl->masters.begin(); i !=_impl->masters.end(); ++i )
        if( (*i)->isDirty( ))
            return true;
    return false;
}

void ObjectMap::_commitMasters( const uint32_t incarnation )
{
    lunchbox::ScopedFastWrite mutex( _impl->mutex );

    for( ObjectsCIter i =_impl->masters.begin(); i !=_impl->masters.end(); ++i )
    {
        Object* object = *i;
        if( !object->isDirty( ))
            continue;

        const ObjectVersion ov( object->getID(),
                                    object->commit( incarnation ));
        Entry& entry = _impl->map[ ov.identifier ];
        if( entry.version == ov.version )
            continue;

        entry.version = ov.version;
        _impl->changed.push_back( ov );
    }
    if( !_impl->changed.empty( ))
        setDirty( DIRTY_CHANGED );
}

void ObjectMap::serialize( DataOStream& os, const uint64_t dirtyBits )
{
    Serializable::serialize( os, dirtyBits );
    lunchbox::ScopedFastWrite mutex( _impl->mutex );
    if( dirtyBits == DIRTY_ALL )
    {
        for( MapCIter i = _impl->map.begin(); i != _impl->map.end(); ++i )
            os << i->first << i->second.version << i->second.type;
        os << ObjectVersion::NONE;
        return;
    }

    if( dirtyBits & DIRTY_ADDED )
    {
        os << _impl->added;
        for( std::vector< uint128_t >::const_iterator i = _impl->added.begin();
             i != _impl->added.end(); ++i )
        {
            const Entry& entry = _impl->map[ *i ];
            os << entry.version << entry.type;
        }
        _impl->added.clear();
    }
    if( dirtyBits & DIRTY_CHANGED )
    {
        os << _impl->changed;
        _impl->changed.clear();
    }
}

void ObjectMap::deserialize( DataIStream& is, const uint64_t dirtyBits )
{
    Serializable::deserialize( is, dirtyBits );
    lunchbox::ScopedFastWrite mutex( _impl->mutex );
    if( dirtyBits == DIRTY_ALL )
    {
        EQASSERT( _impl->map.empty( ));
        
        ObjectVersion ov;
        is >> ov;
        while( ov != ObjectVersion::NONE )
        {
            EQASSERT( _impl->map.find( ov.identifier ) == _impl->map.end( ));
            Entry& entry = _impl->map[ ov.identifier ];
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
            EQASSERT( _impl->map.find( *i ) == _impl->map.end( ));
            Entry& entry = _impl->map[ *i ];
            is >> entry.version >> entry.type;
        }
    }
    if( dirtyBits & DIRTY_CHANGED )
    {
        ObjectVersions changed;
        is >> changed;

        for( ObjectVersionsCIter i = changed.begin(); i!=changed.end(); ++i)
        {
            const ObjectVersion& ov = *i;
            EQASSERT( _impl->map.find( ov.identifier ) != _impl->map.end( ));

            Entry& entry = _impl->map[ ov.identifier ];
            entry.version = ov.version;
            EQASSERT( !entry.instance || entry.instance->isAttached( ));

            if( entry.instance && !entry.instance->isMaster( ))
                entry.instance->sync( ov.version );
        }
    }
}

bool ObjectMap::register_( Object* object, const uint32_t type )
{
    EQASSERT( object );
    if( !object || !_impl->localNode->registerObject( object ))
        return false;

    const Entry entry( object->getVersion(), object, type );
    lunchbox::ScopedFastWrite mutex( _impl->mutex );
    EQASSERT( _impl->map.find( object->getID( )) == _impl->map.end( ));

    _impl->map[ object->getID() ] = entry;
    _impl->masters.push_back( object );
    _impl->added.push_back( object->getID( ));
    setDirty( DIRTY_ADDED );
    return true;
}

Object* ObjectMap::get( const uint128_t& identifier, Object* instance )
{
    if( identifier == uint128_t::ZERO )
        return 0;

    lunchbox::ScopedFastWrite mutex( _impl->mutex );
    MapIter i = _impl->map.find( identifier );
    EQASSERT( i != _impl->map.end( ));
    if( i == _impl->map.end( ))
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

    Object* object = instance ?
                         instance : _impl->factory.createObject( entry.type );
    EQASSERT( object );
    if( !object )
        return 0;

    if( !_impl->localNode->mapObject( object, identifier, entry.version ))
    {
        if( !instance )
            _impl->factory.destroyObject( object, entry.type );
        return 0;
    }

    EQASSERT( object->getVersion() == entry.version );
    entry.instance = object;
    return object;
}

}
