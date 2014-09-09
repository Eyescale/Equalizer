
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

#include "eventHandler.h"
#include "proxy.h"

#include "../messagePump.h"
#include "../channel.h"
#include "../config.h"
#include "../pipe.h"
#include "../window.h"
#include "../configEvent.h"

#include <lunchbox/perThread.h>
#include <dc/Stream.h>

namespace eq
{
namespace dc
{
namespace
{
typedef std::vector< EventHandler* > EventHandlers;
static lunchbox::PerThread< EventHandlers > _eventHandlers;
}

EventHandler::EventHandler( Proxy* proxy )
        : _proxy( proxy )
{
    LBASSERT( proxy );

    if( !_eventHandlers )
        _eventHandlers = new EventHandlers;
    _eventHandlers->push_back( this );

    eq::Pipe* pipe = proxy->getChannel()->getPipe();
    MessagePump* messagePump = pipe->isThreaded() ? pipe->getMessagePump() :
                                            pipe->getConfig()->getMessagePump();
    if( messagePump )
        messagePump->register_( proxy );
    else
        LBINFO << "Using dc::EventHandler without MessagePump, "
               << "external event dispatch assumed" << std::endl;
}

EventHandler::~EventHandler()
{
    eq::Pipe* pipe = _proxy->getChannel()->getPipe();
    MessagePump* messagePump =
        dynamic_cast<MessagePump*>( pipe->isThreaded() ?
                                    pipe->getMessagePump() :
                                    pipe->getConfig()->getMessagePump( ));
    if( messagePump )
        messagePump->deregister( _proxy );

    EventHandlers::iterator i = lunchbox::find( *_eventHandlers, this );
    LBASSERT( i != _eventHandlers->end( ));
    _eventHandlers->erase( i );
    if( _eventHandlers->empty( ))
    {
        delete _eventHandlers.get();
        _eventHandlers = 0;
    }
}

void EventHandler::processEvents( const Proxy* proxy )
{
    if( !_eventHandlers )
        return;

    for( EventHandlers::const_iterator i = _eventHandlers->begin();
         i != _eventHandlers->end(); ++i )
    {
        (*i)->_processEvents( proxy );
    }
}

void EventHandler::_processEvents( const Proxy* proxy )
{
    LB_TS_THREAD( _thread );
    if( !_proxy || (proxy && _proxy != proxy ))
        return;

    const PixelViewport& pvp = _proxy->getChannel()->getPixelViewport();
    eq::Channel* channel = _proxy->getChannel();
    eq::Window* window = channel->getWindow();

    while( _proxy->hasNewEvent( ))
    {
        const ::dc::Event& dcEvent = _proxy->getEvent();

        if( dcEvent.type == ::dc::Event::EVT_CLOSE )
        {
            _proxy->stopRunning();
            ConfigEvent configEvent;
            configEvent.data.type = Event::EXIT;
            window->getConfig()->sendEvent( configEvent );
            break;
        }

        Event event;
        event.originator = channel->getID();
        event.serial = channel->getSerial();
        event.type = Event::UNKNOWN;

        const float x = dcEvent.mouseX * pvp.w;
        const float y = dcEvent.mouseY * pvp.h;

        switch( dcEvent.type )
        {
        case ::dc::Event::EVT_KEY_PRESS:
        case ::dc::Event::EVT_KEY_RELEASE:
            event.type = dcEvent.type == ::dc::Event::EVT_KEY_PRESS ?
                                          Event::KEY_PRESS : Event::KEY_RELEASE;
            event.keyPress.key = dcEvent.key;
            break;
        case ::dc::Event::EVT_PRESS:
        case ::dc::Event::EVT_RELEASE:
            event.type = dcEvent.type == ::dc::Event::EVT_PRESS ?
                                          Event::CHANNEL_POINTER_BUTTON_PRESS :
                                          Event::CHANNEL_POINTER_BUTTON_RELEASE;
            event.pointerButtonPress.x = x;
            event.pointerButtonPress.y = y;

            if( dcEvent.mouseLeft )
                event.pointerButtonPress.buttons |= PTR_BUTTON1;
            if( dcEvent.mouseMiddle )
                event.pointerButtonPress.buttons |= PTR_BUTTON2;
            if( dcEvent.mouseRight )
                event.pointerButtonPress.buttons |= PTR_BUTTON3;
            event.pointerButtonPress.button = event.pointerButtonPress.buttons;
            _computePointerDelta( event );
            break;
        case ::dc::Event::EVT_DOUBLECLICK:
            break;
        case ::dc::Event::EVT_MOVE:
            event.type = Event::CHANNEL_POINTER_MOTION;
            event.pointerMotion.x = x;
            event.pointerMotion.y = y;

            if( dcEvent.mouseLeft )
                event.pointerButtonPress.buttons |= PTR_BUTTON1;
            if( dcEvent.mouseMiddle )
                event.pointerButtonPress.buttons |= PTR_BUTTON2;
            if( dcEvent.mouseRight )
                event.pointerButtonPress.buttons |= PTR_BUTTON3;

            event.pointerMotion.button = event.pointerMotion.buttons;
            _computePointerDelta( event );
            break;
        case ::dc::Event::EVT_WHEEL:
            event.type = Event::CHANNEL_POINTER_WHEEL;
            event.pointerWheel.x = x;
            event.pointerWheel.y = pvp.h - y;
            event.pointerWheel.buttons = PTR_BUTTON_NONE;
            if( dcEvent.dx != 0 )
                event.pointerWheel.xAxis = dcEvent.dx > 0 ? 1 : -1;
            if( dcEvent.dy != 0 )
                event.pointerWheel.yAxis = dcEvent.dy > 0 ? 1 : -1;
            _computePointerDelta( event );
            break;
        case ::dc::Event::EVT_NONE:
        default:
            break;
        }

        if( event.type != Event::UNKNOWN )
        {
            // TODO: compute and use window x,y coordinates
            if( !window->getRenderContext( x, y, event.context ))
                LBVERB << "No rendering context for pointer event at " << x
                       << ", " << y << std::endl;
            channel->processEvent( event );
        }
    }
}

}
}
