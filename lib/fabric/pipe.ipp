
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "pipe.h"

#include "elementVisitor.h"
#include "leafVisitor.h"
#include "packets.h"
#include "task.h"

#include <eq/net/command.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace fabric
{
namespace
{

#define MAKE_PIPE_ATTR_STRING( attr ) ( std::string("EQ_PIPE_") + #attr )
std::string _iPipeAttributeStrings[] = {
    MAKE_PIPE_ATTR_STRING( IATTR_HINT_THREAD ),
    MAKE_PIPE_ATTR_STRING( IATTR_HINT_CUDA_GL_INTEROP ),
    MAKE_PIPE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_PIPE_ATTR_STRING( IATTR_FILL2 )
};

}

template< class N, class P, class W, class V >
Pipe< N, P, W, V >::Pipe( N* parent )
        : _node( parent )
        , _port( EQ_UNDEFINED_UINT32 )
        , _device( EQ_UNDEFINED_UINT32 )
{
    parent->_addPipe( static_cast< P* >( this ) );
}

template< class N, class P, class W, class V >
Pipe< N, P, W, V >::~Pipe()
{
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
void Pipe< N, P, W, V >::attachToSession( const uint32_t id,
                                              const uint32_t instanceID,
                                              net::Session* session )
{
    Object::attachToSession( id, instanceID, session );

    net::CommandQueue* queue = _node->getConfig()->getMainThreadQueue();
    EQASSERT( queue );

    registerCommand( fabric::CMD_PIPE_NEW_WINDOW, 
                     CmdFunc( this, &Pipe< N, P, W, V >::_cmdNewWindow ),
                     queue );
    registerCommand( fabric::CMD_PIPE_NEW_WINDOW_REPLY, 
                     CmdFunc( this, &Pipe< N, P, W, V >::_cmdNewWindowReply ),
                     0 );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::serialize( net::DataOStream& os,
                                    const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os.write( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_WINDOWS )
    {
        os << _mapNodeObjects();
        os.serializeChildren( this, _windows );
    }
    if( dirtyBits & DIRTY_PIXELVIEWPORT )
        os << _data.pvp;
    if( dirtyBits & DIRTY_MEMBER )
        os << _port << _device;
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::deserialize( net::DataIStream& is,
                                      const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is.read( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_WINDOWS )
    {
        bool useChildren;
        is >> useChildren;
        if( useChildren && _mapNodeObjects( ))
        {
            Windows result;
            is.deserializeChildren( this, _windows, result );
            _windows.swap( result );
            EQASSERT( _windows.size() == result.size( ));
        }
        else // consume unused ObjectVersions
        {
            net::ObjectVersions childIDs;
            is >> childIDs;
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
    if( isMaster( ))
        _node->setDirty( N::DIRTY_PIPES );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::notifyDetach()
{
    while( !_windows.empty( ))
    {
        W* window = _windows.back();
        if( window->getID() > EQ_ID_MAX )
        {
            EQASSERT( isMaster( ));
            return;
        }

        net::Session* session = getSession();
        EQASSERT( session );
        EQASSERT( !isMaster( ));

        session->unmapObject( window );
        _removeWindow( window );
        _node->getServer()->getNodeFactory()->releaseWindow( window );
    }
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::create( W** window )
{
    *window = _node->getServer()->getNodeFactory()->createWindow( 
        static_cast< P* >( this ));
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
    EQASSERT( node );
    PipePath path( node->getPath( ));
    
    const typename std::vector< P* >& pipes = node->getPipes();
    typename std::vector< P* >::const_iterator i = std::find( pipes.begin(),
                                                              pipes.end(),
                                                              this );
    EQASSERT( i != pipes.end( ));
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
    EQASSERT( window->getPipe() == this );
    _windows.push_back( window );
}

template< class N, class P, class W, class V >
bool Pipe< N, P, W, V >::_removeWindow( W* window )
{
    typename Windows::iterator i = find( _windows.begin(), _windows.end(),
                                         window );
    
    if ( i == _windows.end( ) )
        return false;

    _windows.erase( i );
    return true;
}

template< class N, class P, class W, class V >
W* Pipe< N, P, W, V >::_findWindow( const uint32_t id )
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
    EQINFO << "Pipe pvp set: " << _data.pvp << std::endl;
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
    EQINFO << getName() << " pvp update: " << _data.pvp << std::endl;
}

//----------------------------------------------------------------------
// Command handlers
//----------------------------------------------------------------------
template< class N, class P, class W, class V > net::CommandResult
Pipe< N, P, W, V >::_cmdNewWindow( net::Command& command )
{
    const PipeNewWindowPacket* packet =
        command.getPacket< PipeNewWindowPacket >();
    
    W* window = 0;
    create( &window );
    EQASSERT( window );

    _node->getConfig()->registerObject( window );
    EQASSERT( window->getID() <= EQ_ID_MAX );

    PipeNewWindowReplyPacket reply( packet );
    reply.windowID = window->getID();
    send( command.getNode(), reply ); 

    return net::COMMAND_HANDLED;
}

template< class N, class P, class W, class V > net::CommandResult
Pipe< N, P, W, V >::_cmdNewWindowReply( net::Command& command )
{
    const PipeNewWindowReplyPacket* packet =
        command.getPacket< PipeNewWindowReplyPacket >();
    getLocalNode()->serveRequest( packet->requestID, packet->windowID );

    return net::COMMAND_HANDLED;
}

}
}
