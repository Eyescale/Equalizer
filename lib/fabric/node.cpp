
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 *               2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "node.h"

namespace eq
{
namespace fabric
{
namespace
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_NODE_") + #attr )
std::string _iAttributeStrings[] = {
    MAKE_ATTR_STRING( IATTR_THREAD_MODEL ),
    MAKE_ATTR_STRING( IATTR_LAUNCH_TIMEOUT ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};
}

template< class C, class N, class P >
Node< C, N, P >::Node() 
        : _tasks( fabric::TASK_NONE )
        , _finishedFrame( 0 )
{
}

template< class C, class N, class P >
Node< C, N, P >::Node( const Node& from, C* config )
        : Object( from )
        , _tasks( TASK_NONE )
        , _finishedFrame( 0 )
{
}

template< class C, class N, class P >
NodePath Node< C, N, P >::getPath() const
{
    const C* config = static_cast< const N* >( this )->getConfig( );
    EQASSERT( config );
    
    const typename std::vector< N* >& nodes = config->getNodes();
    typename std::vector< N* >::const_iterator i = std::find( nodes.begin(),
                                                              nodes.end(),
                                                              this );
    EQASSERT( i != nodes.end( ));

    NodePath path;
    path.nodeIndex = std::distance( nodes.begin(), i );
    return path;
}

template< class C, class N, class P >
void Node< C, N, P >::setIAttribute( const IAttribute attr, const int32_t value )
{
    _iAttributes[attr] = value;
}

template< class C, class N, class P >
int32_t Node< C, N, P >::getIAttribute( const IAttribute attr ) const
{
    return _iAttributes[attr];
}

template< class C, class N, class P >
const std::string& Node< C, N, P >::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

template< class C, class N, class P >
void Node< C, N, P >::_addPipe( P* pipe )
{
    EQASSERT( pipe->getNode() == this );
    _pipes.push_back( pipe );
}

template< class C, class N, class P >
bool Node< C, N, P >::_removePipe( P* pipe )
{
    PipeVector::iterator i = find( _pipes.begin(), _pipes.end(), pipe );
    if( i == _pipes.end( ))
        return false;

    _pipes.erase( i );
    return true;
}

template< class C, class N, class P >
P* Node< C, N, P >::_findPipe( const uint32_t id )
{
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); 
         ++i )
    {
        P* pipe = *i;
        if( pipe->getID() == id )
            return pipe;
    }
    return 0;
}

}
}
