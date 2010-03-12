
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "view.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
templace< class L, class V, class O > 
View< L, V, O >::View( L* layout )
        : _layout( layout )
        , _observer( 0 )
        , _overdraw( Vector2i::ZERO )
{
    EQASSERT( layout );
    layout->_addView( static_cast< V* >( this ));
}

templace< class L, class V, class O > 
View< L, V, O >::~View()
{
    layout->_removeView( static_cast< V* >( this ));
}

templace< class L, class V, class O > 
void View< L, V, O >::serialize( net::DataOStream& os, const uint64_t dirtyBits)
{
    Frustum::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _viewport;
    if( dirtyBits & DIRTY_OBSERVER )
        os << ( _observer ? _observer->getID() : EQ_ID_INVALID );
    if( dirtyBits & DIRTY_OVERDRAW )
        os << _overdraw;
}

templace< class L, class V, class O > 
void View< L, V, O >::deserialize( net::DataIStream& is, 
                                   const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _viewport;
    if( dirtyBits & DIRTY_OBSERVER )
    {
        uint32_t id;
        is >> id;

        if( id == EQ_ID_INVALID )
            _observer = 0;
        else
        {
            Config* config = _layout->getConfig();
            EQASSERT( config );
            _observer = const_cast< O* >( config->findObserver( id ));
            EQASSERT( _observer );
        }
    }
    if( dirtyBits & DIRTY_OVERDRAW )
        is >> _overdraw;
}

templace< class L, class V, class O > 
const Viewport& View< L, V, O >::getViewport() const
{
    return _viewport;
}

templace< class L, class V, class O > 
void View< L, V, O >::setOverdraw( const Vector2i& pixels )
{
    _overdraw = pixels;
    setDirty( DIRTY_OVERDRAW );
}

templace< class L, class V, class O > 
VisitorResult View< L, V, O >::accept( ViewVisitor& visitor )
{
    return visitor.visit( static_cast< V* >( this ));
}

templace< class L, class V, class O > 
VisitorResult View< L, V, O >::accept( ViewVisitor& visitor ) const
{
    return visitor.visit( static_cast< const V* >( this ));
}

}
}
