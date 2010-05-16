
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#include "segment.h"

#include "leafVisitor.h"
#include "paths.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace fabric
{

template< class C, class S, class CH >
Segment< C, S, CH >::Segment( C* canvas )
        : _canvas( canvas )
        , _channel( 0 )
{
    EQASSERT( canvas );
    canvas->_addSegment( static_cast< S* >( this ));
}

template< class C, class S, class CH >
Segment< C, S, CH >::~Segment()
{
    _canvas->_removeSegment( static_cast< S* >( this ));
    _channel = 0;
}

template< class C, class S, class CH >
void Segment< C, S, CH >::serialize( net::DataOStream& os,
                                 const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _vp;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << *static_cast< Frustum* >( this );
    if( dirtyBits & DIRTY_CHANNEL )
        os << _channel;
}

template< class C, class S, class CH >
void Segment< C, S, CH >::deserialize( net::DataIStream& is,
                                   const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _vp;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> *static_cast< Frustum* >( this );
    if( dirtyBits & DIRTY_CHANNEL )
    {
        EQASSERT( _canvas->getConfig()->mapViewObjects( ))

        net::ObjectVersion ov;
        is >> ov;
        if( ov.identifier == EQ_ID_NONE )
            _channel = 0;
        else
        {
            _canvas->getConfig()->find( ov.identifier, &_channel );
            EQASSERT( _channel );
        }
    }
}

template< class C, class S, class CH >
VisitorResult Segment< C, S, CH >::accept( Visitor& visitor )
{
    return visitor.visit( static_cast< S* >( this ));
}

template< class C, class S, class CH >
VisitorResult Segment< C, S, CH >::accept( Visitor& visitor ) const
{
    return visitor.visit( static_cast< const S* >( this ));
}

template< class C, class S, class CH > void Segment< C, S, CH >::backup()
{
    Frustum::backup();
    Object::backup();
}

template< class C, class S, class CH > void Segment< C, S, CH >::restore()
{
    Object::restore();
    Frustum::restore();
}

template< class C, class S, class CH >
void Segment< C, S, CH >::setViewport( const Viewport& vp ) 
{
    if( _vp == vp )
        return;

    _vp = vp; 
    setDirty( DIRTY_VIEWPORT );

    if( getCurrentType() != TYPE_NONE )
        return;

    // if segment has no frustum...
    Wall wall( _canvas->getWall( ));
    wall.apply( vp );
                    
    switch( _canvas->getCurrentType( ))
    {
        case Frustum::TYPE_WALL:
            setWall( wall );
            break;

        case Frustum::TYPE_PROJECTION:
        {
            Projection projection( _canvas->getProjection( )); // keep distance
            projection = wall;
            setProjection( projection );
            break;
        }
        default: 
            EQUNIMPLEMENTED;
        case Frustum::TYPE_NONE:
            break; 
    }
}

template< class C, class S, class CH >
void Segment< C, S, CH >::setWall( const Wall& wall )
{
    if( getWall() == wall && getCurrentType() == TYPE_WALL )
        return;

    Frustum::setWall( wall );
    setDirty( DIRTY_FRUSTUM );
}

template< class C, class S, class CH >
void Segment< C, S, CH >::setProjection( const Projection& projection )
{
    if( getProjection() == projection && getCurrentType() == TYPE_PROJECTION )
        return;

    Frustum::setProjection( projection );
    setDirty( DIRTY_FRUSTUM );
}

template< class C, class S, class CH >
void Segment< C, S, CH >::unsetFrustum()
{
    if( getCurrentType() == TYPE_NONE )
        return;

    Frustum::unsetFrustum();
    setDirty( DIRTY_FRUSTUM );
}

template< class C, class S, class CH >
std::ostream& operator << ( std::ostream& os, const Segment< C, S, CH >& s )
{
    const S& segment = static_cast< const S& >( s );
    os << base::disableFlush << base::disableHeader << "segment" << std::endl;
    os << "{" << std::endl << base::indent;
    
    const std::string& name = segment.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    if( segment.getChannel( ))
    {
        const std::string& channelName = segment.getChannel()->getName();
        if( segment.getConfig()->findChannel( channelName ) ==
            segment.getChannel( ))
        {
            os << "channel  \"" << channelName << "\"" << std::endl;
        }
        else
            os << "channel  " << segment.getChannel()->getPath() << std::endl;
    }

    const Viewport& vp  = segment.getViewport();
    if( vp.isValid( ) && vp != Viewport::FULL )
        os << "viewport " << vp << std::endl;

    os << static_cast< const Frustum& >( segment );

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}


}
}
