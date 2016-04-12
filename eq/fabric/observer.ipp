
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "observer.h"

#include "leafVisitor.h"
#include "log.h"
#include "paths.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{

template< typename C, typename O >
Observer< C, O >::Observer( C* config )
        : _config( config )
{
    LBASSERT( config );
    config->_addObserver( static_cast< O* >( this ));

    const float eyeBase_2 = config->getFAttribute( C::FATTR_EYE_BASE ) * .5f;
    setEyePosition( EYE_LEFT, Vector3f( -eyeBase_2, 0.f, 0.f ));
    setEyePosition( EYE_CYCLOP, Vector3f::ZERO );
    setEyePosition( EYE_RIGHT, Vector3f( eyeBase_2, 0.f, 0.f ));
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< typename C, typename O >
Observer< C, O >::~Observer()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    _config->_removeObserver( static_cast< O* >( this ));
}

template< typename C, typename O >
Observer< C, O >::BackupData::BackupData()
    : focusDistance( 1.f )
    , focusMode( FOCUSMODE_FIXED )
    , camera( OFF )
{
    for( size_t i = 0; i < NUM_EYES; ++i )
        eyePosition[ i ] = Vector3f::ZERO;
    eyePosition[ EYE_LEFT_BIT ].x() = -.05f;
    eyePosition[ EYE_RIGHT_BIT ].x() = .05f;
}

template< typename C, typename O >
void Observer< C, O >::backup()
{
    Object::backup();
    _backup = _data;
}

template< typename C, typename O >
void Observer< C, O >::restore()
{
    _data = _backup;
    Object::restore();
    setDirty( DIRTY_EYE_POSITION | DIRTY_HEAD | DIRTY_FOCUS );
}

template< typename C, typename O >
void Observer< C, O >::serialize( co::DataOStream& os,
                                  const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_HEAD )
        os << _data.headMatrix;
    if( dirtyBits & DIRTY_EYE_POSITION )
        for( size_t i = 0; i < NUM_EYES; ++i )
            os << _data.eyePosition[i];
    if( dirtyBits & DIRTY_FOCUS )
        os << _data.focusDistance << _data.focusMode;
    if( dirtyBits & DIRTY_TRACKER )
        os << _data.camera << _data.vrpnTracker;
}

template< typename C, typename O >
void Observer< C, O >::deserialize( co::DataIStream& is,
                                    const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_HEAD )
        is >> _data.headMatrix;
    if( dirtyBits & DIRTY_EYE_POSITION )
        for( size_t i = 0; i < NUM_EYES; ++i )
            is >> _data.eyePosition[i];
    if( dirtyBits & DIRTY_FOCUS )
        is >> _data.focusDistance >> _data.focusMode;
    if( dirtyBits & DIRTY_TRACKER )
        is >> _data.camera >> _data.vrpnTracker;
}

template< typename C, typename O >
void Observer< C, O >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _config->setDirty( C::DIRTY_OBSERVERS );
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( Visitor& visitor )
{
    return visitor.visit( static_cast< O* >( this ));
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( Visitor& visitor ) const
{
    return visitor.visit( static_cast< const O* >( this ));
}

template< typename C, typename O >
ObserverPath Observer< C, O >::getPath() const
{
    const std::vector< O* >&  observers = _config->getObservers();
    typename std::vector< O* >::const_iterator i = std::find( observers.begin(),
                                                              observers.end(),
                                                              this );
    LBASSERT( i != observers.end( ));

    ObserverPath path;
    path.observerIndex = std::distance( observers.begin(), i );
    return path;
}

template< typename C, typename O >
void Observer< C, O >::setEyePosition( const Eye eye, const Vector3f& pos )
{
    LBASSERT( lunchbox::getIndexOfLastBit( eye ) <= EYE_LAST );
    Vector3f& position = _data.eyePosition[ lunchbox::getIndexOfLastBit( eye )];
    if( position == pos )
        return;

    position = pos;
    setDirty( DIRTY_EYE_POSITION );
}

template< typename C, typename O >
const Vector3f& Observer< C, O >::getEyePosition( const Eye eye ) const
{
    LBASSERT( lunchbox::getIndexOfLastBit( eye ) <= EYE_LAST );
    return _data.eyePosition[ lunchbox::getIndexOfLastBit( eye )];
}

template< typename C, typename O >
void Observer< C, O >::setFocusDistance( const float focusDistance )
{
    if( _data.focusDistance == focusDistance )
        return;

    _data.focusDistance = focusDistance;
    setDirty( DIRTY_FOCUS );
}

template< typename C, typename O >
void Observer< C, O >::setFocusMode( const FocusMode focusMode )
{
    if( _data.focusMode == focusMode )
        return;

    _data.focusMode = focusMode;
    setDirty( DIRTY_FOCUS );
}

template< typename C, typename O >
void Observer< C, O >::setOpenCVCamera( const int32_t camera )
{
    if( _data.camera == camera )
        return;

    _data.camera = camera;
    setDirty( DIRTY_TRACKER );
}

template< typename C, typename O >
void Observer< C, O >::setVRPNTracker( const std::string& tracker )
{
    if( _data.vrpnTracker == tracker )
        return;

    _data.vrpnTracker = tracker;
    setDirty( DIRTY_TRACKER );
}

template< typename C, typename O >
bool Observer< C, O >::setHeadMatrix( const Matrix4f& matrix )
{
    if( _data.headMatrix == matrix )
        return false;

    _data.headMatrix = matrix;
    setDirty( DIRTY_HEAD );
    return true;
}

template< typename C, typename O >
std::ostream& operator << ( std::ostream& os, const Observer< C, O >& observer )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "observer"
       << std::endl;
    os << "{" << std::endl << lunchbox::indent;

    const std::string& name = observer.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    os << "eye_left       " << observer.getEyePosition( EYE_LEFT ) << std::endl
       << "eye_cyclop     " << observer.getEyePosition( EYE_CYCLOP ) <<std::endl
       << "eye_right      " << observer.getEyePosition( EYE_RIGHT ) << std::endl
       << "focus_distance " << observer.getFocusDistance() << std::endl
       << "focus_mode     " << observer.getFocusMode() << std::endl
       << "opencv_camera  " << IAttribute( observer.getOpenCVCamera( ))
       << std::endl;
    if( !observer.getVRPNTracker().empty( ))
        os << "vrpn_tracker   \"" << observer.getVRPNTracker() << "\""
           << std::endl;
    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;
    return os;
}

}
}
