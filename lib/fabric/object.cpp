
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

#include <eq/net/session.h>

namespace eq
{
namespace fabric
{

Object::Object()
        : _userData( 0 )
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
        _userDataVersion.identifier = _userData->getID();
        _userDataVersion.version = _userData->commit();
        setDirty( DIRTY_USERDATA );
        EQASSERT( !_userData->isDirty( ))
    }
    return Serializable::commitNB();
}

void Object::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        os << _name;
    if( dirtyBits & DIRTY_USERDATA )
        os << _userDataVersion;
}

void Object::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        is >> _name;
    if( dirtyBits & DIRTY_USERDATA )
    {
        is >> _userDataVersion;

        if( _userData )
        {
            if( _userDataVersion.identifier == EQ_ID_INVALID )
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
                                             _userDataVersion.identifier, 
                                             _userDataVersion.version );
                
                EQASSERT( !_userData->isMaster( ));
                EQASSERTINFO( _userData->getID() == _userDataVersion.identifier,
                              _userData->getID() << " != " << 
                              _userDataVersion.identifier );
                _userData->sync( _userDataVersion.version );
            }
        }
    }
    if( isMaster( )) // redistribute changes
        setDirty( dirtyBits & ( DIRTY_NAME | DIRTY_USERDATA ));
}

void Object::setName( const std::string& name )
{
    if( _name == name )
        return;
    _name = name;
    setDirty( DIRTY_NAME );
}

void Object::setUserData( net::Object* userData )
{
    _userData = userData;
    if( !userData )
        return;

    if( userData->isMaster( ))
    {
        _userDataVersion.identifier = userData->getID();
        _userDataVersion.version = userData->getVersion();
    }
    else if( _userDataVersion.identifier != EQ_ID_INVALID &&
             _userData->getID() == EQ_ID_INVALID )
    {
        net::Session* session = getSession();
        if( session )
            session->mapObject( _userData, _userDataVersion.identifier, 
                                _userDataVersion.version );
    }
}

const std::string& Object::getName() const
{
    return _name;
}

}
}
