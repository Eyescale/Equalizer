
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

}
}
