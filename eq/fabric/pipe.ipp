
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "pipe.h"

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

#define MAKE_PIPE_ATTR_STRING( attr ) ( std::string("EQ_PIPE_") + #attr )
std::string _iPipeAttributeStrings[] = {
    MAKE_PIPE_ATTR_STRING( IATTR_HINT_THREAD ),
    MAKE_PIPE_ATTR_STRING( IATTR_HINT_AFFINITY ),
    MAKE_PIPE_ATTR_STRING( IATTR_HINT_CUDA_GL_INTEROP ),
};

}

template< class N, class P, class W, class V >
Pipe< N, P, W, V >::Pipe( N* parent )
        : _node( parent )
        , _port( LB_UNDEFINED_UINT32 )
        , _device( LB_UNDEFINED_UINT32 )
{
    memset( _iAttributes, 0xff, IATTR_ALL * sizeof( int32_t ));
    parent->_addPipe( static_cast< P* >( this ) );
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class N, class P, class W, class V >
Pipe< N, P, W, V >::~Pipe()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    while( !_windows.empty() )
    {
        W* window = _windows.back();
        _removeWindow( window );
        delete window;
    }
    _node->_removePipe( static_cast< P* >( this ) );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::backup()
{
    Object::backup();
    _backup = _data;
}


template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::restore()
{
    _data = _backup;
    Object::restore();
    notifyPixelViewportChanged();
    setDirty( DIRTY_PIXELVIEWPORT );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::attach( const uint128_t& id,
                                 const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    co::CommandQueue* queue = _node->getConfig()->getMainThreadQueue();
    LBASSERT( queue );

    registerCommand( CMD_PIPE_NEW_WINDOW,
                     CmdFunc( this, &Pipe< N, P, W, V >::_cmdNewWindow ),
                     queue );
    registerCommand( CMD_PIPE_NEW_WINDOW_REPLY,
                     CmdFunc( this, &Pipe< N, P, W, V >::_cmdNewWindowReply ),
                     0 );
}

template< class N, class P, class W, class V >
uint128_t Pipe< N, P, W, V >::commit( const uint32_t incarnation )
{
    if( Serializable::isDirty( DIRTY_WINDOWS ))
        commitChildren< W >( _windows, CMD_PIPE_NEW_WINDOW, incarnation );
    return Object::commit( incarnation );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::serialize( co::DataOStream& os,
                                    const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os << co::Array< int32_t >( _iAttributes, IATTR_ALL );
    if( dirtyBits & DIRTY_WINDOWS && isMaster( ))
    {
        os << _mapNodeObjects();
        os.serializeChildren( _windows );
    }
    if( dirtyBits & DIRTY_PIXELVIEWPORT )
        os << _data.pvp;
    if( dirtyBits & DIRTY_MEMBER )
        os << _port << _device;
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::deserialize( co::DataIStream& is,
                                      const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is >> co::Array< int32_t >( _iAttributes, IATTR_ALL );
    if( dirtyBits & DIRTY_WINDOWS )
    {
        if( isMaster( ))
            syncChildren( _windows );
        else
        {
            const bool useChildren = is.read< bool >();
            if( useChildren && _mapNodeObjects( ))
            {
                Windows result;
                is.deserializeChildren( this, _windows, result );
                _windows.swap( result );
                LBASSERT( _windows.size() == result.size( ));
            }
            else // consume unused ObjectVersions
                is.read< co::ObjectVersions >();
        }
    }
    if( dirtyBits & DIRTY_PIXELVIEWPORT )
    {
        PixelViewport pvp;
        is >> pvp;
        setPixelViewport( pvp );
    }
    if( dirtyBits & DIRTY_MEMBER )
        is >> _port >> _device;
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _node->setDirty( N::DIRTY_PIPES );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::notifyDetach()
{
    Object::notifyDetach();
    while( !_windows.empty( ))
    {
        W* window = _windows.back();
        if( !window->isAttached()  )
        {
            LBASSERT( isMaster( ));
            return;
        }

        LBASSERT( !isMaster( ));

        getLocalNode()->unmapObject( window );
        _removeWindow( window );
        _node->getServer()->getNodeFactory()->releaseWindow( window );
    }
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::create( W** window )
{
    *window = _node->getServer()->getNodeFactory()->createWindow(
        static_cast< P* >( this ));
    (*window)->init(); // not in ctor, virtual method
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::release( W* window )
{
    _node->getServer()->getNodeFactory()->releaseWindow( window );
}

namespace
{
template< class P, class V >
VisitorResult _accept( P* pipe, V& visitor )
{
    VisitorResult result = visitor.visitPre( pipe );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename P::Windows& windows = pipe->getWindows();
    for( typename P::Windows::const_iterator i = windows.begin();
         i != windows.end(); ++i )
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

    switch( visitor.visitPost( pipe ))
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

template< class N, class P, class W, class V >
VisitorResult Pipe< N, P, W, V >::accept( V& visitor )
{
    return _accept( static_cast< P* >( this ), visitor );
}

template< class N, class P, class W, class V >
VisitorResult Pipe< N, P, W, V >::accept( V& visitor ) const
{
    return _accept( static_cast< const P* >( this ), visitor );
}

template< class N, class P, class W, class V >
PipePath Pipe< N, P, W, V >::getPath() const
{
    const N* node = getNode();
    LBASSERT( node );
    PipePath path( node->getPath( ));

    const typename std::vector< P* >& pipes = node->getPipes();
    typename std::vector< P* >::const_iterator i = std::find( pipes.begin(),
                                                              pipes.end(),
                                                              this );
    LBASSERT( i != pipes.end( ));
    path.pipeIndex = std::distance( pipes.begin(), i );
    return path;
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::setIAttribute( const IAttribute attr,
                                     const int32_t value )
{
    if( _iAttributes[attr] == value )
        return;

    _iAttributes[attr] = value;
    setDirty( DIRTY_ATTRIBUTES );
 }

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::setDevice( const uint32_t device )
{
    _device = device;
    setDirty( DIRTY_MEMBER );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::setPort( const uint32_t port )
{
    _port = port;
    setDirty( DIRTY_MEMBER );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::_addWindow( W* window )
{
    LBASSERT( window->getPipe() == this );
    _windows.push_back( window );
    setDirty( DIRTY_WINDOWS );
}

template< class N, class P, class W, class V >
bool Pipe< N, P, W, V >::_removeWindow( W* window )
{
    typename Windows::iterator i = find( _windows.begin(), _windows.end(),
                                         window );
    if ( i == _windows.end( ) )
        return false;

    _windows.erase( i );
    setDirty( DIRTY_WINDOWS );
    if( !isMaster( ))
        postRemove( window );
    return true;
}

template< class N, class P, class W, class V >
W* Pipe< N, P, W, V >::_findWindow( const uint128_t& id )
{
    for( typename Windows::const_iterator i = _windows.begin();
         i != _windows.end(); ++i )
    {
        W* window = *i;
        if( window->getID() == id )
            return window;
    }
    return 0;
}

template< class N, class P, class W, class V >
const std::string& Pipe< N, P, W, V >::getIAttributeString( const IAttribute attr )
{
    return _iPipeAttributeStrings[attr];
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::setPixelViewport( const PixelViewport& pvp )
{
    if( pvp == _data.pvp || !pvp.hasArea( ))
        return;

    _data.pvp = pvp;
    notifyPixelViewportChanged();
    LBVERB << "Pipe pvp set: " << _data.pvp << std::endl;
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::notifyPixelViewportChanged()
{
    const Windows& windows = getWindows();
    for( typename Windows::const_iterator i = windows.begin();
         i != windows.end(); ++i )
    {
        (*i)->notifyViewportChanged();
    }
    setDirty( DIRTY_PIXELVIEWPORT );
    LBVERB << getName() << " pvp update: " << _data.pvp << std::endl;
}

//----------------------------------------------------------------------
// ICommand handlers
//----------------------------------------------------------------------
template< class N, class P, class W, class V > bool
Pipe< N, P, W, V >::_cmdNewWindow( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    W* window = 0;
    create( &window );
    LBASSERT( window );

    getLocalNode()->registerObject( window );
    LBASSERT( window->isAttached() );

    send( command.getRemoteNode(), CMD_PIPE_NEW_WINDOW_REPLY )
            << command.read< uint32_t >() << window->getID();

    return true;
}

template< class N, class P, class W, class V > bool
Pipe< N, P, W, V >::_cmdNewWindowReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const uint32_t requestID = command.read< uint32_t >();
    const uint128_t& result = command.read< uint128_t >();

    getLocalNode()->serveRequest( requestID, result );

    return true;
}

template< class N, class P, class W, class V >
std::ostream& operator << ( std::ostream& os, const Pipe< N, P, W, V >& pipe )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "pipe"
       << std::endl;
    os << "{" << std::endl << lunchbox::indent;

    const std::string& name = pipe.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    if( pipe.getPort() != LB_UNDEFINED_UINT32 )
        os << "port     " << pipe.getPort() << std::endl;

    if( pipe.getDevice() != LB_UNDEFINED_UINT32 )
        os << "device   " << pipe.getDevice() << std::endl;

    const PixelViewport& pvp = pipe.getPixelViewport();
    if( pvp.hasArea( ))
        os << "viewport " << pvp << std::endl;

    pipe.output( os );
    os << std::endl;

    const typename P::Windows& windows = pipe.getWindows();
    for( typename P::Windows::const_iterator i = windows.begin();
         i != windows.end(); ++i )
    {
        os << **i;
    }

    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;
    return os;
}

}
}
