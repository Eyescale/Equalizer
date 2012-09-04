
/* Copyright (c) 2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <co/command.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/types.h>

namespace eq
{
namespace fabric
{

Object::Object()
        : _userData( 0 )
        , _tasks( TASK_NONE )
        , _error( ERROR_NONE )
        , _serial( EQ_INSTANCE_INVALID )
{}

Object::~Object()
{
    LBASSERTINFO( !isAttached(), "Object " << getID() << " is still attached" );
    LBASSERTINFO( !_userData,
                  "Unset user data before destruction to allow clean release" )
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

uint128_t Object::commit( const uint32_t incarnation )
{
    if( !_userData )
        return Serializable::commit( incarnation );

    if( !_userData->isAttached() && hasMasterUserData( ))
    {
        getLocalNode()->registerObject( _userData );
        _userData->setAutoObsolete( getUserDataLatency() + 1 );
        _data.userData = _userData;
        setDirty( DIRTY_USERDATA );
    }

    if( _userData->isDirty() && _userData->isAttached( ))
    {
        const uint128_t& version = _userData->commit( incarnation );
        LBASSERT( version != co::VERSION_NONE );
//        LBINFO << "Committed " << _userData->getID() << " v" << version
//               << " of " << lunchbox::className( _userData ) << " @"
//               << (void*)_userData << lunchbox::backtrace << std::endl;

        LBASSERT( !_userData->isDirty( ));
        LBASSERT( _data.userData.identifier != _userData->getID() ||
                  _data.userData.version <= version );

        if( _userData->isMaster() && _data.userData != _userData )
        {
            LBASSERTINFO( _data.userData.identifier != _userData->getID() ||
                          _data.userData.version < _userData->getVersion(),
                          _data.userData << " >= " <<
                          co::ObjectVersion( _userData ));

            _data.userData.identifier = _userData->getID();
            _data.userData.version = version;
            setDirty( DIRTY_USERDATA );
        }
    }

    return Serializable::commit( incarnation );
}

void Object::notifyDetach()
{
    Serializable::notifyDetach();
    if( !_userData )
        return;

    LBASSERT( !_userData->isAttached() ||
              _userData->isMaster() == hasMasterUserData( ));

    if( _userData->isMaster( ))
        _data.userData = co::ObjectVersion( 0 );

    getLocalNode()->releaseObject( _userData );
}

void Object::backup()
{
    LBASSERT( !_userData );
    _backup = _data;
}

void Object::restore()
{
    LBASSERT( !_userData );
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
        LBASSERT( !isMaster() ||
                  ( _removedChildren.empty() && dirtyBits == DIRTY_ALL ))
        os << _removedChildren;
        _removedChildren.clear();
    }
    if( (dirtyBits & DIRTY_SERIAL) && isMaster( ))
    {
        _serial = getInstanceID();
        os << _serial;
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
        std::vector< UUID > removed;
        is >> removed;
        if( !removed.empty( ))
        {
            LBASSERT( isMaster( ));
            std::vector< UUID >::const_iterator i = removed.begin();
            for( ; i != removed.end(); ++i )
            {
                removeChild( *i );
            }
        }
    }
    if( (dirtyBits & DIRTY_SERIAL) && !isMaster( ))
        is >> _serial;

    if( isMaster( )) // redistribute changes
        setDirty( dirtyBits & getRedistributableBits( ));

    // Update user data state
    if( !(dirtyBits & DIRTY_USERDATA) || !_userData )
        return;

    LBASSERTINFO( _data.userData.identifier != _userData->getID() ||
                  _data.userData.version >= _userData->getVersion() ||
                  _userData->isMaster(),
                  "Incompatible version, new " << _data.userData << " old " <<
                  co::ObjectVersion( _userData ));

    if( _data.userData.identifier == UUID::ZERO )
    {
        if( _userData->isAttached() && !_userData->isMaster( ))
        {
            LBASSERT( !hasMasterUserData( ));
            getLocalNode()->unmapObject( _userData );
        }
        return;
    }

    if( !_userData->isAttached() &&
        _data.userData.identifier != UUID::ZERO )
    {
        LBASSERT( !hasMasterUserData( ));
        //LBINFO << "Map " << _data.userData << lunchbox::backtrace
        //       << std::endl;
        if( !getLocalNode()->mapObject( _userData, _data.userData ))
        {
            LBWARN << "Mapping of " << lunchbox::className( _userData )
                   << " user data failed" << std::endl;
            LBUNREACHABLE;
            return;
        }
    }

    if( !_userData->isMaster() && _userData->isAttached( ))
    {
        LBASSERTINFO( _userData->getID() == _data.userData.identifier,
                      _userData->getID() << " != " << _data.userData.identifier );
#if 0
        if( _userData->getVersion() < _data.userData.version )
            LBINFO << "Sync " << _data.userData << lunchbox::backtrace
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
    LBASSERT( !userData || !userData->isAttached() );

    if( _userData == userData )
        return;

    if( _userData && _userData->isAttached( ))
    {
        LBASSERT( _userData->isMaster() == hasMasterUserData( ));
        _userData->getLocalNode()->releaseObject( _userData );
    }

    _userData = userData;

    if( hasMasterUserData( ))
        setDirty( DIRTY_USERDATA );
    else if( _data.userData.identifier != UUID::ZERO )
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
    _error = eq::fabric::Error( error );
    setDirty( DIRTY_ERROR );
}

void Object::postRemove( Object* child )
{
    LBASSERT( child );
    if( !child->isAttached( ))
        return;

    LBASSERT( !child->isMaster( ));
    LBASSERT( !isMaster( ));

    _removedChildren.push_back( child->getID( ));
    setDirty( DIRTY_REMOVED );

    co::LocalNodePtr localNode = child->getLocalNode();
    localNode->releaseObject( child );
}

bool Object::_cmdSync( co::Command& command )
{
    LBASSERT( isMaster( ));
    sync( co::VERSION_HEAD );
    return true;
}

}
}
