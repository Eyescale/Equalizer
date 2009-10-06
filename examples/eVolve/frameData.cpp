
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *               2007-2009, Maxim Makhinya
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

namespace eVolve
{

FrameData::FrameData()
    : _ortho(         false )
    , _statistics(    false )
    , _help(          false )
    , _currentViewID( EQ_ID_INVALID )
{
    reset();
    EQINFO << "New FrameData " << std::endl;
}

void FrameData::reset()
{
    _translation     = eq::Vector3f::ZERO;
    _translation.z() = -2.f;
    _rotation        = eq::Matrix4f::IDENTITY;
    _rotation.rotate_x( static_cast<float>( -M_PI_2 ));
    _rotation.rotate_y( static_cast<float>( -M_PI_2 ));

    setDirty( DIRTY_CAMERA );
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
    _rotation = eq::Matrix4f::IDENTITY;
    _rotation.rotate_x( rotation.x() );
    _rotation.rotate_y( rotation.y() );
    _rotation.rotate_z( rotation.z() );
    setDirty( DIRTY_CAMERA );
}

void FrameData::setCurrentViewID( const uint32_t id )
{
    _currentViewID = id;
    setDirty( DIRTY_VIEW );
}

void FrameData::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
{
    eq::Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_VIEW )
        os << _currentViewID;

    if( dirtyBits & DIRTY_CAMERA )
        os << _rotation << _translation;

    if( dirtyBits & DIRTY_FLAGS )
        os << _ortho << _statistics << _help;

    if( dirtyBits & DIRTY_MESSAGE )
        os << _message;
}

void FrameData::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits)
{
    eq::Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_VIEW )
        is >> _currentViewID;

    if( dirtyBits & DIRTY_CAMERA )
        is >> _rotation >> _translation;

    if( dirtyBits & DIRTY_FLAGS )
        is >> _ortho >> _statistics >> _help;

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
