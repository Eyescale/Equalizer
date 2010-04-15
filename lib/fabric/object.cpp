
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
    if( !_userData || _userData->getID() == EQ_ID_INVALID )
        return Serializable::isDirty();

    return Serializable::isDirty() || _userData->isDirty();
}

uint32_t Object::commitNB()
{
    if( _userData && _userData->isDirty() && 
        _userData->getID() != EQ_ID_INVALID )
    {
        _data.userData.identifier = _userData->getID();
        _data.userData.version = _userData->commit();
        setDirty( DIRTY_USERDATA );
        EQASSERT( !_userData->isDirty( ))
    }
    return Serializable::commitNB();
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
            if( _data.userData.identifier == EQ_ID_INVALID )
            {
                if( _userData && !_userData->isMaster() && 
                    _userData->getID() != EQ_ID_INVALID )
                {
                    getSession()->unmapObject( _userData );
                }
            }
            else
            {
                if( _userData->getID() == EQ_ID_INVALID )
                    getSession()->mapObject( _userData,
                                             _data.userData.identifier, 
                                             _data.userData.version );
                
                EQASSERT( !_userData->isMaster( ));
                EQASSERTINFO( _userData->getID() == _data.userData.identifier,
                              _userData->getID() << " != " << 
                              _data.userData.identifier );
                _userData->sync( _data.userData.version );
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
    _userData = userData;
    if( !userData )
        return;

    if( userData->isMaster( ))
    {
        _data.userData.identifier = userData->getID();
        _data.userData.version = userData->getVersion();
    }
    else if( _data.userData.identifier != EQ_ID_INVALID &&
             _userData->getID() == EQ_ID_INVALID )
    {
        net::Session* session = getSession();
        if( session )
            session->mapObject( _userData, _data.userData.identifier, 
                                _data.userData.version );
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
