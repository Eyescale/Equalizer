
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
#include "leafVisitor.h"
#include "paths.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace fabric
{
template< class L, class V, class O > 
View< L, V, O >::View( L* layout )
        : _layout( layout )
        , _observer( 0 )
        , _overdraw( Vector2i::ZERO )
{
    // Note: Views are an exception to the strong structuring, since render
    // client views are multi-buffered (once per pipe) and do not have a parent
    if( layout )
        layout->_addView( static_cast< V* >( this ));
}

template< class L, class V, class O > 
View< L, V, O >::~View()
{
    if( _layout )
        _layout->_removeView( static_cast< V* >( this ));
}

template< class L, class V, class O > 
void View< L, V, O >::serialize( net::DataOStream& os, const uint64_t dirtyBits)
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _viewport;
    if( dirtyBits & DIRTY_OBSERVER )
        os << net::ObjectVersion( _observer );
    if( dirtyBits & DIRTY_OVERDRAW )
        os << _overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << *static_cast< Frustum* >( this );
}

template< class L, class V, class O > 
void View< L, V, O >::deserialize( net::DataIStream& is, 
                                   const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _viewport;
    if( dirtyBits & DIRTY_OBSERVER )
    {
        net::ObjectVersion observer;
        is >> observer;

        if( observer.identifier == EQ_ID_INVALID )
            _observer = 0;
        else
        {
            if( !_observer && _layout ) // don't map render client observers yet
            {
                L* layout = getLayout();
                EQASSERT( layout && layout->getConfig( ));
                layout->getConfig()->find( observer.identifier, &_observer );
                EQASSERT( _observer );
                EQASSERT( _observer->getID() == observer.identifier );
            }
            if( _observer )
                _observer->sync( observer.version );
        }
    }
    if( dirtyBits & DIRTY_OVERDRAW )
        is >> _overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> *static_cast< Frustum* >( this );
}

template< class L, class V, class O > 
void View< L, V, O >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    if( isMaster( ))
        _layout->setDirty( L::DIRTY_VIEWS );
}

template< class L, class V, class O > 
void View< L, V, O >::setViewport( const Viewport& viewport )
{
    _viewport = viewport;
    setDirty( DIRTY_VIEWPORT );
}

template< class L, class V, class O > void View< L, V, O >::backup()
{
    Frustum::backup();
    Object::backup();
}

template< class L, class V, class O > void View< L, V, O >::restore()
{
    Object::restore();
    Frustum::restore();
}

template< class L, class V, class O > 
void View< L, V, O >::setObserver( O* observer )
{
    if( _observer == observer )
        return;

    _observer = observer;
    setDirty( DIRTY_OBSERVER );
}

template< class L, class V, class O > 
const Viewport& View< L, V, O >::getViewport() const
{
    return _viewport;
}

template< class L, class V, class O > 
void View< L, V, O >::setOverdraw( const Vector2i& pixels )
{
    _overdraw = pixels;
    setDirty( DIRTY_OVERDRAW );
}

template< class L, class V, class O > 
VisitorResult View< L, V, O >::accept( LeafVisitor< V >& visitor )
{
    return visitor.visit( static_cast< V* >( this ));
}

template< class L, class V, class O > 
VisitorResult View< L, V, O >::accept( LeafVisitor< V >& visitor ) const
{
    return visitor.visit( static_cast< const V* >( this ));
}

template< class L, class V, class O > 
void View< L, V, O >::setWall( const Wall& wall )
{
    if( getWall() == wall && getCurrentType() == TYPE_WALL )
        return;

    Frustum::setWall( wall );
    setDirty( DIRTY_FRUSTUM );
}

template< class L, class V, class O > 
void View< L, V, O >::setProjection( const Projection& projection )
{
    if( getProjection() == projection && getCurrentType() == TYPE_PROJECTION )
        return;

    Frustum::setProjection( projection );
    setDirty( DIRTY_FRUSTUM );
}

template< class L, class V, class O > 
void View< L, V, O >::unsetFrustum()
{
    if( getCurrentType() == TYPE_NONE )
        return;

    Frustum::unsetFrustum();
    setDirty( DIRTY_FRUSTUM );
}

template< class L, class V, class O > 
void View< L, V, O >::notifyAttached()
{
    Object::notifyAttached();
    net::Object* userData = getUserData();
    if( userData && userData->isMaster( ))
        userData->setAutoObsolete( _layout->getConfig()->getLatency( ));
}

template< class L, class V, class O >
std::ostream& operator << ( std::ostream& os, const View< L, V, O >& view )
{
    os << base::disableFlush << base::disableHeader << "view" << std::endl;
    os << "{" << std::endl << base::indent;
    
    const std::string& name = view.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const eq::Viewport& vp  = view.getViewport();
    if( vp.isValid( ) && vp != eq::Viewport::FULL )
        os << "viewport " << vp << std::endl;

    const O* observer = static_cast< const O* >( view.getObserver( ));
    if( observer )
    {
        const std::string& observerName = observer->getName();
        const L* layout = view.getLayout();
        const O* foundObserver = 0;
        layout->getConfig()->find( observerName, &foundObserver );

        if( layout && foundObserver == observer )
        {
            os << "observer \"" << observerName << "\"" << std::endl;
        }
        else
            os << observer->getPath() << std::endl;
    } 

    switch( view.getCurrentType( ))
    {
        case View< L, V, O >::TYPE_WALL:
            os << view.getWall() << std::endl;
            break;
        case View< L, V, O >::TYPE_PROJECTION:
            os << view.getProjection() << std::endl;
            break;
        default: 
            break;
    }

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}
