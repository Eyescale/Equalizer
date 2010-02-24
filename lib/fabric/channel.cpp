
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

namespace eq
{
namespace fabric
{

template< typename T, typename W >
Channel< T, W >::Channel( W* parent )
        : _window( parent )
{
    parent->_addChannel( static_cast< T* >( this ));
}

template< typename T, typename W >
Channel< T, W >::~Channel()
{  
    _window->_removeChannel( static_cast< T* >( this ));
}

template< typename T, typename W >
VisitorResult Channel< T, W >::accept( LeafVisitor< T >& visitor )
{
    return visitor.visit( static_cast< T* >( this ));
}

template< typename T, typename W >
VisitorResult Channel< T, W >::accept( LeafVisitor< T >& visitor ) const
{
    return visitor.visit( static_cast< const T* >( this ));
}


}
}
