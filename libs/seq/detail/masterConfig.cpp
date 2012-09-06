
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "masterConfig.h"

#include "objectMap.h"
#include "view.h"

#include <seq/application.h>
#ifndef EQ_2_0_API
#  include <eq/client/configEvent.h>
#endif
#include <eq/fabric/configVisitor.h>
#include <eq/client/event.h>
#include <eq/client/eventCommand.h>

namespace seq
{
namespace detail
{
MasterConfig::MasterConfig( eq::ServerPtr parent )
        : Config( parent )
        , _currentViewID( uint128_t::ZERO )
        , _redraw( false )
{}

MasterConfig::~MasterConfig()
{}

bool MasterConfig::init()
{
    LBASSERT( !_objects );
    _objects = new ObjectMap( *this, *getApplication( ));

    co::Object* initData = getInitData();
    if( initData )
        LBCHECK( _objects->register_( initData, OBJECTTYPE_INITDATA ));
    _objects->setInitData( initData );

    LBCHECK( registerObject( _objects ));

    if( !eq::Config::init( _objects->getID( )))
    {
        LBWARN << "Error during initialization: " << getError() << std::endl;
        exit();
        return false;
    }
    if( getError( ))
        LBWARN << "Error during initialization: " << getError() << std::endl;

    _redraw = true;
    return true;
}

bool MasterConfig::exit()
{
    const bool retVal = eq::Config::exit();

    if( _objects )
        deregisterObject( _objects );
    delete _objects;
    _objects = 0;

    return retVal;
}

bool MasterConfig::run( co::Object* frameData )
{
    LBASSERT( _objects );
    if( frameData )
        LBCHECK( _objects->register_( frameData, OBJECTTYPE_FRAMEDATA ));
    _objects->setFrameData( frameData );

    seq::Application* const app = getApplication();
    while( isRunning( ))
    {
        startFrame();
        if( getError( ))
            LBWARN << "Error during frame start: " << getError() << std::endl;
        finishFrame();

        while( !needRedraw( )) // wait for an event requiring redraw
        {
            if( app->hasCommands( )) // execute non-critical pending commands
            {
                app->processCommand();
                handleEvents(); // non-blocking
            }
            else  // no pending commands, block on user event
            {
                const eq::EventCommand& event = getNextEvent();
                if( !handleEvent( event ))
                    LBVERB << "Unhandled " << event << std::endl;
            }
        }
        handleEvents(); // process all pending events
    }
    finishAllFrames();
    return true;
}

uint32_t MasterConfig::startFrame()
{
    _redraw = false;
    return eq::Config::startFrame( _objects->commit( ));
}

namespace
{
class ViewUpdateVisitor : public eq::ConfigVisitor
{
public:
    ViewUpdateVisitor( bool &redraw ) : _redraw( redraw ) {}
    virtual~ ViewUpdateVisitor() {}

    virtual eq::VisitorResult visit( eq::View* v )
        {
            View* view = static_cast< View* >( v );
            if( view->updateData( ))
            {
                LBVERB << "Redraw: new view data" << std::endl;
                _redraw = true;
            }
            return eq::TRAVERSE_CONTINUE;
        }

private:
    bool& _redraw;
};
}
#ifndef EQ_2_0_API
bool MasterConfig::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
      case eq::Event::CHANNEL_POINTER_BUTTON_PRESS:
          _currentViewID = event->data.context.view.identifier;
          return true;

      case eq::Event::KEY_PRESS:
      case eq::Event::KEY_RELEASE:
          if( Config::handleEvent( event ))
          {
              _redraw = true;
              LBVERB << "Redraw: requested by eq::Config" << std::endl;
          }
          // no break;
      case eq::Event::CHANNEL_POINTER_BUTTON_RELEASE:
      case eq::Event::CHANNEL_POINTER_MOTION:
      case eq::Event::WINDOW_POINTER_WHEEL:
      case eq::Event::MAGELLAN_AXIS:
      {
          if( _currentViewID == uint128_t::ZERO )
              return false;

          View* view = static_cast<View*>( find<eq::View>( _currentViewID ));
          if( view->handleEvent( event ))
          {
              _redraw = true;
              LBVERB << "Redraw: requested by view event handler" << std::endl;
          }
          return true;
      }

      case eq::Event::STATISTIC:
      {
          Config::handleEvent( event );
          if( event->data.statistic.type != eq::Statistic::CONFIG_FINISH_FRAME )
              return false;

          ViewUpdateVisitor viewUpdate( _redraw );
          accept( viewUpdate );
          return _redraw;
      }

      case eq::Event::WINDOW_EXPOSE:
      case eq::Event::WINDOW_RESIZE:
      case eq::Event::WINDOW_CLOSE:
      case eq::Event::VIEW_RESIZE:
          _redraw = true;
          LBVERB << "Redraw: window change" << std::endl;
          break;

      default:
          break;
    }

    if( eq::Config::handleEvent( event ))
    {
        _redraw = true;
        LBVERB << "Redraw: requested by config event handler" << std::endl;
    }
    return _redraw;
}
#endif
bool MasterConfig::handleEvent( eq::EventCommand command )
{
    switch( command.getEventType( ))
    {
      case eq::Event::CHANNEL_POINTER_BUTTON_PRESS:
      {
          const eq::Event& event = command.get< eq::Event >();
          _currentViewID = event.context.view.identifier;
          return true;
      }

      case eq::Event::KEY_PRESS:
      case eq::Event::KEY_RELEASE:
          if( Config::handleEvent( command ))
          {
              _redraw = true;
              LBVERB << "Redraw: requested by eq::Config" << std::endl;
          }
          // no break;
      case eq::Event::CHANNEL_POINTER_BUTTON_RELEASE:
      case eq::Event::CHANNEL_POINTER_MOTION:
      case eq::Event::WINDOW_POINTER_WHEEL:
      case eq::Event::MAGELLAN_AXIS:
      {
          if( _currentViewID == uint128_t::ZERO )
              return false;

          View* view = static_cast<View*>( find<eq::View>( _currentViewID ));
          if( view->handleEvent( command ))
          {
              _redraw = true;
              LBVERB << "Redraw: requested by view event handler" << std::endl;
          }
          return true;
      }

      case eq::Event::STATISTIC:
      {
          Config::handleEvent( command );
          const eq::Event& event = command.get< eq::Event >();
          if( event.statistic.type != eq::Statistic::CONFIG_FINISH_FRAME )
              return false;

          ViewUpdateVisitor viewUpdate( _redraw );
          accept( viewUpdate );
          return _redraw;
      }

      case eq::Event::WINDOW_EXPOSE:
      case eq::Event::WINDOW_RESIZE:
      case eq::Event::WINDOW_CLOSE:
      case eq::Event::VIEW_RESIZE:
          _redraw = true;
          LBVERB << "Redraw: window change" << std::endl;
          break;

      default:
          break;
    }

    if( eq::Config::handleEvent( command ))
    {
        _redraw = true;
        LBVERB << "Redraw: requested by config event handler" << std::endl;
    }
    return _redraw;
}
}
}
