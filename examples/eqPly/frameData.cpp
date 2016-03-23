
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "frameData.h"

namespace eqPly
{

FrameData::FrameData()
    : _renderMode( triply::RENDER_MODE_DISPLAY_LIST )
    , _colorMode( COLOR_MODEL )
    , _quality( 1.0f )
    , _ortho( false )
    , _statistics( false )
    , _help( false )
    , _wireframe( false )
    , _pilotMode( false )
    , _idle( false )
    , _compression( true )
{
    reset();
}

void FrameData::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        os << _position << _rotation << _modelRotation;
    if( dirtyBits & DIRTY_FLAGS )
        os << _modelID << _renderMode << _colorMode << _quality << _ortho
           << _statistics << _help << _wireframe << _pilotMode << _idle
           << _compression;
    if( dirtyBits & DIRTY_VIEW )
        os << _currentViewID;
    if( dirtyBits & DIRTY_MESSAGE )
        os << _message;
}

void FrameData::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        is >> _position >> _rotation >> _modelRotation;
    if( dirtyBits & DIRTY_FLAGS )
        is >> _modelID >> _renderMode >> _colorMode >> _quality >> _ortho
           >> _statistics >> _help >> _wireframe >> _pilotMode >> _idle
           >> _compression;
    if( dirtyBits & DIRTY_VIEW )
        is >> _currentViewID;
    if( dirtyBits & DIRTY_MESSAGE )
        is >> _message;
}

void FrameData::setModelID( const eq::uint128_t& id )
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

void FrameData::setRenderMode( const triply::RenderMode mode )
{
    _renderMode = mode;
    setDirty( DIRTY_FLAGS );
}

void FrameData::setIdle( const bool idle )
{
    if( _idle == idle )
        return;

    _idle = idle;
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

void FrameData::adjustQuality( const float delta )
{
    _quality += delta;
    _quality = LB_MAX( _quality, 0.1f );
    _quality = LB_MIN( _quality, 1.0f );
    setDirty( DIRTY_FLAGS );
    LBINFO << "Set non-idle image quality to " << _quality << std::endl;
}

void FrameData::togglePilotMode()
{
    _pilotMode = !_pilotMode;
    setDirty( DIRTY_FLAGS );
}

triply::RenderMode FrameData::toggleRenderMode()
{
    _renderMode = static_cast< triply::RenderMode >(
        ( _renderMode + 1) % triply::RENDER_MODE_ALL );

    setDirty( DIRTY_FLAGS );
    return _renderMode;
}

void FrameData::toggleCompression()
{
    _compression = !_compression;
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
        const eq::Matrix4f& matInverse = _rotation.inverse();
        const eq::Vector4f shift = matInverse * eq::Vector4f( x, y, z, 1 );
        _position += shift;
    }
    else
    {
        _position.x() += x;
        _position.y() += y;
        _position.z() += z;
    }

    setDirty( DIRTY_CAMERA );
}

void FrameData::setCameraPosition( const eq::Vector3f& position )
{
    _position = position;
    setDirty( DIRTY_CAMERA );
}

void FrameData::setRotation( const eq::Vector3f& rotation )
{
    _rotation = eq::Matrix4f();
    _rotation.rotate_x( rotation.x() );
    _rotation.rotate_y( rotation.y() );
    _rotation.rotate_z( rotation.z() );
    setDirty( DIRTY_CAMERA );
}

void FrameData::setModelRotation(  const eq::Vector3f& rotation )
{
    _modelRotation = eq::Matrix4f();
    _modelRotation.rotate_x( rotation.x( ));
    _modelRotation.rotate_y( rotation.y( ));
    _modelRotation.rotate_z( rotation.z( ));
    setDirty( DIRTY_CAMERA );
}

void FrameData::reset()
{
    eq::Matrix4f model = eq::Matrix4f();
    model.rotate_x( static_cast<float>( -M_PI_2 ));
    model.rotate_y( static_cast<float>( -M_PI_2 ));

    if( _position == eq::Vector3f( 0.f, 0.f, -2.f ) &&
        _rotation == eq::Matrix4f() && _modelRotation == model )
    {
        _position.z() = 0.f;
    }
    else
    {
        _position = eq::Vector3f( 0.f, 0.f, -2.f );
        _rotation = eq::Matrix4f();
        _modelRotation = model;
    }
    setDirty( DIRTY_CAMERA );
}

void FrameData::setCurrentViewID( const eq::uint128_t& id )
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
