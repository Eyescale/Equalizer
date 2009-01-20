
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frustum.h"

#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

namespace eq
{

Frustum::Frustum()
        :  _dirty( DIRTY_NONE )
        , _current( TYPE_NONE )
{}

Frustum::~Frustum()
{
    _current = TYPE_NONE;
}

void Frustum::getInstanceData( net::DataOStream& os )
{
    os << static_cast< uint32_t >( DIRTY_ALL );
    serialize( os, DIRTY_ALL );
}

void Frustum::pack( net::DataOStream& os )
{
    if( _dirty == DIRTY_NONE )
        return;

    os << _dirty;
    serialize( os, _dirty );
    _dirty = DIRTY_NONE;
}

void Frustum::applyInstanceData( net::DataIStream& is )
{
    if( is.getRemainingBufferSize() == 0 && is.nRemainingBuffers() == 0 )
        return;
    
    is >> _dirty;
    deserialize( is, _dirty );
}

void Frustum::serialize( net::DataOStream& os, const uint32_t dirtyBits )
{
    if( dirtyBits & DIRTY_WALL )
        os << _current << _wall;
    if( dirtyBits & DIRTY_PROJECTION )
        os << _current << _projection;
}

void Frustum::deserialize( net::DataIStream& is, const uint32_t dirtyBits )
{
    if( dirtyBits & DIRTY_WALL )
        is >> _current >> _wall;
    if( dirtyBits & DIRTY_PROJECTION )
        is >> _current >> _projection;
}

void Frustum::setWall( const Wall& wall )
{
    _wall       = wall;
    // TODO write '= wall' for Projection and update projection here
    _current    = TYPE_WALL;
    _dirty     |= DIRTY_WALL;
}
        
void Frustum::setProjection( const Projection& projection )
{
    _projection = projection;
    _current    = TYPE_PROJECTION;
    _dirty     |= DIRTY_PROJECTION;
}


std::ostream& operator << ( std::ostream& os, const Frustum& frustum )
{
    switch( frustum.getCurrentFrustum( ))
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
