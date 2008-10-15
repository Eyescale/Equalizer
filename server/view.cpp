
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

#include "viewData.h"

namespace eq
{
namespace server
{

View::View( ViewData& data )
        : _data( data )
{
    _updateData();
}

View::View( const View& from, ViewData& data )
        : eq::View( from )
        , _data( data )
{
    _updateData();
}

void View::setWall( const eq::Wall& wall )
{
    eq::View::setWall( wall );
    _updateData();
}
        
void View::setProjection( const eq::Projection& projection )
{
    eq::View::setProjection( projection );
    _updateData();
}

void View::applyInstanceData( net::DataIStream& is )
{
    eq::View::applyInstanceData( is );
    _updateData();
}

void View::_updateData()
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
