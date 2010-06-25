
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
        , _context( &_data.nativeContext )
        , _color( Vector3ub::ZERO )
        , _drawable( FB_WINDOW )
        , _maxSize( Vector2i::ZERO )
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
Channel< W, C >::Channel( const Channel& from )
        : Object( from )
        , _window( from._window )
        , _data( from._data )
        , _context( &_data.nativeContext )
        , _color( from._color )
        , _drawable( from._drawable )
        , _maxSize( from._maxSize )
{
    _window->_addChannel( static_cast< C* >( this ));

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
void Channel< W, C >::backup()
{
    Object::backup();
    _backup = _data;
}

template< class W, class C >
void Channel< W, C >::restore()
{
    _data = _backup;
    Object::restore();
    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT | DIRTY_MEMBER | DIRTY_FRUSTUM );
}

template< class W, class C >
void Channel< W, C >::serialize( net::DataOStream& os, const uint64_t dirtyBits)
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os.write( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _data.nativeContext.vp << _data.nativeContext.pvp 
           << _data.fixedVP << _maxSize;
    if( dirtyBits & DIRTY_MEMBER )
        os << _drawable << _color << _data.nativeContext.view
           << _data.nativeContext.overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << _data.nativeContext.frustum;
    if( dirtyBits == DIRTY_ALL )
        os << _color;
}

template< class W, class C >
void Channel< W, C >::deserialize( net::DataIStream& is,
                                   const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is.read( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_VIEWPORT )
    {
        // Ignore data from master (server) if we have local changes
        if( !Serializable::isDirty( DIRTY_VIEWPORT ) || isMaster( ))
        {
            is >> _data.nativeContext.vp >> _data.nativeContext.pvp
           	   >> _data.fixedVP >> _maxSize;
            notifyViewportChanged();
        }
        else // consume unused data
            is.advanceBuffer( sizeof( _data.nativeContext.vp ) + sizeof( _data.nativeContext.pvp ) + sizeof( _data.fixedVP ) + sizeof( _maxSize ));
    }
    if( dirtyBits & DIRTY_MEMBER )
        is >> _drawable >> _color >> _data.nativeContext.view
           >> _data.nativeContext.overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> _data.nativeContext.frustum;
    if( dirtyBits == DIRTY_ALL )
        is >> _color;
}

template< class W, class C >
void Channel< W, C >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    if( isMaster( ))
        _window->setDirty( W::DIRTY_CHANNELS );
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
template< class W, class C >
void Channel< W, C >::setPixelViewport( const PixelViewport& pvp )
{
    EQASSERT( pvp.isValid( ));
    if( !pvp.isValid( ))
        return;

    _data.fixedVP = false;

    if( _data.nativeContext.pvp == pvp && _data.nativeContext.vp.hasArea( ))
        return;

    _data.nativeContext.pvp = pvp;
    _data.nativeContext.vp.invalidate();

    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
}

template< class W, class C >
void Channel< W, C >::setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _data.fixedVP = true;

    if( _data.nativeContext.vp == vp && _data.nativeContext.pvp.hasArea( ))
        return;

    _data.nativeContext.vp = vp;
    _data.nativeContext.pvp.invalidate();

    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
}

template< class W, class C >
void Channel< W, C >::notifyViewportChanged()
{
    if( !_window )
        return;

    PixelViewport windowPVP = _window->getPixelViewport();
    if( !windowPVP.isValid( ))
        return;

    windowPVP.x = 0;
    windowPVP.y = 0;

    if( _data.fixedVP ) // update pixel viewport
    {
        const PixelViewport oldPVP = _data.nativeContext.pvp;
        _data.nativeContext.pvp = windowPVP;
        _data.nativeContext.pvp.apply( _data.nativeContext.vp );
        if( oldPVP != _data.nativeContext.pvp )
            setDirty( DIRTY_VIEWPORT );
    }
    else           // update viewport
    {
        const Viewport oldVP = _data.nativeContext.vp;
        _data.nativeContext.vp = _data.nativeContext.pvp.getSubVP( windowPVP );
        if( oldVP != _data.nativeContext.vp )
            setDirty( DIRTY_VIEWPORT );
    }

    EQVERB << getName() << " viewport update: " << _data.nativeContext.vp << ":"
           << _data.nativeContext.pvp << std::endl;
}

template< class W, class C >
void Channel< W, C >::setNearFar( const float nearPlane, const float farPlane )
{
    EQASSERT( _context );
    if( _data.nativeContext.frustum.near_plane() == nearPlane && 
        _data.nativeContext.frustum.far_plane() == farPlane )
    {
        return;
    }

    _data.nativeContext.frustum.adjust_near( nearPlane );
    _data.nativeContext.frustum.far_plane() = farPlane;
    _data.nativeContext.ortho.near_plane()  = nearPlane;
    _data.nativeContext.ortho.far_plane()   = farPlane;

    if( _context != &_data.nativeContext )
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
    if( _data.nativeContext.view == view )
        return;
    EQASSERTINFO( view.identifier > EQ_ID_MAX ||
                  _data.nativeContext.view.version <= view.version,
                  _data.nativeContext.view << " ! " << view );

    _data.nativeContext.view = view;
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
    if( _data.nativeContext.overdraw == overdraw )
        return;
    _data.nativeContext.overdraw = overdraw;
    setDirty( DIRTY_MEMBER );
}

template< class W, class C >
ChannelPath Channel< W, C >::getPath() const
{
    const W* window = getWindow();
    EQASSERT( window );
    ChannelPath path( window->getPath( ));
    
    const typename W::Channels& channels = window->getChannels();
    typename W::Channels::const_iterator i = std::find( channels.begin(),
                                                        channels.end(), this );
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
