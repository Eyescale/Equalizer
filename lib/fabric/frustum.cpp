
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

#include "frustum.h"

#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

namespace eq
{
namespace fabric
{
Frustum::Frustum()
        : _current( TYPE_NONE )
{}

Frustum::~Frustum()
{
    _current = TYPE_NONE;
}

void Frustum::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_TYPE )
        os << _current;
    if( dirtyBits & DIRTY_WALL )
        os << _wall;
    if( dirtyBits & DIRTY_PROJECTION )
        os << _projection;
}

void Frustum::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_TYPE )
        is >> _current;
    if( dirtyBits & DIRTY_WALL )
        is >> _wall;
    if( dirtyBits & DIRTY_PROJECTION )
        is >> _projection;
}

void Frustum::setWall( const Wall& wall )
{
    _wall       = wall;
    _projection = wall;
    _current    = TYPE_WALL;
    setDirty( DIRTY_TYPE | DIRTY_WALL );
}
        
void Frustum::setProjection( const Projection& projection )
{
    _projection = projection;
    _wall       = projection;
    _current    = TYPE_PROJECTION;
    setDirty( DIRTY_TYPE | DIRTY_PROJECTION );
}

void Frustum::unsetFrustum()
{
    _current = TYPE_NONE;
    setDirty( DIRTY_TYPE );
}

std::ostream& operator << ( std::ostream& os, const Frustum& frustum )
{
    switch( frustum.getCurrentType( ))
    {
        case Frustum::TYPE_WALL:
            os << frustum.getWall();
            break;
        case Frustum::TYPE_PROJECTION:
            os << frustum.getProjection();
            break;
        case Frustum::TYPE_NONE:
            break;
        default:
            os << "INVALID FRUSTUM";
            break;
    }
    return os;
}

}
}
