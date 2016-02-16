
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "window.h"

#include "channel.h"
#include "commands.h"
#include "elementVisitor.h"
#include "leafVisitor.h"
#include "log.h"
#include "task.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/objectICommand.h>

namespace eq
{
namespace fabric
{

namespace
{
#define MAKE_WINDOW_ATTR_STRING( attr ) ( std::string("EQ_WINDOW_") + #attr )
std::string _iAttributeStrings[] = {
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_CORE_PROFILE ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_OPENGL_MAJOR ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_OPENGL_MINOR ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_STEREO ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DOUBLEBUFFER ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_FULLSCREEN ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DECORATION ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_SWAPSYNC ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DRAWABLE ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_SCREENSAVER ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_GRAB_POINTER ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_WIDTH ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_HEIGHT ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_COLOR ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ALPHA ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_DEPTH ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_STENCIL ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ACCUM ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ACCUM_ALPHA ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_SAMPLES )
};
}

template< class P, class W, class C, class Settings >
Window< P, W, C, Settings >::Window( P* parent )
    : _pipe( parent )
{
    LBASSERT( parent );
    parent->_addWindow( static_cast< W* >( this ) );
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class P, class W, class C, class Settings >
Window< P, W, C, Settings >::BackupData::BackupData()
    : fixedVP( true )
{
}

template< class P, class W, class C, class Settings >
Window< P, W, C, Settings >::~Window( )
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    while( !_channels.empty( ))
    {
        C* channel = _channels.back();

        LBASSERT( channel->getWindow() == this );
        _removeChannel( channel );
        delete channel;
    }
    _pipe->_removeWindow( static_cast< W* >( this ) );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::init()
{
    notifyViewportChanged();
    unsetDirty( DIRTY_VIEWPORT );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::attach( const uint128_t& id,
                                const uint32_t instanceID )
{
    Object::attach( id, instanceID );
    co::CommandQueue* queue = _pipe->getMainThreadQueue();
    LBASSERT( queue );

    registerCommand( CMD_WINDOW_NEW_CHANNEL,
                     CmdFunc( this, &Window< P, W, C, Settings >::_cmdNewChannel ),
                     queue );
    registerCommand( CMD_WINDOW_NEW_CHANNEL_REPLY,
                     CmdFunc( this, &Window< P, W, C, Settings >::_cmdNewChannelReply ),
                     0 );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::backup()
{
    Object::backup();
    _backup = _data;
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::restore()
{
    _data = _backup;
    _data.drawableConfig = DrawableConfig();

    Object::restore();
    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
}

template< class P, class W, class C, class Settings >
uint128_t Window< P, W, C, Settings >::commit( const uint32_t incarnation )
{
    if( Serializable::isDirty( DIRTY_CHANNELS ))
        commitChildren< C >( _channels, CMD_WINDOW_NEW_CHANNEL, incarnation );
    return Object::commit( incarnation );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::serialize( co::DataOStream& os,
                                   const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_SETTINGS )
        _data.windowSettings.serialize( os );
    if( dirtyBits & DIRTY_CHANNELS && isMaster( ))
    {
        os << _mapNodeObjects();
        os.serializeChildren( _channels );
    }
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _data.vp << _data.pvp << _data.fixedVP;
    if( dirtyBits & DIRTY_DRAWABLECONFIG )
        os << _data.drawableConfig;
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::deserialize( co::DataIStream& is,
                                     const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_SETTINGS )
        _data.windowSettings.deserialize( is );
    if( dirtyBits & DIRTY_CHANNELS )
    {
        if( isMaster( ))
            syncChildren( _channels );
        else
        {
            const bool useChildren = is.read< bool >();
            if( useChildren && _mapNodeObjects( ))
            {
                Channels result;
                is.deserializeChildren( this, _channels, result );
                _channels.swap( result );
                LBASSERT( _channels.size() == result.size( ));
            }
            else // consume unused ObjectVersions
                is.read< co::ObjectVersions >();
        }
    }
    if( dirtyBits & DIRTY_VIEWPORT )
    {
        // Ignore data from master (server) if we have local changes
        if( !Serializable::isDirty( DIRTY_VIEWPORT ) || isMaster( ))
        {
            is >> _data.vp >> _data.pvp >> _data.fixedVP;
            notifyViewportChanged();
        }
        else // consume unused data
            is.getRemainingBuffer( sizeof( _data.vp ) + sizeof( _data.pvp ) +
                                   sizeof( _data.fixedVP ));
    }

    if( dirtyBits & DIRTY_DRAWABLECONFIG )
        is >> _data.drawableConfig;
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _pipe->setDirty( P::DIRTY_WINDOWS );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::notifyDetach()
{
    Object::notifyDetach();
    co::LocalNodePtr node = getLocalNode();

    if( isMaster( ))
    {
        for( typename Channels::const_iterator i = _channels.begin();
             i != _channels.end(); ++i )
        {
            node->releaseObject( *i );
        }
    }
    else
    {
        while( !_channels.empty( ))
        {
            C* channel = _channels.back();
            LBASSERT( channel->isAttached( ));

            node->releaseObject( channel );
            _removeChannel( channel );
            _pipe->getServer()->getNodeFactory()->releaseChannel( channel );
        }
    }
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::create( C** channel )
{
    *channel = _pipe->getServer()->getNodeFactory()->createChannel(
        static_cast< W* >( this ));
    (*channel)->init(); // not in ctor, virtual method
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::release( C* channel )
{
    _pipe->getServer()->getNodeFactory()->releaseChannel( channel );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::_addChannel( C* channel )
{
    LBASSERT( channel->getWindow() == this );
    _channels.push_back( channel );
    setDirty( DIRTY_CHANNELS );
}

template< class P, class W, class C, class Settings >
bool Window< P, W, C, Settings >::_removeChannel( C* channel )
{
    typename Channels::iterator i = lunchbox::find( _channels, channel );
    if( i == _channels.end( ))
        return false;

    _channels.erase( i );
    setDirty( DIRTY_CHANNELS );
    if( !isMaster( ))
        postRemove( channel );
    return true;
}

template< class P, class W, class C, class Settings >
C* Window< P, W, C, Settings >::_findChannel( const uint128_t& id )
{
    for( typename Channels::const_iterator i = _channels.begin();
         i != _channels.end(); ++i )
    {
        C* channel = *i;
        if( channel->getID() == id )
            return channel;
    }
    return 0;
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::setIAttribute( const WindowSettings::IAttribute attr,
                                       const int32_t value )
{
    if( _data.windowSettings.setIAttribute( attr, value ))
        setDirty( DIRTY_SETTINGS );
}

template< class P, class W, class C, class Settings > int32_t
Window< P, W, C, Settings >::getIAttribute( const WindowSettings::IAttribute attr ) const
{
    return _data.windowSettings.getIAttribute( attr );
}

template< class P, class W, class C, class Settings > const std::string&
Window< P, W, C, Settings >::getIAttributeString( const WindowSettings::IAttribute attr )
{
    return _iAttributeStrings[attr];
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::setSettings( const Settings& settings )
{
    _data.windowSettings = settings;
    setDirty( DIRTY_SETTINGS );
}

template< class P, class W, class C, class Settings >
const Settings& Window< P, W, C, Settings >::getSettings() const
{
    return _data.windowSettings;
}

template< class P, class W, class C, class Settings >
WindowPath Window< P, W, C, Settings >::getPath() const
{
    const P* pipe = getPipe();
    LBASSERT( pipe );
    WindowPath path( pipe->getPath( ));

    const typename std::vector< W* >& windows = pipe->getWindows();
    typename std::vector< W* >::const_iterator i = std::find( windows.begin(),
                                                              windows.end(),
                                                              this );
    LBASSERT( i != windows.end( ));
    path.windowIndex = std::distance( windows.begin(), i );
    return path;
}

namespace
{
template< class W, class V >
VisitorResult _accept( W* window, V& visitor )
{
    VisitorResult result = visitor.visitPre( window );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename W::Channels& channels = window->getChannels();
    for( typename W::Channels::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;

            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( window ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;

        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

template< class P, class W, class C, class Settings >
VisitorResult Window< P, W, C, Settings >::accept( Visitor& visitor )
{
    return _accept( static_cast< W* >( this ), visitor );
}

template< class P, class W, class C, class Settings >
VisitorResult Window< P, W, C, Settings >::accept( Visitor& visitor  ) const
{
    return _accept( static_cast< const W* >( this ), visitor );
}

template< class P, class W, class C, class Settings >
const PixelViewport& Window< P, W, C, Settings >::getPixelViewport() const
{
    return _data.windowSettings.getPixelViewport();
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::setName( const std::string& name )
{
    Object::setName( name );
    if( _data.windowSettings.getName() == name )
        return;
    _data.windowSettings.setName( name );
    setDirty( DIRTY_SETTINGS );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::setPixelViewport( const PixelViewport& pvp )
{
    LBASSERTINFO( pvp.isValid(), pvp );
    if( !pvp.isValid( ))
        return;

    _data.fixedVP = false;

    if( pvp == _data.pvp && _data.vp.hasArea( ))
        return;

    _data.pvp = pvp;
    _data.vp.invalidate();
    _data.windowSettings.setPixelViewport( pvp );

    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
    setDirty( DIRTY_SETTINGS );
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::setViewport( const Viewport& vp )
{
    if( !vp.hasArea())
        return;

    _data.fixedVP = true;

    if( vp == _data.vp && _data.pvp.hasArea( ))
        return;
    _data.vp = vp;
    _data.pvp.invalidate();
    _data.windowSettings.setPixelViewport( PixelViewport( ));

    setDirty( DIRTY_VIEWPORT );
    setDirty( DIRTY_SETTINGS );
    notifyViewportChanged();
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::notifyViewportChanged()
{
    const PixelViewport pipePVP = _pipe->getPixelViewport();

    if( _data.fixedVP ) // update pixel viewport
    {
        const PixelViewport oldPVP = _data.pvp;
        _data.pvp = pipePVP;
        _data.pvp.apply( _data.vp );
        if( oldPVP != _data.pvp )
        {
            _data.windowSettings.setPixelViewport( _data.pvp );
            setDirty( DIRTY_VIEWPORT );
            setDirty( DIRTY_SETTINGS );
        }
    }
    else           // update viewport
    {
        const Viewport oldVP = _data.vp;
        _data.vp = _data.pvp / pipePVP;
        if( oldVP != _data.vp )
            setDirty( DIRTY_VIEWPORT );
    }

    for( typename Channels::const_iterator i = _channels.begin();
         i != _channels.end(); ++i )
    {
        (*i)->notifyViewportChanged();
    }
    LBVERB << getName() << " viewport update: " << _data.vp << ":" << _data.pvp
           << std::endl;
}

template< class P, class W, class C, class Settings >
void Window< P, W, C, Settings >::_setDrawableConfig(const DrawableConfig& drawableConfig)
{
    _data.drawableConfig = drawableConfig;
    setDirty( DIRTY_DRAWABLECONFIG );
}

//----------------------------------------------------------------------
// ICommand handlers
//----------------------------------------------------------------------
template< class P, class W, class C, class Settings >
bool Window< P, W, C, Settings >::_cmdNewChannel( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    C* channel = 0;
    create( &channel );
    LBASSERT( channel );

    getLocalNode()->registerObject( channel );
    LBASSERT( channel->isAttached() );

    send( command.getRemoteNode(), CMD_WINDOW_NEW_CHANNEL_REPLY )
            << command.read< uint32_t >() << channel->getID();
    LBASSERT( channel->isAttached( ));

    return true;
}

template< class P, class W, class C, class Settings >
bool Window< P, W, C, Settings >::_cmdNewChannelReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const uint32_t requestID = command.read< uint32_t >();
    const uint128_t& result = command.read< uint128_t >();

    getLocalNode()->serveRequest( requestID, result );

    return true;
}

template< class P, class W, class C, class Settings >
std::ostream& operator << ( std::ostream& os, const Window< P, W, C, Settings >& window )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader
       << "window" << std::endl;
    os << "{" << std::endl << lunchbox::indent;

    const std::string& name = window.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Viewport& vp = window.getViewport();
    const PixelViewport& pvp = window.getPixelViewport();
    if( vp.hasArea() && window.hasFixedViewport( ))
    {
        if( pvp.hasArea( ))
            os << "viewport " << pvp << std::endl;
        os << "viewport " << vp << std::endl;
    }
    else if( pvp.hasArea() && !window.hasFixedViewport( ))
    {
        if( vp.hasArea( ))
            os << "viewport " << vp << std::endl;
        os << "viewport " << pvp << std::endl;
    }

    window.output( os );

    const typename W::Channels& channels = window.getChannels();
    for( typename W::Channels::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        os << **i;
    }

    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;
    return os;
}

}
}
