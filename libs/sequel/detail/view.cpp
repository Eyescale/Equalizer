
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "config.h"
#include "pipe.h"

#include <eq/sequel/application.h>
#include <eq/sequel/renderer.h>
#include <eq/sequel/viewData.h>
#include <eq/config.h>
#include <eq/configEvent.h>

namespace seq
{
namespace detail
{

View::View( eq::Layout* parent )
        : eq::View( parent ) 
        , _spinX( 5 )
        , _spinY( 5 )
        , _advance( 0 )
{}

View::~View()
{
}

Config* View::getConfig()
{
    return static_cast< Config* >( eq::View::getConfig( ));
}

Pipe* View::getPipe()
{
    return static_cast< Pipe* >( eq::View::getPipe( ));
}

ViewData* View::getViewData()
{
    return static_cast< ViewData* >( eq::View::getUserData( ));
}

void View::notifyAttach()
{
    eq::View::notifyAttach();
    Pipe* pipe = getPipe();

    if( pipe ) // render client view
        setUserData( pipe->getRenderer()->createViewData( ));
    else // application view
        setUserData( getConfig()->getApplication()->createViewData( ));
}

void View::notifyDetached()
{
    ViewData* data = getViewData();
    setUserData( 0 );

    if( data )
    {
        Pipe* pipe = getPipe();

        if( pipe ) // render client view
            pipe->getRenderer()->destroyViewData( data );
        else // application view
            getConfig()->getApplication()->destroyViewData( data );
    }

    eq::View::notifyDetached();
}

void View::updateData()
{
    ViewData* data = getViewData();
    EQASSERT( data );
    if( !data )
        return;

    data->spinModel( -0.001f * _spinX, -0.001f * _spinY, 0.f );
    data->moveModel( 0.0f, 0.0f, 0.001f * _advance );
}

bool View::handleEvent( const eq::ConfigEvent* event )
{
    ViewData* data = getViewData();
    EQASSERT( data );
    if( !data )
        return false;

    switch( event->data.type )
    {
      case eq::Event::CHANNEL_POINTER_BUTTON_RELEASE:
      {
          const eq::PointerEvent& releaseEvent = 
              event->data.pointerButtonRelease;
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
          switch( event->data.pointerMotion.buttons )
          {
            case eq::PTR_BUTTON1:
                _spinX = 0;
                _spinY = 0;
                data->spinModel( -0.005f * event->data.pointerMotion.dy,
                                 -0.005f * event->data.pointerMotion.dx, 0.f );
                return true;

            case eq::PTR_BUTTON2:
                _advance = -event->data.pointerMotion.dy;
                data->moveModel( 0.f, 0.f, .005f * _advance );
                return true;

            case eq::PTR_BUTTON3:
                data->moveModel(  .0005f * event->data.pointerMotion.dx,
                                 -.0005f * event->data.pointerMotion.dy,
                                   0.f );
                return true;

            default:
                return false;
          }

      case eq::Event::WINDOW_POINTER_WHEEL:
          data->moveModel( -0.05f * event->data.pointerWheel.yAxis, 0.f,
                            0.05f * event->data.pointerWheel.xAxis );
          return true;

      case eq::Event::MAGELLAN_AXIS:
          _spinX = 0;
          _spinY = 0;
          _advance = 0;
          data->spinModel(  0.0001f * event->data.magellan.zRotation,
                           -0.0001f * event->data.magellan.xRotation,
                           -0.0001f * event->data.magellan.yRotation );
          data->moveModel(  0.0001f * event->data.magellan.xAxis,
                           -0.0001f * event->data.magellan.zAxis,
                            0.0001f * event->data.magellan.yAxis );
          return true;

      default:
          return false;
    }
}

}
}
