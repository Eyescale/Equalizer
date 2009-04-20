
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "windowSystem.h"

using namespace eq::base;

namespace eq
{
RenderContext::RenderContext()
        : frameID( 0 )
        , buffer( GL_BACK ) 
        , frustum( vmml::Frustumf::DEFAULT )
        , ortho( vmml::Frustumf::DEFAULT )
        , headTransform( vmml::Matrix4f::IDENTITY )
        , offset( vmml::Vector2i::ZERO )
        , eye( EYE_CYCLOP )
        , overdraw( vmml::Vector4i::ZERO )
{
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const RenderContext& ctx )
{
    os << disableFlush << "ID " << ctx.frameID << " pvp " << ctx.pvp << " vp "
       << ctx.vp << " " << ctx.range << " " << ctx.eye << enableFlush;
    return os;
}
}
