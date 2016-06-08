
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "messagePump.h"

#include "eventHandler.h"
#include "X11Connection.h"

#ifdef EQUALIZER_USE_DEFLECT
#  include "../deflect/connection.h"
#  include "../deflect/eventHandler.h"
#endif

#include <lunchbox/debug.h>
#include <lunchbox/log.h>

namespace eq
{
namespace glx
{
MessagePump::MessagePump()
{
}

MessagePump::~MessagePump()
{
}

void MessagePump::postWakeup()
{
    _connections.interrupt();
}

void MessagePump::dispatchOne( const uint32_t timeout )
{
    const co::ConnectionSet::Event event = _connections.select( timeout );
    switch( event )
    {
        case co::ConnectionSet::EVENT_DISCONNECT:
        {
            co::ConnectionPtr connection = _connections.getConnection();
            _connections.removeConnection( connection );
            LBERROR << "Display connection shut down" << std::endl;
            break;
        }

        case co::ConnectionSet::EVENT_DATA:
        {
#ifdef EQUALIZER_USE_DEFLECT
            co::ConnectionPtr connection = _connections.getConnection();
            const deflect::Connection* dcConnection =
                dynamic_cast< const deflect::Connection* >( connection.get( ));
            if( dcConnection )
               deflect::EventHandler::processEvents( dcConnection->getProxy( ));
            else
#endif
            EventHandler::dispatch();
            break;
        }

        case co::ConnectionSet::EVENT_INTERRUPT:
            break;

        case co::ConnectionSet::EVENT_CONNECT:
        case co::ConnectionSet::EVENT_ERROR:
        default:
            LBWARN << "Error during select" << std::endl;
            break;

        case co::ConnectionSet::EVENT_TIMEOUT:
            break;
    }
}

void MessagePump::dispatchAll()
{
    EventHandler::dispatch();
#ifdef EQUALIZER_USE_DEFLECT
    deflect::EventHandler::processEvents();
#endif
}

void MessagePump::register_( Display* display )
{
    if( ++_referenced[ display ] == 1 )
        _connections.addConnection( new X11Connection( display ));
}

void MessagePump::deregister( Display* display )
{
    if( --_referenced[ display ] == 0 )
    {
        const co::Connections& connections = _connections.getConnections();
        for( co::Connections::const_iterator i = connections.begin();
             i != connections.end(); ++i )
        {
            co::ConnectionPtr connection = *i;
            const X11Connection* x11Connection =
                dynamic_cast< const X11Connection* >( connection.get( ));
            if( x11Connection && x11Connection->getDisplay() == display )
            {
                _connections.removeConnection( connection );
                break;
            }
        }
        _referenced.erase( _referenced.find( display ));
    }
}

void MessagePump::register_( deflect::Proxy* proxy LB_UNUSED )
{
#ifdef EQUALIZER_USE_DEFLECT
    if( ++_referenced[ proxy ] == 1 )
        _connections.addConnection( new deflect::Connection( proxy ));
#endif
}

void MessagePump::deregister( deflect::Proxy* proxy LB_UNUSED  )
{
#ifdef EQUALIZER_USE_DEFLECT
    if( --_referenced[ proxy ] == 0 )
    {
        const co::Connections& connections = _connections.getConnections();
        for( co::Connections::const_iterator i = connections.begin();
             i != connections.end(); ++i )
        {
            co::ConnectionPtr connection = *i;
            const deflect::Connection* dcConnection =
                dynamic_cast< const deflect::Connection* >( connection.get( ));
            if( dcConnection && dcConnection->getProxy() == proxy )
            {
                _connections.removeConnection( connection );
                break;
            }
        }
        _referenced.erase( _referenced.find( proxy ));
    }
#endif
}

}
}
