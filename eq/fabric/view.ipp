
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
namespace
{
#define _MAKE_ATTR_STRING( attr ) ( std::string("EQ_VIEW_") + #attr )
static std::string _sAttributeStrings[] = {
    _MAKE_ATTR_STRING( SATTR_DEFLECT_HOST ),
    _MAKE_ATTR_STRING( SATTR_DEFLECT_ID )
};
}

template< class L, class V, class O >
View< L, V, O >::View( L* layout )
    : _layout( layout )
    , _observer( 0 )
    , _overdraw( Vector2i::ZERO )
    , _minimumCapabilities( LB_BIT_NONE )
    , _maximumCapabilities( LB_BIT_ALL_64 )
    , _capabilities( LB_BIT_ALL_64 )
    , _equalizers( EQUALIZER_ALL )
    , _modelUnit( EQ_M )
{
    // Note: Views are an exception to the strong structuring, since render
    // client views are multi-buffered (once per pipe) and do not have a parent
    if( layout )
        layout->_addChild( static_cast< V* >( this ));
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class L, class V, class O >
View< L, V, O >::~View()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    if( _layout )
        _layout->_removeChild( static_cast< V* >( this ));
}

template< class L, class V, class O >
View< L, V, O >::BackupData::BackupData()
    : mode( MODE_MONO )
{}

template< class L, class V, class O >
void View< L, V, O >::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_OBSERVER )
        os << co::ObjectVersion( _observer );
    if( dirtyBits & DIRTY_FRUSTUM )
        Frustum::serialize( os );
    if( dirtyBits & DIRTY_VIEWPORT )
        os << _data.viewport;
    if( dirtyBits & DIRTY_OVERDRAW )
        os << _overdraw;
    if( dirtyBits & DIRTY_EQUALIZER )
        os << _equalizer;
    if( dirtyBits & DIRTY_MINCAPS )
        os << _minimumCapabilities;
    if( dirtyBits & DIRTY_MAXCAPS )
        os << _maximumCapabilities;
    if( dirtyBits & DIRTY_CAPABILITIES )
        os << _capabilities;
    if( dirtyBits & DIRTY_MODE )
        os << _data.mode;
    if( dirtyBits & DIRTY_EQUALIZERS )
        os << _equalizers;
    if( dirtyBits & DIRTY_MODELUNIT )
        os << _modelUnit;
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os << co::Array< std::string >( _sAttributes, SATTR_ALL );
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

        if( observer.identifier == 0 )
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
                LBASSERT( layout && layout->getConfig( ));
                layout->getConfig()->find( observer.identifier, &_observer );
                if( _observer )
                    _observer->addView( static_cast< V* >( this ));
                LBASSERT( _observer );
                LBASSERT( _observer->getID() == observer.identifier );
            }
            if( _observer )
            {
                LBASSERT( _observer->getID() == observer.identifier );
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
        is >> _overdraw;
    if( dirtyBits & DIRTY_EQUALIZER )
        is >> _equalizer;
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
    if( dirtyBits & DIRTY_MODE )
    {
        uint32_t mode( 0 );
        is >> mode;
        activateMode( Mode( mode ));
    }
    if( dirtyBits & DIRTY_EQUALIZERS )
        is >> _equalizers;
    if( dirtyBits & DIRTY_MODELUNIT )
        is >> _modelUnit;
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is >> co::Array< std::string >( _sAttributes, SATTR_ALL );
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
    const L* layout = getLayout();
    if( !layout )
        return true; // render client view, active by definition.
    return layout->isActive();
}

template< class L, class V, class O >
bool View< L, V, O >::setModelUnit( const float modelUnit )
{
    if( modelUnit < std::numeric_limits< float >::epsilon() ||
        _modelUnit == modelUnit )
    {
        return false;
    }
    _modelUnit = modelUnit;
    setDirty( DIRTY_MODELUNIT );
    return true;
}

template< class L, class V, class O >
float View< L, V, O >::getModelUnit() const
{
    LBASSERT( _modelUnit > 0.f );
    return _modelUnit;
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
    _overdraw = Vector2i::ZERO;
    _minimumCapabilities = LB_BIT_NONE;
    _maximumCapabilities = LB_BIT_ALL_64;
    _capabilities = LB_BIT_ALL_64;
    _equalizers = EQUALIZER_ALL;
    _modelUnit = EQ_M;

    setDirty( DIRTY_VIEW_BITS );
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
    if( _overdraw == pixels )
        return;

    _overdraw = pixels;
    setDirty( DIRTY_OVERDRAW );
}

template< class L, class V, class O >
void View< L, V, O >::useEqualizer( uint32_t bitmask )
{
    if( _equalizers == bitmask )
        return;
    _equalizers = bitmask;
    setDirty( DIRTY_EQUALIZERS );
}

template< class L, class V, class O >
const Equalizer& View< L, V, O >::getEqualizer() const
{
    return _equalizer;
}

template< class L, class V, class O >
Equalizer& View< L, V, O >::getEqualizer()
{
    setDirty( DIRTY_EQUALIZER );
    return _equalizer;
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
uint32_t View< L, V, O >::getUserDataLatency() const
{
    return static_cast< const V* >( this )->getConfig()->getLatency();
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

template< class L, class V, class O >
const std::string& View< L, V, O >::getSAttribute( const SAttribute attr ) const
{
    LBASSERT( attr < SATTR_ALL );
    return _sAttributes[ attr ];
}

template< class L, class V, class O >
const std::string& View< L, V, O >::getSAttributeString( const SAttribute attr )
{
    return _sAttributeStrings[attr];
}

template< class L, class V, class O >
std::ostream& operator << ( std::ostream& os, const View< L, V, O >& view )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader
       << "view" << std::endl;
    os << "{" << std::endl << lunchbox::indent;

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
              << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
              << lunchbox::enableFlush;
}

}
}
