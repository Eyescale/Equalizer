
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "renderContext.h"

EQ_EXPORT std::ostream& eq::operator << ( std::ostream& os, const RenderContext& ctx )
{
    os << "pvp " << ctx.pvp << " vp " << ctx.vp << " " << ctx.range << ctx.eye
       << " frustum " << ctx.frustum;
    return os;
}
