
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Petros Kataras <petroskataras@gmail.com>
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

#include "viewData.h"

#include <eq/fabric/axisEvent.h>
#include <eq/fabric/keyEvent.h>
#include <eq/fabric/pointerEvent.h>
#include <eq/eventICommand.h>
#include <eq/view.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace seq
{
ViewData::ViewData( View& view )
    : _view( view )
    , _spinX( 5 )
    , _spinY( 5 )
    , _advance( 0 )
    , _statistics( false )
    , _ortho( false )
{
    moveModel( 0.f, 0.f, -2.f );
}

ViewData::~ViewData()
{}

void ViewData::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_MODELMATRIX )
        os << _modelMatrix;
    if( dirtyBits & DIRTY_STATISTICS )
        os << _statistics;
    if( dirtyBits & DIRTY_ORTHO )
        os << _ortho;
}

void ViewData::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_MODELMATRIX )
        is >> _modelMatrix;
    if( dirtyBits & DIRTY_STATISTICS )
        is >> _statistics;
    if( dirtyBits & DIRTY_ORTHO )
        is >> _ortho;
}

bool ViewData::handleEvent( const eq::EventType, const SizeEvent& )
{
    return true;
}

bool ViewData::handleEvent( const eq::EventType type, const PointerEvent& event )
{
    switch( type )
    {
    case EVENT_CHANNEL_POINTER_BUTTON_RELEASE:
        if( event.buttons == eq::PTR_BUTTON_NONE )
        {
            if( event.button == eq::PTR_BUTTON1 )
            {
                _spinX = event.dy;
                _spinY = event.dx;
                return true;
            }
            if( event.button == eq::PTR_BUTTON2 )
            {
                _advance = -event.dy;
                return true;
            }
        }
        return false;

    case EVENT_CHANNEL_POINTER_MOTION:
        switch( event.buttons )
        {
        case eq::PTR_BUTTON1:
            _spinX = 0;
            _spinY = 0;
            spinModel( -0.005f * event.dy, -0.005f * event.dx, 0.f );
            return true;

        case eq::PTR_BUTTON2:
            _advance = -event.dy;
            moveModel( 0.f, 0.f, .005f * _advance );
            return true;

        case eq::PTR_BUTTON3:
            moveModel( .0005f * event.dx, -.0005f * event.dy, 0.f );
            return true;

        default:
            return false;
        }

    case EVENT_CHANNEL_POINTER_WHEEL:
        moveModel( -0.05f * event.xAxis, 0.f, 0.05f * event.yAxis );
        return true;

    default:
        return false;
    }
}

bool ViewData::handleEvent( const eq::EventType type, const KeyEvent& event )
{
    switch( type )
    {
    case EVENT_KEY_PRESS:
        switch( event.key )
        {
        case 's':
            showStatistics( !getStatistics( ));
            return true;
        case 'o':
            setOrtho( !useOrtho( ));
            return true;
        }
        return false;

    default:
        return false;
    }
}

bool ViewData::handleEvent( const eq::EventType, const AxisEvent& event )
{
    _spinX = 0;
    _spinY = 0;
    _advance = 0;
    spinModel( 0.0001f * event.zRotation, -0.0001f * event.xRotation,
               -0.0001f * event.yRotation );
    moveModel( 0.0001f * event.xAxis, -0.0001f * event.zAxis,
               0.0001f * event.yAxis );
    return true;
}

bool ViewData::handleEvent( const eq::EventType, const ButtonEvent& )
{
    return false;
}

void ViewData::spinModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f )
        return;

    const Vector3f translation = _modelMatrix.getTranslation();
    _modelMatrix.setTranslation( Vector3f( ));
    _modelMatrix.pre_rotate_x( x );
    _modelMatrix.pre_rotate_y( y );
    _modelMatrix.pre_rotate_z( z );
    _modelMatrix.setTranslation( translation);
    setDirty( DIRTY_MODELMATRIX );
}

void ViewData::moveModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f )
        return;

    const float unit = _view.getModelUnit();
    _modelMatrix.setTranslation( _modelMatrix.getTranslation() +
                                 Vector3f( x * unit, y * unit, z * unit ));
    setDirty( DIRTY_MODELMATRIX );
}

void ViewData::showStatistics( const bool on )
{
    if( _statistics == on )
        return;

    _statistics = on;
    setDirty( DIRTY_STATISTICS );
}

void ViewData::setOrtho( const bool on )
{
    if( _ortho == on )
        return;

    _ortho = on;
    setDirty( DIRTY_ORTHO );
}

void ViewData::setModelMatrix( const Matrix4f& matrix )
{
    if( _modelMatrix == matrix )
        return;

    _modelMatrix = matrix;
    setDirty( DIRTY_MODELMATRIX );
}

bool ViewData::update()
{
    if( _spinX == 0 && _spinY == 0 && _advance == 0 )
        return false;

    spinModel( -0.001f * _spinX, -0.001f * _spinY, 0.f );
    moveModel( 0.0f, 0.0f, 0.001f * _advance );
    return true;
}

}
