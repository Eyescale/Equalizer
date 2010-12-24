
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "eventConnection.h"
#include "pipeConnection.h"

namespace co
{
EventConnection::EventConnection()
#ifdef _WIN32
        : _event( 0 )
#else
        : _set( false )
#endif
{
}

EventConnection::~EventConnection()
{
    close();
}

bool EventConnection::connect()
{
    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

#ifdef _WIN32
    _event = CreateEvent( 0, TRUE, FALSE, 0 );
#else
    _connection = new PipeConnection;
    EQCHECK( _connection->connect( ));
    _set = false;
#endif

    _state = STATE_CONNECTED;
    _fireStateChanged();
    return true;    
}

void EventConnection::close()
{
#ifdef _WIN32
    if( _event )
        CloseHandle( _event );
    _event = 0;
#else
    if( _connection.isValid( ))
        _connection->close();
    _connection = 0;
    _set = false;
#endif

    _state = STATE_CLOSED;
    _fireStateChanged();
}

void EventConnection::set()
{
#ifdef _WIN32
    SetEvent( _event );
#else
    co::base::ScopedMutex<> mutex( _lock );
    if( _set )
        return;
    
    const char c = 42;
    _connection->send( &c, 1, true );
    _set = true;
#endif
}
void EventConnection::reset()
{
#ifdef _WIN32
    ResetEvent( _event );
#else
    co::base::ScopedMutex<> mutex( _lock );
    if( !_set )
        return;
    
    char c = 42;
    _connection->recvNB( &c, 1 );
    _connection->recvSync( 0, 0 );
    _set = false;
#endif
}

Connection::Notifier EventConnection::getNotifier() const
{
#ifdef _WIN32
    return _event;
#else
    return _connection->getNotifier();
#endif
}

}
