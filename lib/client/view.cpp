
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

#include "channel.h"
#include "layout.h"
#include "viewVisitor.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
View::View()
        : _layout( 0 )
        , _channel( 0 )
{
}

View::~View()
{
    EQASSERT( !_layout );
    _layout = 0;
}

void View::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _viewport;
}

void View::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _viewport;

    if( dirtyBits & DIRTY_ALL )
        _baseFrustum = *this; // save baseline data for resizing
}

Config* View::getConfig()
{
    EQASSERT( _layout || _channel );

    if( _layout )
    {
        EQASSERT( !_channel );
        return _layout->getConfig();
    }

    if( _channel )
    {
        EQASSERT( !_layout );
        return _channel->getConfig();
    }

    return 0;
}

const Config* View::getConfig() const
{
    EQASSERT( _layout || _channel );

    if( _layout )
    {
        EQASSERT( !_channel );
        return _layout->getConfig();
    }

    if( _channel )
    {
        EQASSERT( !_layout );
        return _channel->getConfig();
    }

    return 0;
}

const Viewport& View::getViewport() const
{
    return _viewport;
}

VisitorResult View::accept( ViewVisitor& visitor )
{
    return visitor.visit( this );
}

bool View::handleEvent( const Event& event )
{
    switch( event.type )
    {
        case Event::VIEW_RESIZE:
        {
            const ResizeEvent& resize = event.resize;

            switch( getCurrentType( ))
            {
                case TYPE_WALL:
                {
                    const float ratio( resize.dw / resize.dh );
                    Wall wall( _baseFrustum.getWall( ));

                    wall.resizeHorizontal( ratio );
                    setWall( wall );
                    break;
                }

                case View::TYPE_PROJECTION:
                {
                    const float ratio( resize.dw / resize.dh );
                    eq::Projection projection( _baseFrustum.getProjection( ));

                    projection.resizeHorizontal( ratio );
                    setProjection( projection );
                    break;
                }

                case eq::View::TYPE_NONE:
                    break;
                default:
                    EQUNIMPLEMENTED;
                    break;
            }

            return true;
        }
    }
    
    return false;
}


}
