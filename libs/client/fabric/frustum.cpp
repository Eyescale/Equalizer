
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

co::DataOStream& operator << ( co::DataOStream& os, const Frustum& frustum )
{
    switch( frustum.getCurrentType( ))
    {
        case Frustum::TYPE_WALL:
            os << Frustum::TYPE_WALL << frustum.getWall();
            break;

        case Frustum::TYPE_PROJECTION:
            os << Frustum::TYPE_PROJECTION << frustum.getProjection();
            break;

        case Frustum::TYPE_NONE:
            os << Frustum::TYPE_NONE;
            break;

        default:
            EQASSERT( false );
    }
    return os;
}

co::DataIStream& operator >> ( co::DataIStream& is, Frustum& frustum )
{
    Frustum::Type type;
    is >> type;

    switch( type )
    {
        case Frustum::TYPE_WALL:
        {
            Wall wall;
            is >> wall;
            frustum.setWall( wall );
            break;
        }
        case Frustum::TYPE_PROJECTION:
        {
            Projection projection;
            is >> projection;
            frustum.setProjection( projection );
            break;
        }
        case Frustum::TYPE_NONE:
            frustum.unsetFrustum();
            break;

        default:
            EQASSERT( false );
    }
    return is;
}

}
}
