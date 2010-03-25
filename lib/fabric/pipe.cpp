
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
#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

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
        : _tasks( fabric::TASK_NONE )        
        , _port( EQ_UNDEFINED_UINT32 )
        , _device( EQ_UNDEFINED_UINT32 )
        , _node( parent )
{}

template< class N, class P, class W >
Pipe< N, P, W >::Pipe( const P& from, N* node  ) 
        : Object()
        , _tasks( fabric::TASK_NONE )        
        , _port( EQ_UNDEFINED_UINT32 )
        , _device( EQ_UNDEFINED_UINT32 )
        , _node( node )

{}

template< class N, class P, class W >
void Pipe< N, P, W >::setErrorMessage( const std::string& message )
{
    if( _error == message )
        return;
    _error = message;
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

}
}
