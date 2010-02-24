
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

#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

namespace eq
{
namespace fabric
{

Object::Object()
        : _dirty( DIRTY_NONE )
{}

Object::~Object()
{
}

void Object::getInstanceData( net::DataOStream& os )
{
    os << static_cast< uint64_t >( DIRTY_ALL );
    serialize( os, DIRTY_ALL );
}

void Object::pack( net::DataOStream& os )
{
    if( _dirty == DIRTY_NONE )
        return;

    os << _dirty;
    serialize( os, _dirty );
    _dirty = DIRTY_NONE;
}

void Object::applyInstanceData( net::DataIStream& is )
{
    if( is.getRemainingBufferSize() == 0 && is.nRemainingBuffers() == 0 )
        return;
    
    uint64_t dirty;
    is >> dirty;
    deserialize( is, dirty );
}

void Object::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        os << _name;
}

void Object::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        is >> _name;
}

void Object::setDirty( const uint64_t bits )
{
    _dirty |= bits;
}

void Object::attachToSession( const uint32_t id, const uint32_t instanceID, 
                              net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    _dirty = DIRTY_NONE;
}

void Object::setName( const std::string& name )
{
    _name = name;
    setDirty( DIRTY_NAME );
}

const std::string& Object::getName() const
{
    return _name;
}

}
}
