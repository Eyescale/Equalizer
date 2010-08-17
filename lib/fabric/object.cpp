
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

#include <eq/net/session.h>

namespace eq
{
namespace fabric
{

Object::Object()
        : _userData( 0 )
        , _tasks( TASK_NONE )
{}

Object::~Object()
{
    EQASSERTINFO( !_userData,
                  "Unset user data before destructor to allow clean release" )
    net::Session* session = getSession();
    if( session )
        session->releaseObject( this );
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
            getSession()->registerObject( _userData );
            _data.userData = _userData;
            setDirty( DIRTY_USERDATA );
        }

        if( _userData->isDirty() && _userData->isAttached( ))
        {
            const uint32_t version = _userData->commit();
//            EQINFO << "Committed " << _userData->getID() << " v" << version
//                   << " of " << base::className( _userData ) << " @"
//                   << (void*)_userData << base::backtrace << std::endl;

            EQASSERT( !_userData->isDirty( ));
            EQASSERT( _data.userData.identifier != _userData->getID() ||
                      _data.userData.version <= version );

            if( _userData->isMaster() && _data.userData != _userData )
            {
                EQASSERTINFO( _data.userData.identifier != _userData->getID() ||
                              _data.userData.version < _userData->getVersion(),
                              _data.userData << " >= " <<
                              net::ObjectVersion( _userData ));

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
    {
        _data.userData.identifier = EQ_ID_INVALID;
        _data.userData.version = net::VERSION_NONE;
    }

    getSession()->releaseObject( _userData );
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

void Object::serialize( net::DataOStream& os, const uint64_t dirtyBits )
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

void Object::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
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
        std::vector< uint32_t > removed;
        is >> removed;
        if( !removed.empty( ))
        {
            EQASSERT( isMaster( ));
            for( std::vector< uint32_t >::const_iterator i = removed.begin();
                 i != removed.end(); ++i )
            {
                removeChild( *i );
            }
        }
    }

    if( isMaster( )) // redistribute changes
        setDirty( dirtyBits & ( DIRTY_NAME | DIRTY_USERDATA | DIRTY_ERROR ));

    // Update user data state
    if( !(dirtyBits & DIRTY_USERDATA) ||!_userData )
        return;

    EQASSERTINFO( _data.userData.identifier != _userData->getID() ||
                  _data.userData.version >= _userData->getVersion() ||
                  _userData->isMaster(),
                  "Incompatible version, new " << _data.userData << " old " <<
                  net::ObjectVersion( _userData ));

    if( _data.userData.identifier > EQ_ID_MAX )
    {
        if( _userData->isAttached() && !_userData->isMaster( ))
        {
            EQASSERT( !hasMasterUserData( ));
            getSession()->unmapObject( _userData );
        }
        return;
    }

    if( !_userData->isAttached( ))
    {
        EQASSERT( !hasMasterUserData( ));
        //EQINFO << "Map " << _data.userData << base::backtrace << std::endl;
        if( !getSession()->mapObject( _userData, _data.userData ))
        {
            EQWARN << "Mapping of " << base::className( _userData )
                   << " user data failed" << std::endl;
            return;
        }
    }
    EQASSERTINFO( _userData->getID() == _data.userData.identifier,
                  _userData->getID() << " != " << _data.userData.identifier );

    if( !_userData->isMaster( ))
    {
#if 0
        if( _userData->getVersion() < _data.userData.version )
            EQINFO << "Sync " << _data.userData << base::backtrace
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

void Object::setUserData( net::Object* userData )
{
    EQASSERT( !userData || userData->getID() == EQ_ID_INVALID );

    if( _userData == userData )
        return;

    if( _userData && _userData->isAttached( ))
    {
        net::Session* session = _userData->getSession();
        EQASSERT( _userData->isMaster() == hasMasterUserData( ));
        session->releaseObject( _userData );
    }

    _userData = userData;

    if( hasMasterUserData( ))
        setDirty( DIRTY_USERDATA );
    else if( _data.userData.identifier <= EQ_ID_MAX )
    {
        net::Session* session = getSession();
        if( session )
            session->mapObject( _userData, _data.userData );
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

void Object::setErrorMessage( const std::string& message )
{
    if( _error == message )
        return;
    _error = message;
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
