
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
#include "task.h"

#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

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

template< typename T, typename W >
Channel< T, W >::Channel( W* parent )
        : _window( parent )
        , _context( &_nativeContext )
        , _color( Vector3ub::ZERO )
        , _tasks( TASK_NONE )
        , _drawable( FB_WINDOW )
        , _maxSize( Vector2i::ZERO )
        , _fixedVP( true )
{
    parent->_addChannel( static_cast< T* >( this ));
    notifyViewportChanged();

    uint32_t value = (reinterpret_cast< size_t >( this ) & 0xffffffffu);
    for( unsigned i=0; i<8; ++i )
    {
        _color.r() |= ( value&1 << (7-i) ); value >>= 1;
        _color.g() |= ( value&1 << (7-i) ); value >>= 1;
        _color.b() |= ( value&1 << (7-i) ); value >>= 1;
    }
}

template< typename T, typename W >
Channel< T, W >::Channel( const Channel& from, W* parent )
        : _window( parent )
        , _context( &_nativeContext )
        , _color( from._color )
        , _tasks( from._tasks )
        , _drawable( from._drawable )
        , _maxSize( from._maxSize )
        , _fixedVP( from._fixedVP )
{
    parent->_addChannel( static_cast< T* >( this ));

    for( int i = 0; i < IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

    notifyViewportChanged();
}

template< typename T, typename W >
Channel< T, W >::~Channel()
{  
    _window->_removeChannel( static_cast< T* >( this ));
}

template< typename T, typename W >
VisitorResult Channel< T, W >::accept( LeafVisitor< T >& visitor )
{
    return visitor.visit( static_cast< T* >( this ));
}

template< typename T, typename W >
VisitorResult Channel< T, W >::accept( LeafVisitor< T >& visitor ) const
{
    return visitor.visit( static_cast< const T* >( this ));
}

template< typename T, typename W >
void Channel< T, W >::serialize( net::DataOStream& os, const uint64_t dirtyBits)
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os.write( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _nativeContext.vp << _nativeContext.pvp << _fixedVP << _maxSize;
    if( dirtyBits & DIRTY_MEMBER )
        os << _drawable << _tasks << _color << _nativeContext.view
           << _nativeContext.overdraw;
    if( dirtyBits & DIRTY_ERROR )
        os << _error;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << _nativeContext.frustum;
    if( dirtyBits & DIRTY_ALL )
        os << _color;
}

template< typename T, typename W >
void Channel< T, W >::deserialize( net::DataIStream& is,
                                   const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is.read( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _nativeContext.vp >> _nativeContext.pvp >> _fixedVP >> _maxSize;
    if( dirtyBits & DIRTY_MEMBER )
        is >> _drawable >> _tasks >> _color >> _nativeContext.view
           >> _nativeContext.overdraw;
    if( dirtyBits & DIRTY_ERROR )
        is >> _error;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> _nativeContext.frustum;
    if( dirtyBits & DIRTY_ALL )
        is >> _color;
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
template< typename T, typename W >
void Channel< T, W >::setPixelViewport( const PixelViewport& pvp )
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

    EQVERB << "Channel pvp set: " << _nativeContext.pvp << ":" 
           << _nativeContext.vp << std::endl;
}

template< typename T, typename W >
void Channel< T, W >::setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _fixedVP = true;

    if( _nativeContext.vp == vp && _nativeContext.pvp.hasArea( ))
        return;

    _nativeContext.vp = vp;
    const PixelViewport oldPVP = _nativeContext.pvp;
    _nativeContext.pvp.invalidate();
    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );

    EQVERB << "Channel vp set: " << _nativeContext.pvp << ":" 
           << _nativeContext.vp << std::endl;
}

template< typename T, typename W >
void Channel< T, W >::notifyViewportChanged()
{
    if( !_window )
        return;

    eq::PixelViewport windowPVP = _window->getPixelViewport();
    if( !windowPVP.isValid( ))
        return;

    windowPVP.x = 0;
    windowPVP.y = 0;

    if( _fixedVP ) // update pixel viewport
        _nativeContext.pvp = windowPVP.getSubPVP( _nativeContext.vp );
    else           // update viewport
        _nativeContext.vp = _nativeContext.pvp.getSubVP( windowPVP );


    setDirty( DIRTY_VIEWPORT );
    EQINFO << "Channel viewport update: " << _nativeContext.pvp << ":" 
           << _nativeContext.vp << std::endl;
}

template< typename T, typename W >
void Channel< T, W >::setNearFar( const float nearPlane, const float farPlane )
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

template< typename T, typename W >
void Channel< T, W >::setDrawable( const uint32_t drawable ) 
{ 
    _drawable = drawable;
    setDirty( DIRTY_MEMBER );
}

template< typename T, typename W >
void Channel< T, W >::setTasks( const uint32_t tasks )
{
    if( _tasks == tasks )
        return;
    _tasks = tasks;
    setDirty( DIRTY_MEMBER );
}

template< typename T, typename W >
void Channel< T, W >::setViewVersion( const net::ObjectVersion& view )
{
    if( _nativeContext.view == view )
        return;
    _nativeContext.view = view;
    setDirty( DIRTY_MEMBER );
}

template< typename T, typename W >
void Channel< T, W >::setMaxSize( const Vector2i& size )
{
    _maxSize = size;
    setDirty( DIRTY_VIEWPORT );
}

template< typename T, typename W >
void Channel< T, W >::setOverdraw( const Vector4i& overdraw )
{
    if( _nativeContext.overdraw == overdraw )
        return;
    _nativeContext.overdraw = overdraw;
    setDirty( DIRTY_MEMBER );
}

template< typename T, typename W >
int32_t Channel< T, W >::getIAttribute( const IAttribute attr ) const
{
    EQASSERT( attr < IATTR_ALL );
    return _iAttributes[ attr ];
}

template< typename T, typename W >
const std::string& Channel< T, W >::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

template< typename T, typename W >
void Channel< T, W >::setErrorMessage( const std::string& message )
{
    if( _error == message )
        return;
    _error = message;
    setDirty( DIRTY_ERROR );
}

}
}
