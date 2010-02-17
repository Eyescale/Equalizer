
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

FrameData::FrameData()
        : _cameraPosition( eq::Vector3f( 0.f, 0.f, 10.f ))
        , _cameraLookAtPoint( eq::Vector3f::ZERO )
        , _cameraUpVector( eq::Vector3f::ZERO )
        , _statistics( false )
{}

void FrameData::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
{
    eq::Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        os << _cameraPosition << _cameraLookAtPoint << _cameraUpVector;
    if( dirtyBits & DIRTY_FLAGS )
        os << _statistics;
}

void FrameData::deserialize( eq::net::DataIStream& is,
                             const uint64_t dirtyBits )
{
    eq::Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        is >> _cameraPosition >> _cameraLookAtPoint >> _cameraUpVector;
    if( dirtyBits & DIRTY_FLAGS )
        is >> _statistics;
}

void FrameData::setCameraPosition( eq::Vector3f cameraPosition )
{
    _cameraPosition = cameraPosition;
    setDirty( DIRTY_CAMERA );
}

void FrameData::setCameraLookAtPoint( eq::Vector3f cameraLookAtPoint )
{
    _cameraLookAtPoint = cameraLookAtPoint;
    setDirty( DIRTY_CAMERA );
}

void FrameData::setCameraUpVector( eq::Vector3f cameraUpVector )
{
    _cameraUpVector = cameraUpVector;
    setDirty( DIRTY_CAMERA );
}

void FrameData::toggleStatistics()
{
    _statistics = !_statistics;
    setDirty( DIRTY_FLAGS );
}

eq::net::Object::ChangeType FrameData::getChangeType() const
{
    return DELTA;
}
