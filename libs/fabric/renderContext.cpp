
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "renderContext.h"

namespace eq
{
namespace fabric
{

RenderContext::RenderContext()
        : frustum( Frustumf::DEFAULT )
        , ortho( Frustumf::DEFAULT )
        , headTransform( Matrix4f::IDENTITY )
        , frameID( 0 )
        , overdraw( Vector4i::ZERO )
        , offset( Vector2i::ZERO )
        , buffer( 0x0405 ) // GL_BACK
        , taskID( 0 )
        , period( 1 )
        , phase( 0 )
        , eye( EYE_CYCLOP )
{
}

std::ostream& operator << ( std::ostream& os, const RenderContext& ctx )
{
    os << "ID " << ctx.frameID << " pvp " << ctx.pvp << " vp " << ctx.vp << " "
       << ctx.range << " " << ctx.eye;
    return os;
}

}
}
