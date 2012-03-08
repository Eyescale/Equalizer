
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

#include <co/dataOStream.h>
#include <co/dataIStream.h>

namespace eq
{
namespace fabric
{
Frustum::Frustum()
{}

Frustum::~Frustum()
{
    _data.current = TYPE_NONE;
}

void Frustum::backup()
{
    _backup = _data;
}

void Frustum::restore()
{
    _data = _backup;
}

void Frustum::setWall( const Wall& wall )
{
    _data.wall       = wall;
    _data.projection = wall;
    _data.current    = TYPE_WALL;
}
        
void Frustum::setProjection( const Projection& projection )
{
    _data.projection = projection;
    _data.wall       = projection;
    _data.current    = TYPE_PROJECTION;
}

void Frustum::unsetFrustum()
{
    _data.current = TYPE_NONE;
}

void Frustum::serialize( co::DataOStream& os )
{
    switch( getCurrentType( ))
    {
        case TYPE_WALL:
            os << TYPE_WALL << _data.wall;
            break;

        case TYPE_PROJECTION:
            os << TYPE_PROJECTION << _data.projection;
            break;

        case TYPE_NONE:
            os << TYPE_NONE;
            break;

        default:
            EQASSERT( false );
    }
}

void Frustum::deserialize( co::DataIStream& is )
{
    is >> _data.current;

    switch( _data.current )
    {
        case TYPE_WALL:
        {
            is >> _data.wall;
            break;
        }
        case Frustum::TYPE_PROJECTION:
        {
            is >> _data.projection;
            break;
        }
        case Frustum::TYPE_NONE:
            break;

        default:
            EQASSERT( false );
    }
    updateFrustum();
}

std::ostream& operator << ( std::ostream& os, const Frustum& frustum )
{
    switch( frustum.getCurrentType( ))
    {
        case Frustum::TYPE_WALL:
            os << frustum.getWall() << std::endl;
            break;
        case Frustum::TYPE_PROJECTION:
            os << frustum.getProjection() << std::endl;
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
