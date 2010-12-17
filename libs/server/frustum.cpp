
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "config.h"
#include "frustumData.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace server
{
Frustum::Frustum( FrustumData& data )
        : _data( data )
{
    _updateFrustum();
}

Frustum::Frustum( const Frustum& from, FrustumData& data )
        : fabric::Frustum( from )
        , _data( data )
{
    _updateFrustum();
}

void Frustum::setWall( const fabric::Wall& wall )
{
    fabric::Frustum::setWall( wall );
    _updateFrustum();
}
        
void Frustum::setProjection( const fabric::Projection& projection )
{
    fabric::Frustum::setProjection( projection );
    _updateFrustum();
}

void Frustum::_updateFrustum()
{
    switch( getCurrentType( ))
    {
        case TYPE_WALL:
            _data.applyWall( getWall( ));
            break;
        case TYPE_PROJECTION:
            _data.applyProjection( getProjection( ));
            break;

        case TYPE_NONE:
            _data.invalidate();
            break;
        default:
            EQUNREACHABLE;
    }
}

}
}
