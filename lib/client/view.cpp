
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
        , _dirty( false )
{
    _data.wall       = view._data.wall;
    _data.projection = view._data.projection;
    _data.current    = view._data.current;
}

View::~View()
{
    _data.current = TYPE_NONE;
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

}
