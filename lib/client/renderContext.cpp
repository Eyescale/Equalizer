
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
