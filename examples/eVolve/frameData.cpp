
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
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

#ifndef M_PI_2
#  define M_PI_2 1.57079632679489661923
#endif

namespace eVolve
{

FrameData::FrameData()
    : _ortho(         false )
    , _colorMode(     COLOR_MODEL )
    , _bgMode(        BG_BLACK )
    , _normalsQuality(NQ_FULL )
    , _statistics(    false )
    , _help(          false )
    , _quality( 1.0f )
{
    reset();
    LBINFO << "New FrameData " << std::endl;
}

void FrameData::reset()
{
    _translation     = eq::Vector3f::ZERO;
    _translation.z() = -2.f;
    _rotation        = eq::Matrix4f();
    _rotation.rotate_x( static_cast<float>( -M_PI_2 ));
    _rotation.rotate_y( static_cast<float>( -M_PI_2 ));

    setDirty( DIRTY_CAMERA );
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

void FrameData::toggleOrtho( )
{
    _ortho = !_ortho;
    setDirty( DIRTY_FLAGS );
}

void FrameData::setOrtho( const bool ortho )
{
    _ortho = ortho;
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
    _rotation.pre_rotate_x( x );
    _rotation.pre_rotate_y( y );

    setDirty( DIRTY_CAMERA );
}

void FrameData::moveCamera( const float x, const float y, const float z )
{
    _translation.x() += x;
    _translation.y() += y;
    _translation.z() += z;

    setDirty( DIRTY_CAMERA );
}

void FrameData::setTranslation(   const eq::Vector3f& translation )
{
    _translation = translation;
    setDirty( DIRTY_CAMERA );
}

void FrameData::setRotation(  const eq::Vector3f& rotation )
{
    _rotation = eq::Matrix4f();
    _rotation.rotate_x( rotation.x() );
    _rotation.rotate_y( rotation.y() );
    _rotation.rotate_z( rotation.z() );
    setDirty( DIRTY_CAMERA );
}

void FrameData::setCurrentViewID( const eq::uint128_t& id )
{
    _currentViewID = id;
    setDirty( DIRTY_VIEW );
}

void FrameData::adjustQuality( const float delta )
{
    _quality += delta;
    _quality = LB_MAX( _quality, 0.1f );
    _quality = LB_MIN( _quality, 1.0f );
    setDirty( DIRTY_FLAGS );
    LBINFO << "Set non-idle image quality to " << _quality << std::endl;
}

void FrameData::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEW )
        os << _currentViewID;

    if( dirtyBits & DIRTY_CAMERA )
        os << _rotation << _translation;

    if( dirtyBits & DIRTY_FLAGS )
        os  << _ortho << _colorMode << _bgMode << _normalsQuality
            << _statistics << _quality << _help;

    if( dirtyBits & DIRTY_MESSAGE )
        os << _message;
}

void FrameData::deserialize( co::DataIStream& is, const uint64_t dirtyBits)
{
    co::Serializable::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEW )
        is >> _currentViewID;

    if( dirtyBits & DIRTY_CAMERA )
        is >> _rotation >> _translation;

    if( dirtyBits & DIRTY_FLAGS )
        is  >> _ortho >> _colorMode >> _bgMode >> _normalsQuality
            >> _statistics  >> _quality >> _help;

    if( dirtyBits & DIRTY_MESSAGE )
        is >> _message;
}

void FrameData::setMessage( const std::string& message )
{
    if( _message == message )
        return;

    _message = message;
    setDirty( DIRTY_MESSAGE );
}

}
