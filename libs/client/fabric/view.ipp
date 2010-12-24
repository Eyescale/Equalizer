
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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
#include "log.h"
#include "paths.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{
template< class L, class V, class O > 
View< L, V, O >::View( L* layout )
        : _layout( layout )
        , _observer( 0 )
        , _overdraw( Vector2i::ZERO )
        , _mode( MODE_MONO )
        , _minimumCapabilities( EQ_BIT_NONE )
        , _maximumCapabilities( EQ_BIT_ALL_64 )
        , _capabilities( EQ_BIT_ALL_64 )
{
    // Note: Views are an exception to the strong structuring, since render
    // client views are multi-buffered (once per pipe) and do not have a parent
    if( layout )
        layout->_addChild( static_cast< V* >( this ));
    EQLOG( LOG_INIT ) << "New " << co::base::className( this ) << std::endl;
}

template< class L, class V, class O > 
View< L, V, O >::~View()
{
    EQLOG( LOG_INIT ) << "Delete " << co::base::className( this ) << std::endl;
    if( _layout )
        _layout->_removeChild( static_cast< V* >( this ));
}

template< class L, class V, class O > 
void View< L, V, O >::serialize( co::DataOStream& os, const uint64_t dirtyBits)
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _viewport;
    if( dirtyBits & DIRTY_OBSERVER )
        os << co::ObjectVersion( _observer );
    if( dirtyBits & DIRTY_OVERDRAW )
        os << _overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        os << *static_cast< Frustum* >( this );
    if( dirtyBits & DIRTY_MODE )
        os << _mode;
    if( dirtyBits & DIRTY_MINCAPS )
        os << _minimumCapabilities;
    if( dirtyBits & DIRTY_MAXCAPS )
        os << _maximumCapabilities;
    if( dirtyBits & DIRTY_CAPABILITIES )
        os << _capabilities;
}

template< class L, class V, class O > 
void View< L, V, O >::deserialize( co::DataIStream& is, 
                                   const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _viewport;
    if( dirtyBits & DIRTY_OBSERVER )
    {
        co::ObjectVersion observer;
        is >> observer;

        if( observer.identifier == co::base::UUID::ZERO )
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
            {
                if( _observer->isMaster( ))
                    _observer->sync();
                else
                    _observer->sync( observer.version );
            }
        }
    }
    if( dirtyBits & DIRTY_OVERDRAW )
        is >> _overdraw;
    if( dirtyBits & DIRTY_FRUSTUM )
        is >> *static_cast< Frustum* >( this );
    if( dirtyBits & DIRTY_MODE )
    {
        Mode mode;
        is >> mode;
        activateMode( mode );
    }
    if( dirtyBits & ( DIRTY_MINCAPS | DIRTY_MAXCAPS ) )
    {
        if( dirtyBits & DIRTY_MINCAPS )
            is >> _minimumCapabilities;
        if( dirtyBits & DIRTY_MAXCAPS )
            is >> _maximumCapabilities;
        updateCapabilities();
    }
    if( dirtyBits & DIRTY_CAPABILITIES )
        is >> _capabilities;
}

template< class L, class V, class O > 
void View< L, V, O >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    if( _layout )
        _layout->setDirty( L::DIRTY_VIEWS );
}

template< class L, class V, class O > 
void View< L, V, O >::changeMode( const Mode mode ) 
{
    if ( _mode == mode )
        return;

    _mode = mode;
    setDirty( DIRTY_MODE );
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
    setMinimumCapabilities( EQ_BIT_NONE );
    setMinimumCapabilities( EQ_BIT_ALL_64 );
    setCapabilities( EQ_BIT_ALL_64 );
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
    if( _overdraw == pixels )
        return;

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
    co::Object* userData = getUserData();
    if( userData && userData->isMaster( ))
        userData->setAutoObsolete( _layout->getConfig()->getLatency( ));
}

template< class L, class V, class O >
std::ostream& operator << ( std::ostream& os, const View< L, V, O >& view )
{
    os << co::base::disableFlush << co::base::disableHeader
       << "view" << std::endl;
    os << "{" << std::endl << co::base::indent;
    
    const std::string& name = view.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const eq::Viewport& vp  = view.getViewport();
    if( vp.isValid( ) && vp != eq::Viewport::FULL )
        os << "viewport " << vp << std::endl;

    if( view.getMode() == View< L, V, O >::MODE_STEREO )
        os << "mode     STEREO" << std::endl; // MONO is default

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

    os << co::base::exdent << "}" << std::endl << co::base::enableHeader
       << co::base::enableFlush;
    return os;
}

template< class L, class V, class O > 
void View< L, V, O >::setMinimumCapabilities( uint64_t bitmask )
{
    if( bitmask == _minimumCapabilities )
        return;

    _minimumCapabilities = bitmask;
    setDirty( DIRTY_MINCAPS );
}

template< class L, class V, class O > 
uint64_t View< L, V, O >::getMinimumCapabilities() const
{
    return _minimumCapabilities;
}

template< class L, class V, class O > 
void View< L, V, O >::setMaximumCapabilities( uint64_t bitmask )
{
    if( bitmask == _maximumCapabilities )
        return;

    _maximumCapabilities = bitmask;
    setDirty( DIRTY_MAXCAPS );
}

template< class L, class V, class O > 
uint64_t View< L, V, O >::getMaximumCapabilities() const
{
    return _maximumCapabilities;
}

template< class L, class V, class O > 
void View< L, V, O >::setCapabilities( uint64_t bitmask )
{
    if( bitmask == _capabilities )
        return;

    _capabilities = bitmask;
    setDirty( DIRTY_CAPABILITIES );
}

template< class L, class V, class O > 
uint64_t View< L, V, O >::getCapabilities() const
{
    return _capabilities;
}

}
}
