
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#include "log.h"
#include "paths.h"
#include "pipePackets.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{

template< class C, class S, class CH >
Segment< C, S, CH >::Segment( C* canvas )
        : _canvas( canvas )
        , _channel( 0 )
        , _eyes( EYES_ALL )
{
    EQASSERT( canvas );
    canvas->_addChild( static_cast< S* >( this ));
    EQLOG( LOG_INIT ) << "New " << co::base::className( this ) << std::endl;
}

template< class C, class S, class CH >
Segment< C, S, CH >::~Segment()
{
    EQLOG( LOG_INIT ) << "Delete " << co::base::className( this ) << std::endl;
    _canvas->_removeChild( static_cast< S* >( this ));
    _channel = 0;
}

template< class C, class S, class CH >
uint32_t Segment< C, S, CH >::commitNB()
{
    if( Serializable::isDirty( DIRTY_CHANNEL ) && _channel )
        commitChild< typename CH::Parent, PipeNewWindowPacket, Object >
            ( _channel->getWindow(), _channel->getPipe( ));
    return Object::commitNB();
}

template< class C, class S, class CH >
void Segment< C, S, CH >::setEyes( const uint32_t eyes ) 
{ 
    if( _eyes == eyes )
        return;
    setDirty( DIRTY_EYES );
    _eyes = eyes; 
}
        
template< class C, class S, class CH > void 
Segment< C, S, CH >::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _vp;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << *static_cast< Frustum* >( this );
    if( dirtyBits & DIRTY_CHANNEL )
        os << co::ObjectVersion( _channel );
    if( dirtyBits & DIRTY_EYES )
        os << _eyes;
}

template< class C, class S, class CH >
void Segment< C, S, CH >::deserialize( co::DataIStream& is,
                                       const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _vp;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> *static_cast< Frustum* >( this );
    if( dirtyBits & DIRTY_CHANNEL )
    {
        EQASSERT( _canvas->_mapViewObjects( ))

        co::ObjectVersion ov;
        is >> ov;

        _channel = 0;
        if( ov.identifier != co::base::UUID::ZERO )
        {
            _canvas->getConfig()->find( ov.identifier, &_channel );
            EQASSERT( !isMaster() || _channel );
        }
    }
    if( dirtyBits & DIRTY_EYES )
        is >> _eyes; 
}

template< class C, class S, class CH >
void Segment< C, S, CH >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _canvas->setDirty( C::DIRTY_SEGMENTS );
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
    notifyFrustumChanged();
}

template< class C, class S, class CH >
void Segment< C, S, CH >::notifyFrustumChanged()
{
    if( getCurrentType() != TYPE_NONE )
        return;

    // if segment has no frustum...
    Wall wall( _canvas->getWall( ));
    wall.apply( _vp );
                    
    switch( _canvas->getCurrentType( ))
    {
        case TYPE_WALL:
            setWall( wall );
            break;

        case TYPE_PROJECTION:
        {
            Projection projection( _canvas->getProjection( )); // keep distance
            projection = wall;
            setProjection( projection );
            break;
        }
        default: 
            EQUNIMPLEMENTED;
        case TYPE_NONE:
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
    os << co::base::disableFlush << co::base::disableHeader << "segment"
       << std::endl;
    os << "{" << std::endl << co::base::indent;
    
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
    
    const uint32_t eyes = segment.getEyes();
    if( eyes != EYES_ALL )
    {
        os << "eye      [ ";
        if( eyes & fabric::EYE_CYCLOP )
            os << "CYCLOP ";
        if( eyes & fabric::EYE_LEFT )
            os << "LEFT ";
        if( eyes & fabric::EYE_RIGHT )
            os << "RIGHT ";
        os << "]" << std::endl;
    }
    const Viewport& vp  = segment.getViewport();
    if( vp.isValid( ) && vp != Viewport::FULL )
        os << "viewport " << vp << std::endl;

    os << static_cast< const Frustum& >( segment );

    os << co::base::exdent << "}" << std::endl << co::base::enableHeader
       << co::base::enableFlush;
    return os;
}


}
}
