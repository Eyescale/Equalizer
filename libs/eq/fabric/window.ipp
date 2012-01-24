
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com> 
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
#include "elementVisitor.h"
#include "log.h"
#include "windowPackets.h"
#include "task.h"

#include <co/command.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{

namespace
{
#define MAKE_WINDOW_ATTR_STRING( attr ) ( std::string("EQ_WINDOW_") + #attr )
std::string _iAttributeStrings[] = {
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_STEREO ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DOUBLEBUFFER ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_FULLSCREEN ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DECORATION ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_SWAPSYNC ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DRAWABLE ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_SCREENSAVER ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_COLOR ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ALPHA ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_DEPTH ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_STENCIL ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ACCUM ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ACCUM_ALPHA ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_SAMPLES )
};
}

template< class P, class W, class C >
Window< P, W, C >::Window( P* parent )
        : _pipe( parent )
{
    EQASSERT( parent );
    parent->_addWindow( static_cast< W* >( this ) );
    EQLOG( LOG_INIT ) << "New " << co::base::className( this ) << std::endl;
}

template< class P, class W, class C >
Window< P, W, C >::BackupData::BackupData()
        : fixedVP( true )
{
    memset( iAttributes, 0xff, IATTR_ALL * sizeof( int32_t ));
}

template< class P, class W, class C >
Window< P, W, C >::~Window( )
{    
    EQLOG( LOG_INIT ) << "Delete " << co::base::className( this ) << std::endl;
    while( !_channels.empty( ))
    {
        C* channel = _channels.back();

        EQASSERT( channel->getWindow() == this );
        _removeChannel( channel );
        delete channel;
    }
    _pipe->_removeWindow( static_cast< W* >( this ) );
}

template< class P, class W, class C >
void Window< P, W, C >::init()
{
    notifyViewportChanged();
    unsetDirty( DIRTY_VIEWPORT );
}

template< class P, class W, class C >
void Window< P, W, C >::attach( const co::base::UUID& id,
                                const uint32_t instanceID )
{
    Object::attach( id, instanceID );
    co::CommandQueue* queue = _pipe->getMainThreadQueue();
    EQASSERT( queue );

    registerCommand( CMD_WINDOW_NEW_CHANNEL, 
                     CmdFunc( this, &Window< P, W, C >::_cmdNewChannel ),
                     queue );
    registerCommand( CMD_WINDOW_NEW_CHANNEL_REPLY, 
                     CmdFunc( this, &Window< P, W, C >::_cmdNewChannelReply ),
                     0 );
}

template< class P, class W, class C >
void Window< P, W, C >::backup()
{
    Object::backup();
    _backup = _data;
}

template< class P, class W, class C >
void Window< P, W, C >::restore()
{
    _data = _backup;
    _data.drawableConfig = DrawableConfig();

    Object::restore();
    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
}

template< class P, class W, class C >
uint128_t Window< P, W, C >::commit( const uint32_t incarnation )
{
    if( Serializable::isDirty( DIRTY_CHANNELS ))
        commitChildren< C, WindowNewChannelPacket >( _channels, incarnation );
    return Object::commit( incarnation );
}

template< class P, class W, class C >
void Window< P, W, C >::serialize( co::DataOStream& os,
                                   const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os.write( _data.iAttributes, IATTR_ALL * sizeof( int32_t ));
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

template< class P, class W, class C >
void Window< P, W, C >::deserialize( co::DataIStream& is,
                                     const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is.read( _data.iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_CHANNELS )
    {
        if( isMaster( ))
            syncChildren( _channels );
        else
        {
            bool useChildren;
            is >> useChildren;
            if( useChildren && _mapNodeObjects( ))
            {
                Channels result;
                is.deserializeChildren( this, _channels, result );
                _channels.swap( result );
                EQASSERT( _channels.size() == result.size( ));
            }
            else // consume unused ObjectVersions
            {
                co::ObjectVersions childIDs;
                is >> childIDs;
            }
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
            is.advanceBuffer( sizeof( _data.vp ) + sizeof( _data.pvp ) +
                              sizeof( _data.fixedVP ));
    }

    if( dirtyBits & DIRTY_DRAWABLECONFIG )
        is >> _data.drawableConfig;
}

template< class P, class W, class C >
void Window< P, W, C >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _pipe->setDirty( P::DIRTY_WINDOWS );
}

template< class P, class W, class C >
void Window< P, W, C >::notifyDetach()
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
            EQASSERT( channel->isAttached( ));

            node->releaseObject( channel );
            _removeChannel( channel );
            _pipe->getServer()->getNodeFactory()->releaseChannel( channel );
        }
    }
}

template< class P, class W, class C >
void Window< P, W, C >::create( C** channel )
{
    *channel = _pipe->getServer()->getNodeFactory()->createChannel( 
        static_cast< W* >( this ));
    (*channel)->init(); // not in ctor, virtual method
}

template< class P, class W, class C >
void Window< P, W, C >::release( C* channel )
{
    _pipe->getServer()->getNodeFactory()->releaseChannel( channel );
}

template< class P, class W, class C >
void Window< P, W, C >::_addChannel( C* channel )
{
    EQASSERT( channel->getWindow() == this );
    _channels.push_back( channel );
    setDirty( DIRTY_CHANNELS );
}

template< class P, class W, class C >
bool Window< P, W, C >::_removeChannel( C* channel )
{
    typename Channels::iterator i = stde::find( _channels, channel );
    if( i == _channels.end( ))
        return false;

    _channels.erase( i );
    setDirty( DIRTY_CHANNELS );
    if( !isMaster( ))
        postRemove( channel );
    return true;
}

template< class P, class W, class C >
C* Window< P, W, C >::_findChannel( const co::base::UUID& id )
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

template< class P, class W, class C > const std::string&
Window< P, W, C >::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

template< class P, class W, class C >
WindowPath Window< P, W, C >::getPath() const
{
    const P* pipe = getPipe();
    EQASSERT( pipe );
    WindowPath path( pipe->getPath( ));
    
    const typename std::vector< W* >& windows = pipe->getWindows();
    typename std::vector< W* >::const_iterator i = std::find( windows.begin(),
                                                              windows.end(),
                                                              this );
    EQASSERT( i != windows.end( ));
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

template< class P, class W, class C >
VisitorResult Window< P, W, C >::accept( Visitor& visitor )
{
    return _accept( static_cast< W* >( this ), visitor );
}

template< class P, class W, class C >
VisitorResult Window< P, W, C >::accept( Visitor& visitor  ) const
{
    return _accept( static_cast< const W* >( this ), visitor );
}

template< class P, class W, class C >
void Window< P, W, C >::setPixelViewport( const PixelViewport& pvp )
{
    EQASSERTINFO( pvp.isValid(), pvp );
    if( !pvp.isValid( ))
        return;

    _data.fixedVP = false;

    if( pvp == _data.pvp && _data.vp.hasArea( ))
        return;

    _data.pvp = pvp;
    _data.vp.invalidate();

    notifyViewportChanged();
    setDirty( DIRTY_VIEWPORT );
}

template< class P, class W, class C >
void Window< P, W, C >::setViewport( const Viewport& vp )
{
    if( !vp.hasArea())
        return;

    _data.fixedVP = true;

    if( vp == _data.vp && _data.pvp.hasArea( ))
        return;
    _data.vp = vp;
    _data.pvp.invalidate();

    setDirty( DIRTY_VIEWPORT );
    notifyViewportChanged();
}

template< class P, class W, class C >
void Window< P, W, C >::notifyViewportChanged()
{
    const PixelViewport pipePVP = _pipe->getPixelViewport();

    if( _data.fixedVP ) // update pixel viewport
    {
        const PixelViewport oldPVP = _data.pvp;
        _data.pvp = pipePVP;
        _data.pvp.apply( _data.vp );
        if( oldPVP != _data.pvp )
            setDirty( DIRTY_VIEWPORT );
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
    EQVERB << getName() << " viewport update: " << _data.vp << ":" << _data.pvp
           << std::endl;
}

template< class P, class W, class C >
void Window< P, W, C >::_setDrawableConfig(const DrawableConfig& drawableConfig)
{ 
    _data.drawableConfig = drawableConfig;
    setDirty( DIRTY_DRAWABLECONFIG );
}

//----------------------------------------------------------------------
// Command handlers
//----------------------------------------------------------------------
template< class P, class W, class C >
bool Window< P, W, C >::_cmdNewChannel( co::Command& command )
{
    const WindowNewChannelPacket* packet =
        command.get< WindowNewChannelPacket >();
    
    C* channel = 0;
    create( &channel );
    EQASSERT( channel );

    getLocalNode()->registerObject( channel );
    EQASSERT( channel->isAttached() );

    WindowNewChannelReplyPacket reply( packet );
    reply.channelID = channel->getID();
    send( command.getNode(), reply ); 
    EQASSERT( channel->isAttached( ));

    return true;
}

template< class P, class W, class C >
bool Window< P, W, C >::_cmdNewChannelReply( co::Command& command )
{
    const WindowNewChannelReplyPacket* packet =
        command.get< WindowNewChannelReplyPacket >();
    getLocalNode()->serveRequest( packet->requestID, packet->channelID );

    return true;
}

template< class P, class W, class C >
std::ostream& operator << ( std::ostream& os, const Window< P, W, C >& window )
{
    os << co::base::disableFlush << co::base::disableHeader
       << "window" << std::endl;
    os << "{" << std::endl << co::base::indent;

    const std::string& name = window.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Viewport& vp = window.getViewport();
    const PixelViewport& pvp = window.getPixelViewport();
    if( vp.isValid( ) && window.hasFixedViewport( ))
    {
        if( pvp.hasArea( ))
            os << "viewport " << pvp << std::endl;
        os << "viewport " << vp << std::endl;
    }
    else if( pvp.hasArea( ))
    {
        if( vp != Viewport::FULL && vp.isValid( ))
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

    os << co::base::exdent << "}" << std::endl << co::base::enableHeader
       << co::base::enableFlush;
    return os;
}

}
}
