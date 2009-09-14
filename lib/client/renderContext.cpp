
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "os.h"

using namespace eq::base;

namespace eq
{
RenderContext::RenderContext()
        : frameID( 0 )
        , buffer( GL_BACK ) 
        , frustum( Frustumf::DEFAULT )
        , ortho( Frustumf::DEFAULT )
        , headTransform( Matrix4f::IDENTITY )
        , offset( Vector2i::ZERO )
        , eye( EYE_CYCLOP )
        , overdraw( Vector4i::ZERO )
        , taskID( 0 )
{
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const RenderContext& ctx )
{
    os << "ID " << ctx.frameID << " pvp " << ctx.pvp << " vp " << ctx.vp << " "
       << ctx.range << " " << ctx.eye;
    return os;
}
}
