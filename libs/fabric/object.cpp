
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

#include "object.h"

#include "task.h"

#include <co/types.h>

namespace eq
{
namespace fabric
{

Object::Object()
        : _userData( 0 )
        , _tasks( TASK_NONE )
        , _error( ERROR_NONE )
{}

Object::~Object()
{
    EQASSERTINFO( !_userData,
                  "Unset user data before destructor to allow clean release" )
    co::LocalNodePtr node = getLocalNode();
    if( node.isValid() )
        node->releaseObject( this );
}

bool Object::isDirty() const
{
    if( _userData && _userData->isAttached( ))
    {
        if( _userData->isMaster( ))
            _userData->sync(); // apply slave object commits
        return Serializable::isDirty() || _userData->isDirty();
    }

    // else
    return Serializable::isDirty();
}

uint32_t Object::commitNB()
{
    if( _userData )
    {
        if( !_userData->isAttached() && hasMasterUserData( ))
        {
            getLocalNode()->registerObject( _userData );
            _data.userData = _userData;
            setDirty( DIRTY_USERDATA );
        }

        if( _userData->isDirty() && _userData->isAttached( ))
        {
            const uint128_t& version = _userData->commit();
            EQASSERT( version != co::VERSION_NONE );
//            EQINFO << "Committed " << _userData->getID() << " v" << version
//                   << " of " << co::base::className( _userData ) << " @"
//                   << (void*)_userData << co::base::backtrace << std::endl;

            EQASSERT( !_userData->isDirty( ));
            EQASSERT( _data.userData.identifier != _userData->getID() ||
                      _data.userData.version <= version );

            if( _userData->isMaster() && _data.userData != _userData )
            {
                EQASSERTINFO( _data.userData.identifier != _userData->getID() ||
                              _data.userData.version < _userData->getVersion(),
                              _data.userData << " >= " <<
                              co::ObjectVersion( _userData ));

                _data.userData.identifier = _userData->getID();
                _data.userData.version = version;
                setDirty( DIRTY_USERDATA );
            }
        }
    }

    return Serializable::commitNB();
}

void Object::notifyDetach()
{
    Serializable::notifyDetach();
    if( !_userData )
        return;

    EQASSERT( !_userData->isAttached() ||
              _userData->isMaster() == hasMasterUserData( ));

    if( _userData->isMaster( ))
        _data.userData = co::ObjectVersion( 0 );

    getLocalNode()->releaseObject( _userData );
}

void Object::backup()
{
    EQASSERT( !_userData );
    _backup = _data;
}

void Object::restore()
{
    EQASSERT( !_userData );
    _data = _backup;
    setDirty( DIRTY_NAME | DIRTY_USERDATA );
}

void Object::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        os << _data.name;
    if( dirtyBits & DIRTY_USERDATA )
        os << _data.userData;
    if( dirtyBits & DIRTY_TASKS )
        os << _tasks;
    if( dirtyBits & DIRTY_ERROR )
        os << _error;
    if( dirtyBits & DIRTY_REMOVED )
    {
        EQASSERT( !isMaster() || 
                  ( _removedChildren.empty() && dirtyBits == DIRTY_ALL ))
        os << _removedChildren;
        _removedChildren.clear();
    }
}

void Object::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        is >> _data.name;
    if( dirtyBits & DIRTY_USERDATA )
    {
        is >> _data.userData;
        // map&sync below to allow early exits
    }
    if( dirtyBits & DIRTY_TASKS )
        is >> _tasks;
    if( dirtyBits & DIRTY_ERROR )
        is >> _error;
    if( dirtyBits & DIRTY_REMOVED )
    {
        std::vector< co::base::UUID > removed;
        is >> removed;
        if( !removed.empty( ))
        {
            EQASSERT( isMaster( ));
            std::vector< co::base::UUID >::const_iterator i = removed.begin();
            for( ; i != removed.end(); ++i )
            {
                removeChild( *i );
            }
        }
    }

    if( isMaster( )) // redistribute changes
        setDirty( dirtyBits & getRedistributableBits( ));

    // Update user data state
    if( !(dirtyBits & DIRTY_USERDATA) ||!_userData )
        return;

    EQASSERTINFO( _data.userData.identifier != _userData->getID() ||
                  _data.userData.version >= _userData->getVersion() ||
                  _userData->isMaster(),
                  "Incompatible version, new " << _data.userData << " old " <<
                  co::ObjectVersion( _userData ));

    if( _data.userData.identifier == co::base::UUID::ZERO )
    {
        if( _userData->isAttached() && !_userData->isMaster( ))
        {
            EQASSERT( !hasMasterUserData( ));
            getLocalNode()->unmapObject( _userData );
        }
        return;
    }

    if( !_userData->isAttached() &&
        _data.userData.identifier != co::base::UUID::ZERO )
    {
        EQASSERT( !hasMasterUserData( ));
        //EQINFO << "Map " << _data.userData << co::base::backtrace
        //       << std::endl;
        if( !getLocalNode()->mapObject( _userData, _data.userData ))
        {
            EQWARN << "Mapping of " << co::base::className( _userData )
                   << " user data failed" << std::endl;
            return;
        }
    }

    if( !_userData->isMaster() && _userData->isAttached( ))
    {
        EQASSERTINFO( _userData->getID() == _data.userData.identifier,
                      _userData->getID() << " != " << _data.userData.identifier );
#if 0
        if( _userData->getVersion() < _data.userData.version )
            EQINFO << "Sync " << _data.userData << co::base::backtrace
                   << std::endl;
#endif
        _userData->sync( _data.userData.version );
    }
}

void Object::setName( const std::string& name )
{
    if( _data.name == name )
        return;
    _data.name = name;
    setDirty( DIRTY_NAME );
}

void Object::setUserData( co::Object* userData )
{
    EQASSERT( !userData || !userData->isAttached() );

    if( _userData == userData )
        return;

    if( _userData && _userData->isAttached( ))
    {
        EQASSERT( _userData->isMaster() == hasMasterUserData( ));
        _userData->getLocalNode()->releaseObject( _userData );
    }

    _userData = userData;

    if( hasMasterUserData( ))
        setDirty( DIRTY_USERDATA );
    else if( _data.userData.identifier != co::base::UUID::ZERO )
    {
        co::LocalNodePtr node = getLocalNode();
        if( node.isValid() )
            node->mapObject( _userData, _data.userData );
    }
}

const std::string& Object::getName() const
{
    return _data.name;
}

void Object::setTasks( const uint32_t tasks )
{
    if( _tasks == tasks )
        return;
    _tasks = tasks;
    setDirty( DIRTY_TASKS );
}

void Object::setError( const int32_t error )
{
    if( _error == error )
        return;
    _error = co::base::Error( error );
    setDirty( DIRTY_ERROR );
}

void Object::postRemove( const Object* child )
{
    EQASSERT( child );
    if( !child->isAttached( ))
        return;

    EQASSERT( !child->isMaster( ));
    EQASSERT( !isMaster( ));

    _removedChildren.push_back( child->getID( ));
    setDirty( DIRTY_REMOVED );
}

}
}
