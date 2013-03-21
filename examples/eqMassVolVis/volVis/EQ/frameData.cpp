
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "frameData.h"
#include <algorithm> // min, max

#ifndef M_PI_2
#  define M_PI_2 1.57079632679489661923
#endif

namespace massVolVis
{

FrameData::FrameData()
    : _maxTreeDepth( 9 )
    , _colorMode(     COLOR_MODEL )
    , _bgMode(        BG_WHITE )
    , _pilotMode(     false )
    , _normalsQuality(NQ_FULL )
    , _recording(     false )
    , _statistics(    false )
    , _boundingBoxes( false )
    , _help(          false )
    , _quality( 1.0f )
    , _currentViewId( lunchbox::UUID::ZERO )
    , _renderBudget( 0xFFFFFFFF )
    , _screenShot( 0 )
    , _renderError( 0xFFF )
{
    reset();
    LBINFO << "New FrameData " << std::endl;
}


void FrameData::reset()
{
    _cameraPosition     = eq::Vector3f::ZERO;
    _cameraPosition.z() = -2.f;
    _cameraRotation     = eq::Matrix4f::IDENTITY;
    _modelRotation      = eq::Matrix4f::IDENTITY;
    _modelRotation.rotate_x( static_cast<float>( -M_PI_2 ));
    _modelRotation.rotate_y( static_cast<float>( -M_PI_2 ));
    _cameraSpin         = 0.f;
    _cameraTranslation  = 0.f;

    setDirty( DIRTY_CAMERA );
}


void FrameData::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEW )
        os << _currentViewId;

    if( dirtyBits & DIRTY_CAMERA )
        os << _cameraRotation << _modelRotation << _cameraPosition << _cameraSpin << _cameraTranslation;

    if( dirtyBits & DIRTY_FLAGS )
        os << _statistics << _colorMode << _bgMode << _pilotMode << _normalsQuality
           << _quality    << _help << _boundingBoxes << _recording
           << _renderBudget << _screenShot << _maxTreeDepth << _renderError;

    if( dirtyBits & DIRTY_MESSAGE )
        os << _message;
}


void FrameData::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEW )
        is >> _currentViewId;

    if( dirtyBits & DIRTY_CAMERA )
        is >> _cameraRotation >> _modelRotation >> _cameraPosition >> _cameraSpin >> _cameraTranslation;

    if( dirtyBits & DIRTY_FLAGS )
        is >> _statistics >> _colorMode >> _bgMode >> _pilotMode >> _normalsQuality
           >> _quality >> _help >> _boundingBoxes >> _recording
           >> _renderBudget >> _screenShot >> _maxTreeDepth >> _renderError;

    if( dirtyBits & DIRTY_MESSAGE )
        is >> _message;
}


void FrameData::setMaxTreeDepth( uint8_t value )
{
    value = std::max<uint8_t>( 1u, std::min<uint8_t>( value, 9u ));

    if( value == _maxTreeDepth )
        return;

    _maxTreeDepth = value;
    setDirty( DIRTY_FLAGS );
}


void FrameData::makeScreenshot()
{
    _screenShot++;

    setDirty( DIRTY_FLAGS );
}


void FrameData::increaseBudget()
{
    if( _renderBudget >= 0xFFFFFFFF )
        _renderBudget = 1;
    else
        _renderBudget += 9;

    setDirty( DIRTY_FLAGS );
}


void FrameData::decreaseBudget()
{
    if( _renderBudget >= 0xFFFFFFFF )
        return;
    if( _renderBudget < 10 )
        _renderBudget = 0xFFFFFFFF;
    else
        _renderBudget -= 9;

    setDirty( DIRTY_FLAGS );
}


void FrameData::increaseError()
{
    if( _renderError >= 0xFFF )
        _renderError = 0;

    _renderError += 5;
    if( _renderError > 1000 )
        _renderError = 1000;

    setDirty( DIRTY_FLAGS );
}


void FrameData::decreaseError()
{
    if( _renderError <= 10 )
        _renderError = 0xFFF;

    if( _renderError < 0xFFF )
        _renderError -= 5;

    setDirty( DIRTY_FLAGS );
}

bool FrameData::useRenderingError() const
{
    return _renderError <= 1000;
}


void FrameData::toggleBackground()
{
    _bgMode = static_cast< BackgroundMode >(( _bgMode + 1 ) % BG_ALL );
    setDirty( DIRTY_FLAGS );
}


void FrameData::toggleNormalsQuality()
{
    _normalsQuality = 
        static_cast< NormalsQuality >(( _normalsQuality + 1 ) % NQ_ALL );
    setDirty( DIRTY_FLAGS );
}


void FrameData::toggleColorMode()
{
    _colorMode = static_cast< ColorMode >(( _colorMode + 1 ) % COLOR_ALL );
    setDirty( DIRTY_FLAGS );
}


void FrameData::togglePilotMode()
{
    _pilotMode = !_pilotMode;
    setDirty( DIRTY_FLAGS );
}


void FrameData::toggleBoundingBoxesDisplay()
{
    _boundingBoxes = !_boundingBoxes;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleRecording()
{
    _recording = !_recording;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleStatistics()
{
    _statistics = !_statistics;
    setDirty( DIRTY_FLAGS );
}


void FrameData::toggleHelp()
{
    _help = !_help;
    setDirty( DIRTY_FLAGS );
}


void FrameData::spinCamera( const float x, const float y )
{
    _cameraRotation.pre_rotate_x( x );
    _cameraRotation.pre_rotate_y( y );

    _cameraSpin = sqrt( x*x + y*y );

    setDirty( DIRTY_CAMERA );
}


void FrameData::spinModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f && _cameraSpin == 0.f )
        return;

    _modelRotation.pre_rotate_x( x );
    _modelRotation.pre_rotate_y( y );
    _modelRotation.pre_rotate_z( z );

    _cameraSpin = sqrt( x*x + y*y + z*z );

    setDirty( DIRTY_CAMERA );
}


void FrameData::moveCamera( const float x, const float y, const float z )
{
    eq::Vector3f oldPos = _cameraPosition;

    if( _pilotMode )
    {
        eq::Matrix4f matInverse;
        _cameraRotation.inverse( matInverse );
        eq::Vector4f shift = matInverse * eq::Vector4f( x, y, z, 1 );

        _cameraPosition += shift;
    }
    else
    {
        _cameraPosition.x() += x;
        _cameraPosition.y() += y;
        _cameraPosition.z() += z;
    }
    oldPos -= _cameraPosition;

    _cameraTranslation = sqrt( oldPos.x()*oldPos.x() + oldPos.y()*oldPos.y() + oldPos.z()*oldPos.z() );

    setDirty( DIRTY_CAMERA );
}


void FrameData::resetCameraSpin()
{
    _cameraSpin = 0.f;
    setDirty( DIRTY_CAMERA );
}


void FrameData::setCameraPosition( const eq::Vector3f& position )
{
    _cameraPosition = position;
    setDirty( DIRTY_CAMERA );
}


void FrameData::setCameraRotation( const eq::Vector3f& rotation )
{
    _cameraRotation = eq::Matrix4f::IDENTITY;
    _cameraRotation.rotate_x( rotation.x() );
    _cameraRotation.rotate_y( rotation.y() );
    _cameraRotation.rotate_z( rotation.z() );
    setDirty( DIRTY_CAMERA );
}

void FrameData::setModelRotation( const eq::Vector3f& rotation )
{
    _modelRotation = eq::Matrix4f::IDENTITY;
    _modelRotation.rotate_x( rotation.x() );
    _modelRotation.rotate_y( rotation.y() );
    _modelRotation.rotate_z( rotation.z() );
    setDirty( DIRTY_CAMERA );
}


void FrameData::setCurrentViewId( const eq::uint128_t& id )
{
    _currentViewId = id;
    setDirty( DIRTY_VIEW );
}


void FrameData::adjustQuality( const float delta )
{
    _quality += delta;
    _quality = EQ_MAX( _quality, 0.1f );
    _quality = EQ_MIN( _quality, 1.0f );
    setDirty( DIRTY_FLAGS );
    LBINFO << "Set non-idle image quality to " << _quality << std::endl;
}


void FrameData::setMessage( const std::string& message )
{
    if( _message == message )
        return;

    _message = message;
    setDirty( DIRTY_MESSAGE );
}


}//namespace massVolVis
