
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

#ifndef EQ_2_0_API
#  include <eq/configEvent.h>
#endif
#include <eq/fabric/event.h>
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
    co::Serializable::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_MODELMATRIX )
        os << _modelMatrix;
    if( dirtyBits & DIRTY_STATISTICS )
        os << _statistics;
    if( dirtyBits & DIRTY_ORTHO )
        os << _ortho;
}

void ViewData::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_MODELMATRIX )
        is >> _modelMatrix;
    if( dirtyBits & DIRTY_STATISTICS )
        is >> _statistics;
    if( dirtyBits & DIRTY_ORTHO )
        is >> _ortho;
}
#ifndef EQ_2_0_API
bool ViewData::handleEvent( const eq::ConfigEvent* event )
{
    return _handleEvent( event->data );
}
#endif

bool ViewData::handleEvent( eq::EventICommand command )
{
    const eq::Event& event = command.read< eq::Event >();
    return _handleEvent( event );
}

bool ViewData::_handleEvent( const eq::Event& event )
{
    switch( event.type )
    {
      case eq::Event::CHANNEL_POINTER_BUTTON_RELEASE:
      {
          const eq::PointerEvent& releaseEvent =
              event.pointerButtonRelease;
          if( releaseEvent.buttons == eq::PTR_BUTTON_NONE )
          {
              if( releaseEvent.button == eq::PTR_BUTTON1 )
              {
                  _spinX = releaseEvent.dy;
                  _spinY = releaseEvent.dx;
                  return true;
              }
              if( releaseEvent.button == eq::PTR_BUTTON2 )
              {
                  _advance = -releaseEvent.dy;
                  return true;
              }
          }
          return false;
      }
      case eq::Event::CHANNEL_POINTER_MOTION:
          switch( event.pointerMotion.buttons )
          {
            case eq::PTR_BUTTON1:
                _spinX = 0;
                _spinY = 0;
                spinModel( -0.005f * event.pointerMotion.dy,
                           -0.005f * event.pointerMotion.dx, 0.f );
                return true;

            case eq::PTR_BUTTON2:
                _advance = -event.pointerMotion.dy;
                moveModel( 0.f, 0.f, .005f * _advance );
                return true;

            case eq::PTR_BUTTON3:
                moveModel(  .0005f * event.pointerMotion.dx,
                           -.0005f * event.pointerMotion.dy, 0.f );
                return true;

            default:
                return false;
          }

      case eq::Event::CHANNEL_POINTER_WHEEL:
          moveModel( -0.05f * event.pointerWheel.xAxis, 0.f,
                      0.05f * event.pointerWheel.yAxis );
          return true;

      case eq::Event::MAGELLAN_AXIS:
          _spinX = 0;
          _spinY = 0;
          _advance = 0;
          spinModel(  0.0001f * event.magellan.zRotation,
                     -0.0001f * event.magellan.xRotation,
                     -0.0001f * event.magellan.yRotation );
          moveModel(  0.0001f * event.magellan.xAxis,
                     -0.0001f * event.magellan.zAxis,
                      0.0001f * event.magellan.yAxis );
          return true;

      case eq::Event::KEY_PRESS:
          switch( event.keyPress.key )
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

void ViewData::spinModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f )
        return;

    const Vector3f translation = _modelMatrix.getTranslation();
    _modelMatrix.setTranslation( Vector3f::ZERO );
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
