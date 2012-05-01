
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include "serializable.h"

#include "dataIStream.h"
#include "dataOStream.h"

namespace co
{
namespace detail 
{
class Serializable
{
public:
    Serializable() : dirty( co::Serializable::DIRTY_NONE ) {}
    ~Serializable() {}

    /** The current dirty bits. */
    uint64_t dirty;
};
}

Serializable::Serializable()
        : _impl( new detail::Serializable )
{}

Serializable::Serializable( const Serializable& )
        : co::Object()
        , _impl( new detail::Serializable )
{}

Serializable::~Serializable() 
{
    delete _impl;
}

uint64_t Serializable::getDirty() const
{
    return _impl->dirty;
}

bool Serializable::isDirty() const
{
    return ( _impl->dirty != DIRTY_NONE );
}

bool Serializable::isDirty( const uint64_t dirtyBits ) const
{
    return (_impl->dirty & dirtyBits) == dirtyBits;
}

uint128_t Serializable::commit( const uint32_t incarnation )
{
    const uint128_t& version = co::Object::commit( incarnation );
    _impl->dirty = DIRTY_NONE;
    return version;
}

void Serializable::setDirty( const uint64_t bits )
{
    _impl->dirty |= bits;
}

void Serializable::unsetDirty( const uint64_t bits )
{
    _impl->dirty &= ~bits;
}

void Serializable::notifyAttached()
{
    if( isMaster( ))
        _impl->dirty = DIRTY_NONE;
}

void Serializable::applyInstanceData( co::DataIStream& is )
{
    if( !is.hasData( ))
        return;
    deserialize( is, DIRTY_ALL );
}

void Serializable::pack( co::DataOStream& os )
{
    if( _impl->dirty == DIRTY_NONE )
        return;

    os << _impl->dirty;
    serialize( os, _impl->dirty );
}

void Serializable::unpack( co::DataIStream& is )
{
    LBASSERT( is.hasData( ));
    if( !is.hasData( ))
        return;

    uint64_t dirty;
    is >> dirty;
    deserialize( is, dirty );
}

}
