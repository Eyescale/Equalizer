
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "renderContext.h"

std::ostream& eq::operator << ( std::ostream& os, const RenderContext& ctx )
{
    os << "pvp " << ctx.pvp << " range " << ctx.range;
    return os;
}
