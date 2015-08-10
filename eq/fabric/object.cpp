
/* Copyright (c) 2009-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <co/iCommand.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/types.h>

namespace eq
{
namespace fabric
{
namespace detail
{
struct BackupData
{
    /** The application-defined name of the object. */
    std::string name;

    /** The user data parameters if no _userData object is set. */
    co::ObjectVersion userData;
};

class Object
{
public:
    Object()
        : userData( 0 )
        , tasks( TASK_NONE )
        , serial( CO_INSTANCE_INVALID )
    {}

    Object( const Object& from )
        : data( from.data )
        , backup()
        , userData( from.userData )
        , tasks( from.tasks )
        , serial( from.serial )
    {}

    // cppcheck-suppress operatorEqVarError
    Object& operator = ( const Object& from )
    {
        if( this == &from )
            return *this;

        data = from.data;
        userData = from.userData;
        tasks = from.tasks;
        serial = from.serial;
        return *this;
    }

    BackupData data; //!< Active data
    BackupData backup; //!< Backed up version (from .eqc)

    co::Object* userData; //!< User data object
    uint32_t tasks; //!< Worst-case set of tasks
    uint32_t serial; //!< Server-unique serial number

    /** The identifiers of removed children since the last slave commit. */
    std::vector< uint128_t > removedChildren;
};
}

Object::Object()
    : _impl( new detail::Object )
{}

Object::Object( const Object& from )
    : co::Serializable( from )
    , _impl( new detail::Object( *from._impl ))
{}

Object::~Object()
{
    LBASSERTINFO( !isAttached(), "Object " << getID() << " is still attached" );
    LBASSERTINFO( !_impl->userData,
                  "Unset user data before destruction to allow clean release" )

    co::LocalNodePtr node = getLocalNode();
    if( node.isValid() )
        node->releaseObject( this );
    delete _impl;
}

Object& Object::operator = ( const Object& from )
{
    if( this == &from )
        return *this;

    co::Serializable::operator = ( from );
    *_impl = *from._impl;
    return *this;
}

bool Object::isDirty() const
{
    if( _impl->userData && _impl->userData->isAttached( ))
    {
        if( _impl->userData->isMaster( ))
            _impl->userData->sync(); // apply slave object commits
        return Serializable::isDirty() || _impl->userData->isDirty();
    }

    // else
    return Serializable::isDirty();
}

uint128_t Object::commit( const uint32_t incarnation )
{
    if( !_impl->userData )
        return Serializable::commit( incarnation );

    if( !_impl->userData->isAttached() && hasMasterUserData( ))
    {
        getLocalNode()->registerObject( _impl->userData );
        _impl->userData->setAutoObsolete( getUserDataLatency() + 1 );
        _impl->data.userData = _impl->userData;
        setDirty( DIRTY_USERDATA );
    }

    if( _impl->userData->isDirty() && _impl->userData->isAttached( ))
    {
        const uint128_t& version = _impl->userData->commit( incarnation );
        LBASSERT( version != co::VERSION_NONE );
        LBASSERT( !_impl->userData->isDirty( ));
        LBASSERT( _impl->data.userData.identifier != _impl->userData->getID() ||
                  _impl->data.userData.version <= version );

        if( _impl->userData->isMaster() &&
            _impl->data.userData != co::ObjectVersion( _impl->userData ))
        {
            LBASSERTINFO( _impl->data.userData.identifier !=
                          _impl->userData->getID() ||
                          _impl->data.userData.version <
                          _impl->userData->getVersion(),
                          _impl->data.userData << " >= " <<
                          co::ObjectVersion( _impl->userData ));

            _impl->data.userData.identifier = _impl->userData->getID();
            _impl->data.userData.version = version;
            setDirty( DIRTY_USERDATA );
        }
    }

    return Serializable::commit( incarnation );
}

void Object::notifyDetach()
{
    Serializable::notifyDetach();
    if( !_impl->userData )
        return;

    LBASSERT( !_impl->userData->isAttached() ||
              _impl->userData->isMaster() == hasMasterUserData( ));

    if( _impl->userData->isMaster( ))
        _impl->data.userData = co::ObjectVersion( 0 );

    getLocalNode()->releaseObject( _impl->userData );
}

void Object::backup()
{
    LBASSERT( !_impl->userData );
    _impl->backup = _impl->data;
}

void Object::restore()
{
    LBASSERT( !_impl->userData );
    _impl->data = _impl->backup;
    setDirty( DIRTY_NAME | DIRTY_USERDATA );
}

void Object::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        os << _impl->data.name;
    if( dirtyBits & DIRTY_USERDATA )
        os << _impl->data.userData;
    if( dirtyBits & DIRTY_TASKS )
        os << _impl->tasks;
    if( dirtyBits & DIRTY_REMOVED )
    {
        LBASSERT( !isMaster() ||
                  ( _impl->removedChildren.empty() && dirtyBits == DIRTY_ALL ))
        os << _impl->removedChildren;
        _impl->removedChildren.clear();
    }
    if( (dirtyBits & DIRTY_SERIAL) && isMaster( ))
    {
        _impl->serial = getInstanceID();
        os << _impl->serial;
    }
}

void Object::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        is >> _impl->data.name;
    if( dirtyBits & DIRTY_USERDATA )
    {
        is >> _impl->data.userData;
        // map&sync below to allow early exits
    }
    if( dirtyBits & DIRTY_TASKS )
        is >> _impl->tasks;
    if( dirtyBits & DIRTY_REMOVED )
    {
        std::vector< uint128_t > removed;
        is >> removed;
        if( !removed.empty( ))
        {
            LBASSERT( isMaster( ));
            std::vector< uint128_t >::const_iterator i = removed.begin();
            for( ; i != removed.end(); ++i )
            {
                removeChild( *i );
            }
        }
    }
    if( (dirtyBits & DIRTY_SERIAL) && !isMaster( ))
        is >> _impl->serial;

    if( isMaster( )) // redistribute changes
        setDirty( dirtyBits & getRedistributableBits( ));

    // Update user data state
    if( !(dirtyBits & DIRTY_USERDATA) || !_impl->userData )
        return;

    LBASSERTINFO( _impl->data.userData.identifier != _impl->userData->getID() ||
                  _impl->data.userData.version>=_impl->userData->getVersion() ||
                  _impl->userData->isMaster(),
                  "Incompatible version, new " << _impl->data.userData
                  << " old " << co::ObjectVersion( _impl->userData ));

    if( _impl->data.userData.identifier == 0 )
    {
        if( _impl->userData->isAttached() && !_impl->userData->isMaster( ))
        {
            LBASSERT( !hasMasterUserData( ));
            getLocalNode()->unmapObject( _impl->userData );
        }
        return;
    }

    if( !_impl->userData->isAttached() && _impl->data.userData.identifier != 0 )
    {
        LBASSERT( !hasMasterUserData( ));
        if( !getLocalNode()->mapObject( _impl->userData, _impl->data.userData ))
        {
            LBWARN << "Mapping of " << lunchbox::className( _impl->userData )
                   << " user data failed" << std::endl;
            LBUNREACHABLE;
            return;
        }
    }

    if( !_impl->userData->isMaster() && _impl->userData->isAttached( ))
    {
        LBASSERTINFO( _impl->userData->getID() ==
                      _impl->data.userData.identifier,
                      _impl->userData->getID() << " != "
                      << _impl->data.userData.identifier );
        _impl->userData->sync( _impl->data.userData.version );
    }
}

void Object::setName( const std::string& name )
{
    if( _impl->data.name == name )
        return;
    _impl->data.name = name;
    setDirty( DIRTY_NAME );
}

void Object::setUserData( co::Object* userData )
{
    LBASSERT( !userData || !userData->isAttached() );

    if( _impl->userData == userData )
        return;

    if( _impl->userData && _impl->userData->isAttached( ))
    {
        LBASSERT( _impl->userData->isMaster() == hasMasterUserData( ));
        _impl->userData->getLocalNode()->releaseObject( _impl->userData );
    }

    _impl->userData = userData;

    if( hasMasterUserData( ))
        setDirty( DIRTY_USERDATA );
    else if( _impl->data.userData.identifier != 0 )
    {
        co::LocalNodePtr node = getLocalNode();
        if( node.isValid() )
            node->mapObject( _impl->userData, _impl->data.userData );
    }
}

co::Object* Object::getUserData()
{
    return _impl->userData;
}

const co::Object* Object::getUserData() const
{
    return _impl->userData;
}

uint32_t Object::getTasks() const
{
    return _impl->tasks;
}

uint32_t Object::getSerial() const
{
    return _impl->serial;
}

const std::string& Object::getName() const
{
    return _impl->data.name;
}

void Object::setTasks( const uint32_t tasks )
{
    if( _impl->tasks == tasks )
        return;
    _impl->tasks = tasks;
    setDirty( DIRTY_TASKS );
}

void Object::postRemove( Object* child )
{
    LBASSERT( child );
    if( !child->isAttached( ))
        return;

    LBASSERT( !child->isMaster( ));
    LBASSERT( !isMaster( ));

    _impl->removedChildren.push_back( child->getID( ));
    setDirty( DIRTY_REMOVED );

    co::LocalNodePtr localNode = child->getLocalNode();
    localNode->releaseObject( child );
}

bool Object::_cmdSync( co::ICommand& )
{
    LBASSERT( isMaster( ));
    sync( co::VERSION_HEAD );
    return true;
}

}
}
