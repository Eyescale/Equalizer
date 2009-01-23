
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "renderContext.h"

using namespace eq::base;

namespace eq
{
EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const RenderContext& ctx )
{
    os << disableFlush << "ID " << ctx.frameID << " pvp " << ctx.pvp << " vp "
       << ctx.vp << " " << ctx.range << " " << ctx.eye;
    return os;
}
}
