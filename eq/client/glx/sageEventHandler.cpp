
/* Copyright (c) 2013, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "sageEventHandler.h"

#include "messagePump.h"
#include "../channel.h"
#include "../config.h"
#include "../pipe.h"
#include "../sageProxy.h"
#include "../window.h"

#include <libsage.h>

#include <lunchbox/perThread.h>

namespace eq
{
namespace glx
{
namespace
{
typedef std::vector< SageEventHandler* > EventHandlers;
static lunchbox::PerThread< EventHandlers > _eventHandlers;
}

SageEventHandler::SageEventHandler( SageProxy* sage )
        : _sage( sage )
{
    LBASSERT( sage );

    if( !_eventHandlers )
        _eventHandlers = new EventHandlers;
    _eventHandlers->push_back( this );

    eq::Pipe* pipe = sage->getChannel()->getPipe();
    MessagePump* messagePump =
        dynamic_cast< MessagePump* >( pipe->isThreaded() ?
                                      pipe->getMessagePump() :
                                      pipe->getConfig()->getMessagePump( ));
    if( messagePump )
        messagePump->register_( sage );
    else
        LBINFO << "Using glx::SageEventHandler without glx::MessagePump, "
               << "external event dispatch assumed" << std::endl;
}

SageEventHandler::~SageEventHandler()
{
    eq::Pipe* pipe = _sage->getChannel()->getPipe();
    MessagePump* messagePump = dynamic_cast<MessagePump*>( pipe->isThreaded() ?
                 pipe->getMessagePump() : pipe->getConfig()->getMessagePump( ));
    if( messagePump )
        messagePump->deregister( _sage );

    EventHandlers::iterator i = stde::find( *_eventHandlers, this );
    LBASSERT( i != _eventHandlers->end( ));
    _eventHandlers->erase( i );
    if( _eventHandlers->empty( ))
    {
        delete _eventHandlers.get();
        _eventHandlers = 0;
    }
}

void SageEventHandler::processEvents( const SageProxy* sage )
{
    if( !_eventHandlers )
        return;

    for( EventHandlers::const_iterator i = _eventHandlers->begin();
         i != _eventHandlers->end(); ++i )
    {
        (*i)->_processEvents( sage );
    }
}

void SageEventHandler::_processEvents( const SageProxy* sage )
{
    LB_TS_THREAD( _thread );
    if( !_sage || (sage && _sage != sage ))
        return;

    const PixelViewport& pvp = _sage->getChannel()->getPixelViewport();
    eq::Window* window = _sage->getChannel()->getWindow();

    sageMessage msg;
    while( _sage->getSail()->checkMsg( msg, false ) > 0 )
    {
        if( msg.getCode() == APP_QUIT )
        {
            _sage->stopRunning();
            continue;
        }

        Event event;
        event.originator = window->getID();
        event.serial = window->getSerial();
        event.type = Event::UNKNOWN;

        int deviceID;
        float x, y;
        std::istringstream data( static_cast<char*>( msg.getData( )));
        data >> deviceID >> x >> y;
        x *= pvp.w;
        y *= pvp.h;

        switch( msg.getCode( ))
        {
        case EVT_APP_SHARE:
            processMessages( _sage->getSail(), 0, 0 );
            break;
        case EVT_CLICK:
        {
            int buttonID, isDown, clickEvent;
            data >> buttonID >> isDown >> clickEvent;

            event.type = isDown ? Event::WINDOW_POINTER_BUTTON_PRESS :
                                  Event::WINDOW_POINTER_BUTTON_RELEASE;
            event.pointerButtonPress.x = x;
            event.pointerButtonPress.y = pvp.h - y;
            event.pointerButtonPress.buttons = buttonID == 1 ? PTR_BUTTON1 :
                                                               PTR_BUTTON3;
            event.pointerButtonPress.button = event.pointerButtonPress.buttons;
            break;
        }
        case EVT_DOUBLE_CLICK:
            LBWARN << "Implement EVT_DOUBLE_CLICK" << std::endl;
            break;
        case EVT_PAN:
        case EVT_ROTATE:
        {
            float dx, dy;
            data >> dx >> dy;
            dx *= pvp.w;
            dy *= pvp.h;
            event.pointerMotion.dx = dx;
            event.pointerMotion.dy = -dy;
        }
        case EVT_MOVE:
        {
            event.type = Event::WINDOW_POINTER_MOTION;
            event.pointerMotion.x = x;
            event.pointerMotion.y = pvp.h - y;

            switch( msg.getCode( ))
            {
            case EVT_MOVE:
                event.pointerMotion.buttons = PTR_BUTTON_NONE;
                break;
            case EVT_PAN:
                event.pointerMotion.buttons = PTR_BUTTON1;
                break;
            case EVT_ROTATE:
                event.pointerMotion.buttons = PTR_BUTTON3;
                break;
            }

            event.pointerMotion.button = event.pointerMotion.buttons;
            break;
        }
        case EVT_ZOOM:
        {
            float dx, dy;
            data >> dx >> dy;

            event.type = Event::WINDOW_POINTER_WHEEL;
            event.pointerWheel.x = x;
            event.pointerWheel.y = pvp.h - y;
            event.pointerWheel.buttons = PTR_BUTTON_NONE;
            if( dx != 0 )
                event.pointerWheel.xAxis = dx < 0 ? 1 : -1;
            if( dy != 0 )
                event.pointerWheel.yAxis = dy < 0 ? 1 : -1;
            break;
        }
        case EVT_ARROW:
            LBWARN << "Implement EVT_ARROW" << std::endl;
            break;
        case EVT_KEY:
            LBWARN << "Implement EVT_KEY" << std::endl;
            break;
        default:
            LBWARN << "Unhandled SAGE message " << msg.getCode() << std::endl;
        }

        if( event.type != Event::UNKNOWN )
        {
            if( !window->getRenderContext( x, y, event.context ))
                LBVERB << "No rendering context for pointer event at " << x
                       << ", " << y << std::endl;
            window->processEvent( event );
        }
    }
}

}
}
