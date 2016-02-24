
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
#include <deflect/Stream.h>

namespace eq
{
namespace deflect
{
namespace
{
typedef std::vector< EventHandler* > EventHandlers;
static lunchbox::PerThread< EventHandlers > _eventHandlers;

// Values come from QtCore/qnamespace.h, but don't want to depend on Qt just for that
uint32_t _getKey( const int key )
{
    switch( key )
    {
    case 0x01000000: return KC_ESCAPE;
    case 0x01000001: return KC_TAB;
    case 0x01000003: return KC_BACKSPACE;
    case 0x01000004: return KC_RETURN;
    case 0x01000010: return KC_HOME;
    case 0x01000011: return KC_END;
    case 0x01000012: return KC_LEFT;
    case 0x01000013: return KC_UP;
    case 0x01000014: return KC_RIGHT;
    case 0x01000015: return KC_DOWN;
    case 0x01000016: return KC_PAGE_UP;
    case 0x01000017: return KC_PAGE_DOWN;
    case 0x01000020: return KC_SHIFT_L;
    case 0x01000021: return KC_CONTROL_L;
    case 0x01000023: return KC_ALT_L;
    case 0x01000030: return KC_F1;
    case 0x01000031: return KC_F2;
    case 0x01000032: return KC_F3;
    case 0x01000033: return KC_F4;
    case 0x01000034: return KC_F5;
    case 0x01000035: return KC_F6;
    case 0x01000036: return KC_F7;
    case 0x01000037: return KC_F8;
    case 0x01000038: return KC_F9;
    case 0x01000039: return KC_F10;
    case 0x0100003a: return KC_F11;
    case 0x0100003b: return KC_F12;
    case 0x0100003c: return KC_F13;
    case 0x0100003d: return KC_F14;
    case 0x0100003e: return KC_F15;
    case 0x0100003f: return KC_F16;
    case 0x01000040: return KC_F17;
    case 0x01000041: return KC_F18;
    case 0x01000042: return KC_F19;
    case 0x01000043: return KC_F20;
    case 0x01000044: return KC_F21;
    case 0x01000045: return KC_F22;
    case 0x01000046: return KC_F23;
    case 0x01000047: return KC_F24;
    case 0x01001103: return KC_ALT_R;
    case 0x01ffffff: return KC_VOID;
    default:         return key;
    }
}
}

EventHandler::EventHandler( Proxy* proxy )
    : _proxy( proxy )
{
    LBASSERT( proxy );

    if( !_eventHandlers )
        _eventHandlers = new EventHandlers;
    _eventHandlers->push_back( this );

    Pipe* pipe = proxy->getChannel()->getPipe();
    MessagePump* messagePump = pipe->isThreaded() ? pipe->getMessagePump() :
                                            pipe->getConfig()->getMessagePump();
    if( messagePump )
        messagePump->register_( proxy );
    else
        LBINFO << "Using deflect::EventHandler without MessagePump, "
               << "external event dispatch assumed" << std::endl;
}

EventHandler::~EventHandler()
{
    Pipe* pipe = _proxy->getChannel()->getPipe();
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
    Channel* channel = _proxy->getChannel();
    Window* window = channel->getWindow();

    while( _proxy->hasNewEvent( ))
    {
        ::deflect::Event deflectEvent = _proxy->getEvent();

        if( deflectEvent.type == ::deflect::Event::EVT_CLOSE )
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

        const float x = deflectEvent.mouseX * pvp.w;
        const float y = deflectEvent.mouseY * pvp.h;

        if( _proxy-> getNavigationMode() == Proxy::MODE_PAN )
            std::swap( deflectEvent.mouseLeft, deflectEvent.mouseRight );

        switch( deflectEvent.type )
        {
        case ::deflect::Event::EVT_KEY_PRESS:
        case ::deflect::Event::EVT_KEY_RELEASE:
            event.type = deflectEvent.type == ::deflect::Event::EVT_KEY_PRESS ?
                                          Event::KEY_PRESS : Event::KEY_RELEASE;
            event.keyPress.key = _getKey(  deflectEvent.key );
            break;
        case ::deflect::Event::EVT_PRESS:
        case ::deflect::Event::EVT_RELEASE:
            event.type = deflectEvent.type == ::deflect::Event::EVT_PRESS ?
                                          Event::CHANNEL_POINTER_BUTTON_PRESS :
                                          Event::CHANNEL_POINTER_BUTTON_RELEASE;
            event.pointerButtonPress.x = x;
            event.pointerButtonPress.y = y;

            if( deflectEvent.mouseLeft )
                event.pointerButtonPress.buttons |= PTR_BUTTON1;
            if( deflectEvent.mouseMiddle )
                event.pointerButtonPress.buttons |= PTR_BUTTON2;
            if( deflectEvent.mouseRight )
                event.pointerButtonPress.buttons |= PTR_BUTTON3;
            event.pointerButtonPress.button = event.pointerButtonPress.buttons;
            _computePointerDelta( event );
            break;
        case ::deflect::Event::EVT_DOUBLECLICK:
            break;
        case ::deflect::Event::EVT_MOVE:
            event.type = Event::CHANNEL_POINTER_MOTION;
            event.pointerMotion.x = x;
            event.pointerMotion.y = y;

            if( deflectEvent.mouseLeft )
                event.pointerButtonPress.buttons |= PTR_BUTTON1;
            if( deflectEvent.mouseMiddle )
                event.pointerButtonPress.buttons |= PTR_BUTTON2;
            if( deflectEvent.mouseRight )
                event.pointerButtonPress.buttons |= PTR_BUTTON3;

            event.pointerMotion.button = event.pointerMotion.buttons;
            event.pointerMotion.dx = deflectEvent.dx * pvp.w;
            event.pointerMotion.dy = deflectEvent.dy * pvp.h;
            break;
        case ::deflect::Event::EVT_WHEEL:
            event.type = Event::CHANNEL_POINTER_WHEEL;
            event.pointerWheel.x = x;
            event.pointerWheel.y = pvp.h - y;
            event.pointerWheel.buttons = PTR_BUTTON_NONE;
            event.pointerWheel.xAxis = deflectEvent.dx / 40.f;
            event.pointerWheel.yAxis = deflectEvent.dy / 40.f;
            event.pointerMotion.dx = -deflectEvent.dx;
            event.pointerMotion.dy = -deflectEvent.dy;
            break;
        case ::deflect::Event::EVT_TAP_AND_HOLD:
        {
            const Proxy::NavigationMode mode =
                    _proxy->getNavigationMode() == Proxy::MODE_PAN
                        ? Proxy::MODE_ROTATE : Proxy::MODE_PAN;
            _proxy->setNavigationMode( mode );
            Event windowEvent;
            windowEvent.originator = window->getID();
            windowEvent.serial = window->getSerial();
            windowEvent.type = Event::WINDOW_EXPOSE;
            window->processEvent( windowEvent );
        } break;
        case ::deflect::Event::EVT_NONE:
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
