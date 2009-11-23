
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "frameData.h"

namespace eqPly
{

FrameData::FrameData()
        : _modelID( EQ_ID_INVALID )
        , _renderMode( mesh::RENDER_MODE_DISPLAY_LIST )
        , _colorMode( COLOR_MODEL )
        , _ortho( false )
        , _statistics( false )
        , _help( false )
        , _wireframe( false )
        , _pilotMode( false )
        , _idleMode( false )
        , _currentViewID( EQ_ID_INVALID )
{
    reset();
    EQINFO << "New FrameData " << std::endl;
}

void FrameData::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
{
    eq::Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        os << _translation << _rotation << _modelRotation;
    if( dirtyBits & DIRTY_FLAGS )
        os << _modelID << _renderMode << _colorMode << _ortho << _statistics
           << _help << _wireframe << _pilotMode << _idleMode;
    if( dirtyBits & DIRTY_VIEW )
        os << _currentViewID;
    if( dirtyBits & DIRTY_MESSAGE )
        os << _message;
}

void FrameData::deserialize( eq::net::DataIStream& is,
                             const uint64_t dirtyBits )
{
    eq::Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        is >> _translation >> _rotation >> _modelRotation;
    if( dirtyBits & DIRTY_FLAGS )
        is >> _modelID >> _renderMode >> _colorMode >> _ortho >> _statistics
           >> _help >> _wireframe >> _pilotMode >> _idleMode;
    if( dirtyBits & DIRTY_VIEW )
        is >> _currentViewID;
    if( dirtyBits & DIRTY_MESSAGE )
        is >> _message;
}

void FrameData::setModelID( const uint32_t id )
{
    if( _modelID == id )
        return;

    _modelID = id;
    setDirty( DIRTY_FLAGS );
}

void FrameData::setColorMode( const ColorMode mode )
{
    _colorMode = mode;
    setDirty( DIRTY_FLAGS );
}

void FrameData::setRenderMode( const mesh::RenderMode mode )
{
    _renderMode = mode;
    setDirty( DIRTY_FLAGS );
}

void FrameData::setIdle( const bool idleMode )
{
    if( _idleMode == idleMode )
        return;

    _idleMode = idleMode;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleOrtho()
{
    _ortho = !_ortho;
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

void FrameData::toggleWireframe()
{
    _wireframe = !_wireframe;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleColorMode()
{
    _colorMode = static_cast< ColorMode >(( _colorMode + 1) % COLOR_ALL );
    setDirty( DIRTY_FLAGS );
}

void FrameData::togglePilotMode()
{
    _pilotMode = !_pilotMode;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleRenderMode()
{
    _renderMode = static_cast< mesh::RenderMode >(
        ( _renderMode + 1) % mesh::RENDER_MODE_ALL );

    EQINFO << "Switched to " << _renderMode << std::endl;
    setDirty( DIRTY_FLAGS );
}

void FrameData::spinCamera( const float x, const float y )
{
    if( x == 0.f && y == 0.f )
        return;

    _rotation.pre_rotate_x( x );
    _rotation.pre_rotate_y( y );
    setDirty( DIRTY_CAMERA );
}

void FrameData::spinModel( const float x, const float y )
{
    if( x == 0.f && y == 0.f )
        return;

    _modelRotation.pre_rotate_x( x );
    _modelRotation.pre_rotate_y( y );
    setDirty( DIRTY_CAMERA );
}

void FrameData::spinModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f )
        return;

    _modelRotation.pre_rotate_x( x );
    _modelRotation.pre_rotate_y( y );
    _modelRotation.pre_rotate_z( z );
    setDirty( DIRTY_CAMERA );
}

void FrameData::moveCamera( const float x, const float y, const float z )
{
    if( _pilotMode )
    {
        eq::Matrix4f matInverse;
        compute_inverse( _rotation, matInverse );
        eq::Vector4f shift = matInverse * eq::Vector4f( x, y, z, 1 );

        _translation += shift;
    }
    else
    {
        _translation.x() += x;
        _translation.y() += y;
        _translation.z() += z;
    }

    setDirty( DIRTY_CAMERA );
}

void FrameData::setCameraPosition( const float x, const float y, const float z )
{
    _translation.x() = x;
    _translation.y() = y;
    _translation.z() = z;
    setDirty( DIRTY_CAMERA );
}

void FrameData::setTranslation( const eq::Vector3f& translation )
{
    _translation = translation;
    setDirty( DIRTY_CAMERA );
}

void FrameData::setRotation( const eq::Vector3f& rotation )
{
    _rotation = eq::Matrix4f::IDENTITY;
    _rotation.rotate_x( rotation.x() );
    _rotation.rotate_y( rotation.y() );
    _rotation.rotate_z( rotation.z() );
    setDirty( DIRTY_CAMERA );
}

void FrameData::setModelRotation(  const eq::Vector3f& rotation )
{
    _modelRotation = eq::Matrix4f::IDENTITY;
    _modelRotation.rotate_x( rotation.x() );
    _modelRotation.rotate_y( rotation.y() );
    _modelRotation.rotate_z( rotation.z() );
    setDirty( DIRTY_CAMERA );
}

void FrameData::reset()
{
    _translation   = eq::Vector3f::ZERO;
    _translation.z() = -2.f;
    _rotation      = eq::Matrix4f::IDENTITY;
    _modelRotation = eq::Matrix4f::IDENTITY;
    _modelRotation.rotate_x( static_cast<float>( -M_PI_2 ));
    _modelRotation.rotate_y( static_cast<float>( -M_PI_2 ));
    setDirty( DIRTY_CAMERA );
}

void FrameData::setCurrentViewID( const uint32_t id )
{
    _currentViewID = id;
    setDirty( DIRTY_VIEW );
}

void FrameData::setMessage( const std::string& message )
{
    if( _message == message )
        return;

    _message = message;
    setDirty( DIRTY_MESSAGE );
}

}

