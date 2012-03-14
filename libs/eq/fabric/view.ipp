
/* Copyright (c) 2008-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "view.h"

#include "equalizerTypes.h"
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
View< L, V, O >::BackupData::BackupData()
        : overdraw( Vector2i::ZERO )
        , tileSize( Vector2i::ZERO )
        , minimumCapabilities( EQ_BIT_NONE )
        , maximumCapabilities( EQ_BIT_ALL_64 )
        , capabilities( EQ_BIT_ALL_64 )
        , mode( MODE_MONO )
        , equalizers( EQUALIZER_ALL )
        , modelUnit( EQ_M )
{}

template< class L, class V, class O > 
void View< L, V, O >::serialize( co::DataOStream& os, const uint64_t dirtyBits)
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_OBSERVER )
        os << co::ObjectVersion( _observer );
    if( dirtyBits & DIRTY_FRUSTUM )
        Frustum::serialize( os );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _data.viewport;
    if( dirtyBits & DIRTY_OVERDRAW )
        os << _data.overdraw;
    if( dirtyBits & DIRTY_TILESIZE )
        os << _data.tileSize;
    if( dirtyBits & DIRTY_MINCAPS )
        os << _data.minimumCapabilities;
    if( dirtyBits & DIRTY_MAXCAPS )
        os << _data.maximumCapabilities;
    if( dirtyBits & DIRTY_CAPABILITIES )
        os << _data.capabilities;
    if( dirtyBits & DIRTY_MODE )
        os << _data.mode;
    if( dirtyBits & DIRTY_EQUALIZERS )
        os << _data.equalizers;
    if( dirtyBits & DIRTY_MODELUNIT )
        os << _data.modelUnit;
}

template< class L, class V, class O > 
void View< L, V, O >::deserialize( co::DataIStream& is, 
                                   const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_OBSERVER )
    {
        co::ObjectVersion observer;
        is >> observer;

        if( observer.identifier == co::base::UUID::ZERO )
        {
            if( _observer )
                _observer->removeView( static_cast< V* >( this ));
            _observer = 0;
        }
        else
        {
            if( !_observer && _layout ) // don't map render client observers yet
            {
                L* layout = getLayout();
                EQASSERT( layout && layout->getConfig( ));
                layout->getConfig()->find( observer.identifier, &_observer );
                if( _observer )
                    _observer->addView( static_cast< V* >( this ));
                EQASSERT( _observer );
                EQASSERT( _observer->getID() == observer.identifier );
            }
            if( _observer )
            {
                EQASSERT( _observer->getID() == observer.identifier );
                if( _observer->isMaster( ))
                    _observer->sync();
                else
                    _observer->sync( observer.version );
            }
        }
    }
    if( dirtyBits & DIRTY_FRUSTUM )
        Frustum::deserialize( is );
    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _data.viewport;
    if( dirtyBits & DIRTY_OVERDRAW )
        is >> _data.overdraw;
    if( dirtyBits & DIRTY_TILESIZE )
        is >> _data.tileSize;
    if( dirtyBits & ( DIRTY_MINCAPS | DIRTY_MAXCAPS ) )
    {
        if( dirtyBits & DIRTY_MINCAPS )
            is >> _data.minimumCapabilities;
        if( dirtyBits & DIRTY_MAXCAPS )
            is >> _data.maximumCapabilities;
        updateCapabilities();
    }
    if( dirtyBits & DIRTY_CAPABILITIES )
        is >> _data.capabilities;
    if( dirtyBits & DIRTY_MODE )
    {
        Mode mode;
        is >> mode;
        activateMode( mode );
    }
    if( dirtyBits & DIRTY_EQUALIZERS )
        is >> _data.equalizers;
    if( dirtyBits & DIRTY_MODELUNIT )
        is >> _data.modelUnit;
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
    if ( _data.mode == mode )
        return;

    _data.mode = mode;
    setDirty( DIRTY_MODE );
}

template< class L, class V, class O > 
bool View< L, V, O >::isActive() const
{
    return getLayout()->isActive();
}

template< class L, class V, class O >
void View< L, V, O >::setModelUnit( const float modelUnit )
{
    if( modelUnit < std::numeric_limits< float >::epsilon() ||
        _data.modelUnit == modelUnit )
    {
        return;
    }
    _data.modelUnit = modelUnit;
    setDirty( DIRTY_MODELUNIT );
}

template< class L, class V, class O >
float View< L, V, O >::getModelUnit() const
{
    EQASSERT( _data.modelUnit > 0.f );
    return _data.modelUnit;
}

template< class L, class V, class O > 
void View< L, V, O >::setViewport( const Viewport& viewport )
{
    _data.viewport = viewport;
    setDirty( DIRTY_VIEWPORT );
}

template< class L, class V, class O > void View< L, V, O >::backup()
{
    _backup = _data;
    Frustum::backup();
    Object::backup();
}

template< class L, class V, class O > void View< L, V, O >::restore()
{
    Object::restore();
    Frustum::restore();
    _data = _backup;
    setDirty( DIRTY_VIEWPORT | DIRTY_OVERDRAW | DIRTY_FRUSTUM | DIRTY_MODE |
              DIRTY_MINCAPS | DIRTY_MAXCAPS | DIRTY_CAPABILITIES ); // TODO: add new bits?
}

template< class L, class V, class O > 
void View< L, V, O >::setObserver( O* observer )
{
    if( _observer == observer )
        return;

    if( _observer )
        _observer->removeView( static_cast< V* >( this ));
    _observer = observer;
    if( _observer )
        _observer->addView( static_cast< V* >( this ));
    setDirty( DIRTY_OBSERVER );
}

template< class L, class V, class O > 
const Viewport& View< L, V, O >::getViewport() const
{
    return _data.viewport;
}

template< class L, class V, class O > 
void View< L, V, O >::setOverdraw( const Vector2i& pixels )
{
    if( _data.overdraw == pixels )
        return;

    _data.overdraw = pixels;
    setDirty( DIRTY_OVERDRAW );
}


template< class L, class V, class O > 
void View< L, V, O >::useEqualizer( uint32_t bitmask )
{
    if( _data.equalizers == bitmask )
        return;
    _data.equalizers = bitmask;
    setDirty( DIRTY_EQUALIZERS );
}

template< class L, class V, class O > 
void View< L, V, O >::setTileSize( const Vector2i& size )
{
    if( _data.tileSize == size || size.x() < 1 || size.y() < 1 )
        return;
    
    _data.tileSize = size;
    setDirty( DIRTY_TILESIZE );
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
uint32_t View< L, V, O >::getUserDataLatency() const
{
    return static_cast< const V* >( this )->getConfig()->getLatency();
}

template< class L, class V, class O > 
void View< L, V, O >::setMinimumCapabilities( uint64_t bitmask )
{
    if( bitmask == _data.minimumCapabilities )
        return;

    _data.minimumCapabilities = bitmask;
    setDirty( DIRTY_MINCAPS );
}

template< class L, class V, class O > 
uint64_t View< L, V, O >::getMinimumCapabilities() const
{
    return _data.minimumCapabilities;
}

template< class L, class V, class O > 
void View< L, V, O >::setMaximumCapabilities( uint64_t bitmask )
{
    if( bitmask == _data.maximumCapabilities )
        return;

    _data.maximumCapabilities = bitmask;
    setDirty( DIRTY_MAXCAPS );
}

template< class L, class V, class O > 
uint64_t View< L, V, O >::getMaximumCapabilities() const
{
    return _data.maximumCapabilities;
}

template< class L, class V, class O > 
void View< L, V, O >::setCapabilities( uint64_t bitmask )
{
    if( bitmask == _data.capabilities )
        return;

    _data.capabilities = bitmask;
    setDirty( DIRTY_CAPABILITIES );
}

template< class L, class V, class O > 
uint64_t View< L, V, O >::getCapabilities() const
{
    return _data.capabilities;
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

    const Viewport& vp = view.getViewport();
    if( vp.isValid( ) && vp != Viewport::FULL )
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

    return os << static_cast< const Frustum& >( view )
              << co::base::exdent << "}" << std::endl << co::base::enableHeader
              << co::base::enableFlush;
}

}
}
