
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "canvas.h"

#include "canvasVisitor.h"
#include "config.h"
//#include "layout.h"

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

void Canvas::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
        EQUNIMPLEMENTED;
}

void Canvas::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
        EQUNIMPLEMENTED;
}

void Canvas::useLayout( Layout* layout )
{
    _layout = layout;
    setDirty( DIRTY_LAYOUT );
}

VisitorResult Canvas::accept( CanvasVisitor* visitor )
{ 
    return visitor->visit( this );
}

}
