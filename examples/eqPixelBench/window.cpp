
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

namespace eqPixelBench
{

bool Window::configInit( const uint32_t initID )
{
    setIAttribute( IATTR_PLANES_ALPHA,   8 ); // enforce visual with alpha
    setIAttribute( IATTR_PLANES_STENCIL, 8 ); // enforce visual with stencil

    return eq::Window::configInit( initID );
}
}

