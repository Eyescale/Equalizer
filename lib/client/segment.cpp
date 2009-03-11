
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "segment.h"

#include "canvas.h"
#include "config.h"
#include "segmentVisitor.h"

using namespace eq::base;

namespace eq
{

Segment::Segment()
        : _canvas( 0 )
{
}

Segment::~Segment()
{
    EQASSERT( _canvas == 0 );
}

void Segment::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_VIEWPORT )
        os << _vp;
}

void Segment::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _vp;
}

Config* Segment::getConfig()
{
    EQASSERT( _canvas );
    return _canvas ? _canvas->getConfig() : 0;
}

const Config* Segment::getConfig() const
{
    EQASSERT( _canvas );
    return _canvas ? _canvas->getConfig() : 0;
}

VisitorResult Segment::accept( SegmentVisitor& visitor )
{
    return visitor.visit( this );
}

}
