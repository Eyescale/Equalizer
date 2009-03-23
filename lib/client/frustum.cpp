
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frustum.h"

#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

namespace eq
{

Frustum::Frustum()
        : _current( TYPE_NONE )
{}

Frustum::~Frustum()
{
    _current = TYPE_NONE;
}

void Frustum::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_TYPE )
        os << _current;
    if( dirtyBits & DIRTY_WALL )
        os << _wall;
    if( dirtyBits & DIRTY_PROJECTION )
        os << _projection;
}

void Frustum::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_TYPE )
        is >> _current;
    if( dirtyBits & DIRTY_WALL )
        is >> _wall;
    if( dirtyBits & DIRTY_PROJECTION )
        is >> _projection;
}

void Frustum::setWall( const Wall& wall )
{
    _wall       = wall;
    _projection = wall;
    _current    = TYPE_WALL;
    setDirty( DIRTY_TYPE | DIRTY_WALL );
}
        
void Frustum::setProjection( const Projection& projection )
{
    _projection = projection;
    _wall       = projection;
    _current    = TYPE_PROJECTION;
    setDirty( DIRTY_TYPE | DIRTY_PROJECTION );
}

void Frustum::unsetFrustum()
{
    _current = TYPE_NONE;
    setDirty( DIRTY_TYPE );
}

std::ostream& operator << ( std::ostream& os, const Frustum& frustum )
{
    switch( frustum.getCurrentType( ))
    {
        case Frustum::TYPE_WALL:
            os << frustum.getWall();
            break;
        case Frustum::TYPE_PROJECTION:
            os << frustum.getProjection();
            break;
        case Frustum::TYPE_NONE:
            break;
        default:
            os << "INVALID FRUSTUM";
            break;
    }
    return os;
}
}
