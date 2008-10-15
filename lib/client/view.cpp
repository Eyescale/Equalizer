
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
View::View()
        : _dirty( false )
{
    _data.current = TYPE_NONE;
}

View::View( const View& view )
        : net::Object()
        , _data( view._data )
        , _dirty( false )
{
}

View::View( const Type& last, const Wall& wall, const Projection& projection )
{
    _data.wall       = wall;
    _data.projection = projection;
    _data.current    = last;
}

View::~View()
{
    _data.current = TYPE_NONE;
}

View& View::operator = ( const View& view )
{
    _data  = view._data;
    _dirty = true;
    return *this;
}

void View::getInstanceData( net::DataOStream& os )
{
    os.writeOnce( &_data, sizeof( _data )); 
}

void View::applyInstanceData( net::DataIStream& is )
{
    EQASSERT( is.getRemainingBufferSize() == sizeof( _data )); 

    memcpy( &_data, is.getRemainingBuffer(), sizeof( _data ));
    is.advanceBuffer( sizeof( _data ));

    EQASSERT( is.nRemainingBuffers() == 0 );
    EQASSERT( is.getRemainingBufferSize() == 0 );
}

void View::pack( net::DataOStream& os )
{
    if( !_dirty )
        return;

    getInstanceData( os );
    _dirty = false;
}

void View::setWall( const Wall& wall )
{
    _data.wall       = wall;
    // TODO write '= wall' for Projection and update projection here
    _data.current    = TYPE_WALL;
    _dirty = true;
}

void View::setProjection( const Projection& projection )
{
    _data.projection = projection;
    _data.current    = TYPE_PROJECTION;

    _dirty = true;
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
