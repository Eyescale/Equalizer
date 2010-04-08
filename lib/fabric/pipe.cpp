
/* Copyright (c)      2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com> 
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
#include "task.h"

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

template< class N, class P, class W >
Pipe< N, P, W >::Pipe( N* parent )
        : _port( EQ_UNDEFINED_UINT32 )
        , _device( EQ_UNDEFINED_UINT32 )
        , _node( parent )
{
    parent->_addPipe( static_cast< P* >( this ) );
}

template< class N, class P, class W >
Pipe< N, P, W >::Pipe( const Pipe& from, N* parent  ) 
        : Entity( from )
        , _port( from.getPort() )
        , _device( from.getDevice() )
        , _pvp( from._pvp )
        , _node( parent )
        , _cudaGLInterop( from._cudaGLInterop )
{
    parent->_addPipe( static_cast< P* >( this ) );
    notifyPixelViewportChanged();
}

template< class N, class P, class W >
Pipe< N, P, W >::~Pipe()
{    
    WindowVector& windows = _getWindows(); 
    while( !windows.empty() )
    {
        W* window = windows.back();
        _removeWindow( window );
        delete window;
    }
    windows.clear();
    _node->_removePipe( static_cast< P* >( this ) );
}

template< class N, class P, class W >
PipePath Pipe< N, P, W >::getPath() const
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

template< class N, class P, class W >
void Pipe< N, P, W >::_addWindow( W* window )
{
    EQASSERT( window->getPipe() == this );
    _windows.push_back( window );
}

template< class N, class P, class W >
void Pipe< N, P, W >::setDevice( const uint32_t device )  
{ 
    _device = device; 
    setDirty( DIRTY_MEMBER );
}

template< class N, class P, class W >
void Pipe< N, P, W >::setPort( const uint32_t port )      
{ 
    _port = port; 
    setDirty( DIRTY_MEMBER );
}

template< class N, class P, class W >
bool Pipe< N, P, W >::_removeWindow( W* window )
{
    typename WindowVector::iterator i = find( _windows.begin(), _windows.end(),
                                        window );
    
    if ( i == _windows.end( ) )
        return false;

    _windows.erase( i );
    return true;
}

template< class N, class P, class W >
W* Pipe< N, P, W >::_findWindow( const uint32_t id )
{
    for( typename WindowVector::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        W* window = *i;
        if( window->getID() == id )
            return window;
    }
    return 0;
}

template< class N, class P, class W >
const std::string& Pipe< N, P, W >::getIAttributeString( const IAttribute attr )
{
    return _iPipeAttributeStrings[attr];
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
template< class N, class P, class W >
void Pipe< N, P, W >::setPixelViewport( const PixelViewport& pvp )
{
    if( pvp == _pvp || !pvp.hasArea( ))
        return;

    _pvp = pvp;

    notifyPixelViewportChanged();
    
    EQINFO << "Pipe pvp set: " << _pvp << std::endl;
}

template< class N, class P, class W >
void Pipe< N, P, W >::notifyPixelViewportChanged()
{
    const WindowVector& windows = getWindows();
    for( typename WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        (*i)->notifyViewportChanged();
    }
    setDirty( DIRTY_PIXELVIEWPORT );
    EQINFO << getName() << "pipe pvp update: " << _pvp << std::endl;
}

template< class N, class P, class W  >
void Pipe< N, P, W >::serialize( net::DataOStream& os, 
                                   const uint64_t dirtyBits)
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_PIXELVIEWPORT )
        os << _pvp;
    if( dirtyBits & DIRTY_MEMBER )
        os << _port << _device << _cudaGLInterop;
}

template< class N, class P, class W  >
void Pipe< N, P, W >::deserialize( net::DataIStream& is,
                                     const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_PIXELVIEWPORT )
    {
        PixelViewport pvp;
        is >> pvp;
        setPixelViewport( pvp );
    }
    if( dirtyBits & DIRTY_MEMBER )
        is >> _port >> _device >> _cudaGLInterop;
}

}
}
