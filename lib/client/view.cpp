
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "view.h"

#include "config.h"
#include "event.h"
#include "pipe.h"
#include "layout.h"
#include "observer.h"
#include "viewVisitor.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
View::View()
        : _layout( 0 )
        , _pipe( 0 )
        , _observer( 0 )
        , _overdraw( Vector2i::ZERO )
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
    if( dirtyBits & DIRTY_OBSERVER )
        os << ( _observer ? _observer->getID() : EQ_ID_INVALID );
    if( dirtyBits & DIRTY_OVERDRAW )
        os << _overdraw;
}

void View::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _viewport;
    if( dirtyBits & DIRTY_OBSERVER )
    {
        uint32_t id;
        is >> id;

        if( id == EQ_ID_INVALID )
            _observer = 0;
        else
        {
            Config* config = getConfig();
            EQASSERT( config );
            _observer = config->findObserver( id );
        }
    }
    if( dirtyBits & DIRTY_OVERDRAW )
        is >> _overdraw;

    if( dirtyBits == ( DIRTY_CUSTOM - 1 ))
        _baseFrustum = *this; // save baseline data for resizing
}

Config* View::getConfig()
{
    EQASSERT( _layout || _pipe );

    if( _layout )
    {
        EQASSERT( !_pipe );
        return _layout->getConfig();
    }

    if( _pipe )
    {
        EQASSERT( !_layout );
        return _pipe->getConfig();
    }

    return 0;
}

const Config* View::getConfig() const
{
    EQASSERT( _layout || _pipe );

    if( _layout )
    {
        EQASSERT( !_pipe );
        return _layout->getConfig();
    }

    if( _pipe )
    {
        EQASSERT( !_layout );
        return _pipe->getConfig();
    }

    return 0;
}

const Viewport& View::getViewport() const
{
    return _viewport;
}

void View::setOverdraw( const Vector2i& pixels )
{
    _overdraw = pixels;
    setDirty( DIRTY_OVERDRAW );
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
            if( resize.dw == 0.f || resize.dh == 0.f )
                return true;

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
