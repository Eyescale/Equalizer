
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
 *                          Enrique <egparedes@ifi.uzh.ch>
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
#include "log.h"
#include "task.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{
namespace
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_CHANNEL_") + #attr )
static std::string _iAttributeStrings[] = {
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_ATTR_STRING( IATTR_HINT_SENDTOKEN )
};

static std::string _sAttributeStrings[] = {
    MAKE_ATTR_STRING( SATTR_DUMP_IMAGE )
};
}

template< class W, class C > Channel< W, C >::Channel( W* parent )
    : _window( parent )
    , _context( &_data.nativeContext )
    , _maxSize( Vector2i::ZERO )
{
    memset( _iAttributes, 0xff, IATTR_ALL * sizeof( int32_t ));
    parent->_addChannel( static_cast< C* >( this ));
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class W, class C > Channel< W, C >::Channel( const Channel& from )
        : Object( from )
        , _window( from._window )
        , _data( from._data )
        , _context( &_data.nativeContext )
        , _maxSize( from._maxSize )
{
    _window->_addChannel( static_cast< C* >( this ));

    for( int i = 0; i < IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];
    for( int i = 0; i < SATTR_ALL; ++i )
        _sAttributes[i] = from._sAttributes[i];
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class W, class C > void Channel< W, C >::init()
{
    notifyViewportChanged();
    unsetDirty( DIRTY_VIEWPORT );
}

template< class W, class C > Channel< W, C >::~Channel()
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
void Channel< W, C >::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    LBASSERT( dirtyBits == DIRTY_ALL ||
              getWindow()->Serializable::isDirty( W::DIRTY_CHANNELS ));
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os << co::Array< int32_t >( _iAttributes, IATTR_ALL )
           << co::Array< std::string >( _sAttributes, SATTR_ALL );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _data.nativeContext.vp << _data.nativeContext.pvp
           << _data.fixedVP << _maxSize;
    if( dirtyBits & DIRTY_MEMBER )
        os << _data.nativeContext.view << _data.nativeContext.overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << _data.nativeContext.frustum;
    if( dirtyBits & DIRTY_CAPABILITIES )
        os << _data.capabilities;
}

template< class W, class C >
void Channel< W, C >::deserialize( co::DataIStream& is,
                                   const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is >> co::Array< int32_t >( _iAttributes, IATTR_ALL )
           >> co::Array< std::string >( _sAttributes, SATTR_ALL );
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
            is.getRemainingBuffer( sizeof( _data.nativeContext.vp ) +
                                   sizeof( _data.nativeContext.pvp ) +
                                   sizeof( _data.fixedVP ) +sizeof( _maxSize ));
    }
    if( dirtyBits & DIRTY_MEMBER )
        is >> _data.nativeContext.view  >> _data.nativeContext.overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> _data.nativeContext.frustum;
    if( dirtyBits & DIRTY_CAPABILITIES )
    {
        is >> _data.capabilities;
        updateCapabilities();
    }
}

template< class W, class C >
void Channel< W, C >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _window->setDirty( W::DIRTY_CHANNELS );
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
template< class W, class C >
void Channel< W, C >::setPixelViewport( const PixelViewport& pvp )
{
    LBASSERT( pvp.isValid( ));
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
        _data.nativeContext.vp = _data.nativeContext.pvp / windowPVP;
        if( oldVP != _data.nativeContext.vp )
            setDirty( DIRTY_VIEWPORT );
    }

    LBVERB << getName() << " viewport update: " << _data.nativeContext.vp << ":"
           << _data.nativeContext.pvp << std::endl;
}

template< class W, class C >
void Channel< W, C >::setNearFar( const float nearPlane, const float farPlane )
{
    LBASSERT( _context );
    if( _data.nativeContext.frustum.near_plane() != nearPlane ||
        _data.nativeContext.frustum.far_plane() != farPlane )
    {
        _data.nativeContext.frustum.adjust_near( nearPlane );
        _data.nativeContext.frustum.far_plane() = farPlane;
        _data.nativeContext.ortho.near_plane()  = nearPlane;
        _data.nativeContext.ortho.far_plane()   = farPlane;
        setDirty( DIRTY_FRUSTUM );
    }

    if( _context == &_data.nativeContext )
        return;

    if( _context->frustum.near_plane() != nearPlane ||
        _context->frustum.far_plane() != farPlane )
    {
        _context->frustum.adjust_near( nearPlane );
        _context->frustum.far_plane() = farPlane;
        _context->ortho.near_plane() = nearPlane;
        _context->ortho.far_plane()  = farPlane;
    }
}

template< class W, class C >
void Channel< W, C >::setViewVersion( const co::ObjectVersion& view )
{
    if( _data.nativeContext.view == view )
        return;
    LBASSERTINFO( view.identifier != 0 ||
                  _data.nativeContext.view.version <= view.version,
                  _data.nativeContext.view << " != " << view );

    _data.nativeContext.view = view;
    setDirty( DIRTY_MEMBER );
}

template< class W, class C >
uint64_t Channel< W, C >::getCapabilities() const
{
    return _data.capabilities;
}

template< class W, class C >
void Channel< W, C >::setCapabilities( const uint64_t bitmask )
{
    if ( bitmask == _data.capabilities )
        return;

    _data.capabilities = bitmask;
    setDirty( DIRTY_CAPABILITIES );
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
    LBASSERT( window );
    ChannelPath path( window->getPath( ));

    const typename W::Channels& channels = window->getChannels();
    typename W::Channels::const_iterator i = std::find( channels.begin(),
                                                        channels.end(), this );
    LBASSERT( i != channels.end( ));
    path.channelIndex = std::distance( channels.begin(), i );
    return path;
}


template< class W, class C >
int32_t Channel< W, C >::getIAttribute( const IAttribute attr ) const
{
    LBASSERT( attr < IATTR_ALL );
    return _iAttributes[ attr ];
}

template< class W, class C >
const std::string& Channel< W, C >::getSAttribute( const SAttribute attr ) const
{
    LBASSERT( attr < SATTR_ALL );
    return _sAttributes[ attr ];
}

template< class W, class C >
const std::string& Channel< W, C >::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

template< class W, class C >
const std::string& Channel< W, C >::getSAttributeString( const SAttribute attr )
{
    return _sAttributeStrings[attr];
}

template< class W, class C >
std::ostream& operator << ( std::ostream& os,
                            const Channel< W, C >& channel)
{
    if( channel.omitOutput( ))
        return os;

    os << lunchbox::disableFlush << lunchbox::disableHeader << "channel"
       << std::endl;
    os << "{" << std::endl << lunchbox::indent;

    const std::string& name = channel.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Viewport& vp = channel.getViewport();
    const PixelViewport& pvp = channel.getPixelViewport();
    if( vp.hasArea() && channel.hasFixedViewport( ))
    {
        if( pvp.hasArea( ))
            os << "viewport " << pvp << std::endl;
        os << "viewport " << vp << std::endl;
    }
    else if( pvp.hasArea() && !channel.hasFixedViewport( ))
    {
        if( vp.hasArea( ))
            os << "viewport " << vp << std::endl;
        os << "viewport " << pvp << std::endl;
    }

    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;

    return os;
}

}
}
