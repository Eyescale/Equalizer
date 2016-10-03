
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include <eq/eventICommand.h>
#include <eq/fabric/configVisitor.h>
#include <eq/fabric/pointerEvent.h>
#include <eq/fabric/sizeEvent.h>
#include <eq/fabric/statistic.h>

namespace seq
{
namespace detail
{
MasterConfig::MasterConfig( eq::ServerPtr parent )
        : Config( parent )
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
        LBWARN << "Error during initialization" << std::endl;
        exit();
        return false;
    }

    _redraw = true;
    return true;
}

bool MasterConfig::exit()
{
    const bool retVal = eq::Config::exit();

    if( _objects )
        deregisterObject( _objects );
    _objects->clear();
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
                const eq::EventICommand& event = getNextEvent();
                if( !Config::handleEvent( event ))
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
    explicit ViewUpdateVisitor( bool &redraw ) : _redraw( redraw ) {}
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

template< class E >
bool MasterConfig::_handleEvent( eq::EventType type, E& event )
{
    if( Config::handleEvent( type, event ))
        _redraw = true;

    if( _currentViewID == 0 )
        return _redraw;

    View* view = static_cast< View* >( find< eq::View >( _currentViewID ));
    if( view && view->handleEvent( type, event ))
        _redraw = true;

    return _redraw;
}



bool MasterConfig::handleEvent( EventICommand command )
{
    switch( command.getEventType( ))
    {
    case EVENT_REDRAW:
        _redraw = true;
        LBVERB << "Redraw request" << std::endl;
        return true;

    default:
        return Config::handleEvent( command );
    }
}

bool MasterConfig::handleEvent( const eq::EventType type,
                                const SizeEvent& event )
{
    _redraw = true;
    return _handleEvent( type, event );
}

bool MasterConfig::handleEvent( const eq::EventType type,
                                const PointerEvent& event )
{
    switch( type )
    {
    case EVENT_CHANNEL_POINTER_BUTTON_PRESS:
        _currentViewID = event.context.view.identifier;
        break;

    default:
        break;
    }

    return _handleEvent( type, event );
}

bool MasterConfig::handleEvent( const eq::EventType type,
                                const KeyEvent& event )
{
    return _handleEvent( type, event );
}

bool MasterConfig::handleEvent( const eq::EventType type,
                                const AxisEvent& event )
{
    return _handleEvent( type, event );
}

bool MasterConfig::handleEvent( const eq::EventType type,
                                const ButtonEvent& event )
{
    return _handleEvent( type, event );
}

bool MasterConfig::handleEvent( const eq::EventType type, const Event& event )
{
    if( Config::handleEvent( type, event ))
        _redraw = true;

    switch( type )
    {
    case EVENT_WINDOW_EXPOSE:
    case EVENT_WINDOW_CLOSE:
        _redraw = true;
        return true;

    default:
        return _redraw;
    }
}

void MasterConfig::addStatistic( const Statistic& stat )
{
    Config::addStatistic( stat );
    if( stat.type == eq::Statistic::CONFIG_FINISH_FRAME )
    {
        ViewUpdateVisitor viewUpdate( _redraw );
        accept( viewUpdate );
    }
}

}
}
