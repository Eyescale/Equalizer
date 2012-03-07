
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "pipe.h"

#include "elementVisitor.h"
#include "leafVisitor.h"
#include "log.h"
#include "pipePackets.h"
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
        , _port( EQ_UNDEFINED_UINT32 )
        , _device( EQ_UNDEFINED_UINT32 )
{
    memset( _iAttributes, 0xff, IATTR_ALL * sizeof( int32_t ));
    parent->_addPipe( static_cast< P* >( this ) );
    EQLOG( LOG_INIT ) << "New " << co::base::className( this ) << std::endl;
}

template< class N, class P, class W, class V >
Pipe< N, P, W, V >::~Pipe()
{
    EQLOG( LOG_INIT ) << "Delete " << co::base::className( this ) << std::endl;
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
void Pipe< N, P, W, V >::attach( const co::base::UUID& id,
                                 const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    co::CommandQueue* queue = _node->getConfig()->getMainThreadQueue();
    EQASSERT( queue );

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
        commitChildren< W, PipeNewWindowPacket >( _windows, incarnation );
    return Object::commit( incarnation );
}

template< class N, class P, class W, class V >
void Pipe< N, P, W, V >::serialize( co::DataOStream& os, 
                                    const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os.write( _iAttributes, IATTR_ALL * sizeof( int32_t ));
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
        is.read( _iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_WINDOWS )
    {
        if( isMaster( ))
            syncChildren( _windows );
        else
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
                co::ObjectVersions childIDs;
                is >> childIDs;
            }
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
            EQASSERT( isMaster( ));
            return;
        }

        EQASSERT( !isMaster( ));

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
W* Pipe< N, P, W, V >::_findWindow( const co::base::UUID& id )
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
template< class N, class P, class W, class V > bool
Pipe< N, P, W, V >::_cmdNewWindow( co::Command& command )
{
    const PipeNewWindowPacket* packet =
        command.get< PipeNewWindowPacket >();
    
    W* window = 0;
    create( &window );
    EQASSERT( window );

    getLocalNode()->registerObject( window );
    EQASSERT( window->isAttached() );

    PipeNewWindowReplyPacket reply( packet );
    reply.windowID = window->getID();
    send( command.getNode(), reply ); 

    return true;
}

template< class N, class P, class W, class V > bool
Pipe< N, P, W, V >::_cmdNewWindowReply( co::Command& command )
{
    const PipeNewWindowReplyPacket* packet =
        command.get< PipeNewWindowReplyPacket >();
    getLocalNode()->serveRequest( packet->requestID, packet->windowID );

    return true;
}

template< class N, class P, class W, class V >
std::ostream& operator << ( std::ostream& os, const Pipe< N, P, W, V >& pipe )
{
    os << co::base::disableFlush << co::base::disableHeader << "pipe"
       << std::endl;
    os << "{" << std::endl << co::base::indent;

    const std::string& name = pipe.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    if( pipe.getPort() != EQ_UNDEFINED_UINT32 )
        os << "port     " << pipe.getPort() << std::endl;
        
    if( pipe.getDevice() != EQ_UNDEFINED_UINT32 )
        os << "device   " << pipe.getDevice() << std::endl;
    
    const PixelViewport& pvp = pipe.getPixelViewport();
    if( pvp.isValid( ))
        os << "viewport " << pvp << std::endl;

    pipe.output( os );
    os << std::endl;

    const typename P::Windows& windows = pipe.getWindows();
    for( typename P::Windows::const_iterator i = windows.begin();
         i != windows.end(); ++i )
    {
        os << **i;
    }

    os << co::base::exdent << "}" << std::endl << co::base::enableHeader
       << co::base::enableFlush;
    return os;
}

}
}
