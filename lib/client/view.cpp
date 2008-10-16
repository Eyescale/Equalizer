
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
View::View()
        : _dirty( DIRTY_NONE )
        , _current( TYPE_NONE )
        , _eyeBase( 0.f )
{
}

View::View( net::DataIStream& is )
        : _eyeBase( 0.f )
{
    deserialize( is );
}

View::~View()
{
    _current = TYPE_NONE;
}

void View::getInstanceData( net::DataOStream& os )
{
    os << _current;
    if( _current == TYPE_NONE ) // OPT
        return;

    os << _dirty;
    if( _dirty & DIRTY_WALL )
        os << _wall;
    if( _dirty & DIRTY_PROJECTION )
        os << _projection;
    if( _dirty & DIRTY_EYEBASE )
        os << _eyeBase;
}

void View::applyInstanceData( net::DataIStream& is )
{
    deserialize( is );
}

void View::deserialize( net::DataIStream& is )
{
    is >> _current;
    if( _current == TYPE_NONE ) // OPT
    {
        _dirty = DIRTY_NONE;
        return;
    }

    is >> _dirty;
    if( _dirty & DIRTY_WALL )
        is >> _wall;
    if( _dirty & DIRTY_PROJECTION )
        is >> _projection;
    if( _dirty & DIRTY_EYEBASE )
        is >> _eyeBase;
}

void View::pack( net::DataOStream& os )
{
    if( _dirty == DIRTY_NONE )
        return;

    getInstanceData( os );
    _dirty = DIRTY_NONE;
}

void View::setWall( const Wall& wall )
{
    _wall       = wall;
    // TODO write '= wall' for Projection and update projection here
    _current    = TYPE_WALL;
    _dirty      = DIRTY_WALL;
}

void View::setProjection( const Projection& projection )
{
    _projection = projection;
    _current    = TYPE_PROJECTION;
    _dirty      = DIRTY_PROJECTION;
}

void View::setEyeBase( const float eyeBase )
{
    _eyeBase = eyeBase;
    _dirty   = DIRTY_EYEBASE;
}

std::ostream& operator << ( std::ostream& os, const View& view )
{
    switch( view.getCurrentType( ))
    {
        case View::TYPE_WALL:
            os << view.getWall();
            break;
        case View::TYPE_PROJECTION:
            os << view.getProjection();
            break;
        default: 
            os << "INVALID VIEW";
            break;
    }
    return os;
}
}
