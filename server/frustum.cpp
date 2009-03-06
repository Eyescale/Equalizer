
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frustum.h"

#include "config.h"
#include "frustumData.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

using namespace eq::base;

namespace eq
{
namespace server
{
Frustum::Frustum( FrustumData& data )
        : _data( data )
{
    _updateFrustum();
}

Frustum::Frustum( const Frustum& from, FrustumData& data )
        : eq::Frustum( from )
        , _data( data )
{
    _updateFrustum();
}

void Frustum::setWall( const eq::Wall& wall )
{
    eq::Frustum::setWall( wall );
    _updateFrustum();
}
        
void Frustum::setProjection( const eq::Projection& projection )
{
    eq::Frustum::setProjection( projection );
    _updateFrustum();
}

void Frustum::_updateFrustum()
{
    switch( getCurrentType( ))
    {
        case TYPE_WALL:
            _data.applyWall( getWall( ));
            break;
        case TYPE_PROJECTION:
            _data.applyProjection( getProjection( ));
            break;

        case TYPE_NONE:
            _data.invalidate();
            break;
        default:
            EQUNREACHABLE;
    }
}

}
}
