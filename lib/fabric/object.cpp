
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
{}

bool Object::isDirty() const
{
    if( !_userData || _userData->getID() >= EQ_ID_MAX )
        return Serializable::isDirty();

    return Serializable::isDirty() || _userData->isDirty();
}

uint32_t Object::commitNB()
{
    if( _userData )
    {
        if( _userData->getID() == EQ_ID_INVALID && hasMasterUserData())
        {
            getSession()->registerObject( _userData );
            _data.userData.identifier = _userData->getID();
            _data.userData.version = _userData->getVersion();
            setDirty( DIRTY_USERDATA );
        }

        if( _userData->isDirty() && _userData->getID() <= EQ_ID_MAX )
        {
            _data.userData.identifier = _userData->getID();
            _data.userData.version = _userData->commit();
            setDirty( DIRTY_USERDATA );
            EQASSERT( !_userData->isDirty( ));
        }
    }

    return Serializable::commitNB();
}

void Object::notifyDetach()
{
    Serializable::notifyDetach();
    if( !_userData )
        return;

    EQASSERT( _userData->isMaster() == hasMasterUserData( ));

    if( _userData->isMaster( ))
    {
        getSession()->deregisterObject( _userData );
        _data.userData.identifier = EQ_ID_INVALID;
    }
    else
        getSession()->unmapObject( _userData );
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
}

void Object::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        is >> _data.name;
    if( dirtyBits & DIRTY_USERDATA )
    {
        is >> _data.userData;

        if( _userData )
        {
            if( _data.userData.identifier <= EQ_ID_MAX )
            {
                if( _userData->getID() == EQ_ID_INVALID )
                {
                    EQASSERT( !hasMasterUserData( ));
                    getSession()->mapObject( _userData, _data.userData );
                }

                EQASSERTINFO( _userData->getID() == _data.userData.identifier,
                              _userData->getID() << " != " << 
                              _data.userData.identifier );
                if( _userData->isMaster( ))
                    _userData->sync();
                else
                    _userData->sync( _data.userData.version );
            }
            else if( _userData->getID() <= EQ_ID_MAX && !_userData->isMaster( ))
            {
                EQASSERT( !hasMasterUserData( ));
                getSession()->unmapObject( _userData );
            }
        }
    }
    if( dirtyBits & DIRTY_TASKS )
        is >> _tasks;
    if( dirtyBits & DIRTY_ERROR )
        is >> _error;

    if( isMaster( )) // redistribute changes
        setDirty( dirtyBits & ( DIRTY_NAME | DIRTY_USERDATA | DIRTY_ERROR ));
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

    if( _userData && _userData->getID() <= EQ_ID_MAX )
    {
        EQASSERT( _userData->isMaster() == hasMasterUserData( ));
        if( _userData->isMaster( ))
            getSession()->deregisterObject( _userData );
        else
            getSession()->unmapObject( _userData );
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

}
}
