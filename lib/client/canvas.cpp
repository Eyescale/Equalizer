
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "canvas.h"

#include "canvasVisitor.h"
#include "config.h"
#include "layout.h"

namespace eq
{

Canvas::Canvas()
        : _config( 0 )
        , _layout( 0 )
{}

Canvas::~Canvas()
{
    EQASSERT( !_config );
    
    _config  = 0;
    _layout  = 0;
}

void Canvas::serialize( net::DataOStream& os, const uint32_t dirtyBits )
{
    if( dirtyBits & DIRTY_LAYOUT )
        EQUNIMPLEMENTED;

    Frustum::serialize( os, dirtyBits );
}

void Canvas::deserialize( net::DataIStream& is, const uint32_t dirtyBits )
{
    if( _dirty & DIRTY_LAYOUT )
        EQUNIMPLEMENTED;

    Frustum::deserialize( is, dirtyBits );
}

void Canvas::useLayout( Layout* layout )
{
    _layout = layout;
    _dirty |= DIRTY_LAYOUT;
}

VisitorResult Canvas::accept( CanvasVisitor* visitor )
{ 
    return visitor->visit( this );
}

}
