
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "channel.h"
#include "leafVisitor.h"
#include "task.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace fabric
{
namespace
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_CHANNEL_") + #attr )
static std::string _iAttributeStrings[] = {
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_ATTR_STRING( IATTR_HINT_SENDTOKEN ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};
}

template< class W, class C >
Channel< W, C >::Channel( W* parent )
        : _window( parent )
        , _context( &_nativeContext )
        , _color( Vector3ub::ZERO )
        , _drawable( FB_WINDOW )
        , _maxSize( Vector2i::ZERO )
        , _fixedVP( true )
{
    parent->_addChannel( static_cast< C* >( this ));
    notifyViewportChanged();

    uint32_t value = (reinterpret_cast< size_t >( this ) & 0xffffffffu);
    for( unsigned i=0; i<8; ++i )
    {
        _color.r() |= ( value&1 << (7-i) ); value >>= 1;
        _color.g() |= ( value&1 << (7-i) ); value >>= 1;
        _color.b() |= ( value&1 << (7-i) ); value >>= 1;
    }
}

template< class W, class C >
Channel< W, C >::Channel( const Channel& from, W* parent )
        : Entity( from )
        , _window( parent )
        , _nativeContext( from._nativeContext )
        , _context( &_nativeContext )
        , _color( from._color )
        , _drawable( from._drawable )
        , _maxSize( from._maxSize )
        , _fixedVP( from._fixedVP )
{
    parent->_addChannel( static_cast< C* >( this ));

    for( int i = 0; i < IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

    notifyViewportChanged();
}

template< class W, class C >
Channel< W, C >::~Channel()
{  
    _window->_removeChannel( static_cast< C* >( this ));
}

template< class W, class C >
VisitorResult Channel< W, C >::accept( Visitor& visitor )
{
    return visitor.visit( static_cast< C* >( this ));
}

template< class W, class C >
VisitorResult Channel< W, C >::accept( Visitor& visitor ) const
{
    return visitor.visit( static_cast< const C* >( this ));
}

template< class W, class C >
void Channel< W, C >::serialize( net::DataOStream& os, const uint64_t dirtyBits)
{
    Entity::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os.write( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _nativeContext.vp << _nativeContext.pvp << _fixedVP << _maxSize;
    if( dirtyBits & DIRTY_MEMBER )
        os << _drawable << _color << _nativeContext.view
           << _nativeContext.overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << _nativeContext.frustum;
    if( dirtyBits & DIRTY_ALL )
        os << _color;
}

template< class W, class C >
void Channel< W, C >::deserialize( net::DataIStream& is,
                                   const uint64_t dirtyBits )
{
    Entity::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is.read( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_VIEWPORT )
    {
        is >> _nativeContext.vp >> _nativeContext.pvp >> _fixedVP >> _maxSize;
        notifyViewportChanged();
    }
    if( dirtyBits & DIRTY_MEMBER )
        is >> _drawable >> _color >> _nativeContext.view
           >> _nativeContext.overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> _nativeContext.frustum;
    if( dirtyBits & DIRTY_ALL )
        is >> _color;
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
template< class W, class C >
void Channel< W, C >::setPixelViewport( const PixelViewport& pvp )
{
    EQASSERT( pvp.hasArea( ));
    if( !pvp.hasArea( ))
        return;

    _fixedVP = false;

    if( _nativeContext.pvp == pvp && _nativeContext.vp.hasArea( ))
        return;

    _nativeContext.pvp = pvp;
    _nativeContext.vp.invalidate();

    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
}

template< class W, class C >
void Channel< W, C >::setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _fixedVP = true;

    if( _nativeContext.vp == vp && _nativeContext.pvp.hasArea( ))
        return;

    _nativeContext.vp = vp;
    _nativeContext.pvp.invalidate();

    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
}

template< class W, class C >
void Channel< W, C >::notifyViewportChanged()
{
    if( !_window )
        return;

    eq::PixelViewport windowPVP = _window->getPixelViewport();
    if( !windowPVP.isValid( ))
        return;

    windowPVP.x = 0;
    windowPVP.y = 0;

    if( _fixedVP ) // update pixel viewport
    {
        const PixelViewport oldPVP = _nativeContext.pvp;
        _nativeContext.pvp = windowPVP;
        _nativeContext.pvp.apply( _nativeContext.vp );
        if( oldPVP != _nativeContext.pvp )
            setDirty( DIRTY_VIEWPORT );
    }
    else           // update viewport
    {
        const Viewport oldVP = _nativeContext.vp;
        _nativeContext.vp = _nativeContext.pvp.getSubVP( windowPVP );
        if( oldVP != _nativeContext.vp )
            setDirty( DIRTY_VIEWPORT );
    }

    EQINFO << getName() << " viewport update: " << _nativeContext.vp << ":"
           << _nativeContext.pvp << std::endl;
}

template< class W, class C >
void Channel< W, C >::setNearFar( const float nearPlane, const float farPlane )
{
    EQASSERT( _context );
    if( _nativeContext.frustum.near_plane() == nearPlane && 
        _nativeContext.frustum.far_plane() == farPlane )
    {
        return;
    }

    _nativeContext.frustum.adjust_near( nearPlane );
    _nativeContext.frustum.far_plane() = farPlane;
    _nativeContext.ortho.near_plane()  = nearPlane;
    _nativeContext.ortho.far_plane()   = farPlane;

    if( _context != &_nativeContext )
    {
        _context->frustum.adjust_near( nearPlane );
        _context->frustum.far_plane() = farPlane;
        _context->ortho.near_plane() = nearPlane;
        _context->ortho.far_plane()  = farPlane;
    }
    setDirty( DIRTY_FRUSTUM );
}

template< class W, class C >
void Channel< W, C >::setDrawable( const uint32_t drawable ) 
{ 
    _drawable = drawable;
    setDirty( DIRTY_MEMBER );
}

template< class W, class C >
void Channel< W, C >::setViewVersion( const net::ObjectVersion& view )
{
    if( _nativeContext.view == view )
        return;
    _nativeContext.view = view;
    setDirty( DIRTY_MEMBER );
}

template< class W, class C >
void Channel< W, C >::setMaxSize( const Vector2i& size )
{
    _maxSize = size;
    setDirty( DIRTY_VIEWPORT );
}

template< class W, class C >
void Channel< W, C >::setOverdraw( const Vector4i& overdraw )
{
    if( _nativeContext.overdraw == overdraw )
        return;
    _nativeContext.overdraw = overdraw;
    setDirty( DIRTY_MEMBER );
}

template< class W, class C >
ChannelPath Channel< W, C >::getPath() const
{
    const W* window = getWindow();
    EQASSERT( window );
    ChannelPath path( window->getPath( ));
    
    const typename W::ChannelVector& channels = window->getChannels();
    typename W::ChannelVector::const_iterator i = std::find( channels.begin(),
                                                             channels.end(),
                                                             this );
    EQASSERT( i != channels.end( ));
    path.channelIndex = std::distance( channels.begin(), i );
    return path;
}


template< class W, class C >
int32_t Channel< W, C >::getIAttribute( const IAttribute attr ) const
{
    EQASSERT( attr < IATTR_ALL );
    return _iAttributes[ attr ];
}

template< class W, class C >
const std::string& Channel< W, C >::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

}
}
