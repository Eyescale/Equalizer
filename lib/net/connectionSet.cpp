
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "connectionSet.h"

#include "connection.h"
#include "node.h"
#include "pipeConnection.h"

#include <eq/base/base.h>
#include <eq/base/stdExt.h>
#include <eq/base/thread.h>

#include <algorithm>
#include <errno.h>

#ifdef WIN32
#  define EQ_SOCKET_ERROR getErrorString( _error ) << '(' << _error << ')'
#else
#  define EQ_SOCKET_ERROR strerror( _error )
#endif

using namespace eq::base;

#define SELF_INTERRUPT 42

#ifdef WIN32
#  define SELECT_TIMEOUT WAIT_TIMEOUT
#  define SELECT_ERROR   WAIT_FAILED
#else
#  define SELECT_TIMEOUT  0
#  define SELECT_ERROR   -1
#endif

namespace eq
{
namespace net
{

ConnectionSet::ConnectionSet()
        : _selfCommand( 0 )
        , _error( 0 )
        , _dirty( true )
{
    // Whenever another threads modifies the connection list while the
    // connection set is waiting in a select, the select is interrupted by
    // sending a character through this connection. select() will recognize
    // this and restart with the modified fd set.
    _selfConnection = new PipeConnection;
    if( !_selfConnection->connect( ))
    {
        EQERROR << "Could not create connection" << std::endl;
        return;
    }
    _selfConnection->recvNB( &_selfCommand, sizeof( _selfCommand ));
}

ConnectionSet::~ConnectionSet()
{
    _connection = 0;

    _selfConnection->close();
    _selfConnection = 0;
}


void ConnectionSet::_dirtyFDSet()
{
    if( _dirty )
        return;

    EQINFO << "FD set modified, restarting select" << std::endl;
    _dirty = true;

    interrupt();
}

void ConnectionSet::interrupt()
{
    const char c = SELF_INTERRUPT;
    _selfConnection->send( &c, 1, true );
}

void ConnectionSet::addConnection( ConnectionPtr connection )
{
    EQASSERT( connection->isConnected() || connection->isListening( ));
    {
        base::ScopedMutex mutex( _mutex );
        EQASSERTINFO( _connections.size() < 63,
            "Connection set can't handle more than 63 connections" );
        _connections.push_back( connection );
        connection->addListener( this );
    }
    _dirtyFDSet();
}

bool ConnectionSet::removeConnection( ConnectionPtr connection )
{
    {
        ScopedMutex mutex( _mutex );
        ConnectionVector::iterator i = find( _connections.begin(),
                                             _connections.end(), connection );
        if( i == _connections.end( ))
            return false;

        connection->removeListener( this );
        _connections.erase( i );
    }

    if( _connection == connection )
        _connection = 0;

    _dirtyFDSet();
    return true;
}

void ConnectionSet::clear()
{
    _connection = 0;
    for( ConnectionVector::iterator i = _connections.begin(); 
         i != _connections.end(); ++i )

        (*i)->removeListener( this );

    _connections.clear();
    _dirtyFDSet();
    _fdSet.clear();
    _fdSetConnections.clear();
}
        
ConnectionSet::Event ConnectionSet::select( const int timeout )
{
    while( true )
    {
        _connection = 0;
        _error      = 0;

        if( !_setupFDSet( ))
            return EVENT_INVALID_HANDLE;

        // poll for a result
#ifdef WIN32
        const DWORD waitTime = timeout > 0 ? timeout : INFINITE;
        const DWORD ret = WaitForMultipleObjectsEx( _fdSet.size, _fdSet.data,
                                                    FALSE, waitTime, TRUE );
#else
        const int ret = poll( _fdSet.data, _fdSet.size, timeout );
#endif
        switch( ret )
        {
            case SELECT_TIMEOUT:
                return EVENT_TIMEOUT;

            case SELECT_ERROR:
#ifdef WIN32
                _error = GetLastError();
                if( _error == WSA_INVALID_HANDLE )
                {
                    _dirty = true;
                    break;
                }
#else
                if( errno == EINTR ) // Interrupted system call (gdb) - ignore
                    break;

                _error = errno;
#endif

                EQERROR << "Error during select: " << EQ_SOCKET_ERROR << std::endl;
                return EVENT_SELECT_ERROR;

            default: // SUCCESS
                {
                    Event event = _getSelectResult( ret );

                    if( event == EVENT_NONE )
                         break;

                    if( _connection == _selfConnection )
                    {
                        EQASSERT( event == EVENT_DATA );
                        event = _handleSelfCommand();
                        if( event == EVENT_NONE )
                            break;
                        return event;
                    }
                    
                    if( event == EVENT_DATA && _connection->isListening( ))
                        event = EVENT_CONNECT;

                    EQVERB << "selected connection " << _connection << " of "
                           << _fdSetConnections.size << ", event " << event
                           << std::endl;
                    return event;
                }
        }
    }
}
     
ConnectionSet::Event ConnectionSet::_getSelectResult( const uint32_t index )
{
#ifdef WIN32
    const uint32_t i = index - WAIT_OBJECT_0;
    _connection = _fdSetConnections[i];

    EQASSERT( _fdSet[i] == _connection->getNotifier( ));
    return EVENT_DATA;
#else
    for( size_t i = 0; i < _fdSet.size; ++i )
    {
        const pollfd& pollFD = _fdSet[i];
        if( pollFD.revents == 0 )
            continue;

        const int pollEvents = pollFD.revents;
        EQASSERT( pollFD.fd > 0 );

        _connection = _fdSetConnections[i];
        EQASSERT( _connection.isValid( ));

        EQVERB << "Got event on connection @" << (void*)_connection.get()
               << std::endl;

        if( pollEvents & POLLERR )
        {
            EQINFO << "Error during poll()" << std::endl;
            return EVENT_ERROR;
        }

        if( pollEvents & POLLHUP ) // disconnect happened
            return EVENT_DISCONNECT;

        if( pollEvents & POLLNVAL ) // disconnected connection
            return EVENT_DISCONNECT;

        // Note: Intuitively I would handle the read before HUP to
        // read remaining data of the connection, but at least on
        // OS X both events happen simultaneously and no more data
        // can be read.
        if( pollEvents & POLLIN || pollEvents & POLLPRI )
            return EVENT_DATA;

        EQERROR << "Unhandled poll event(s): " << pollEvents << std::endl;
        ::abort();
    }
    return EVENT_NONE;
#endif
}

ConnectionSet::Event ConnectionSet::_handleSelfCommand()
{
    EQASSERT( _connection == _selfConnection );
    _connection = 0;
    
    _selfConnection->recvSync( 0, 0 );
    const uint8_t command( _selfCommand );
    _selfConnection->recvNB( &_selfCommand, sizeof( _selfCommand ));

    switch( command ) 
    {
        case SELF_INTERRUPT:
            return EVENT_INTERRUPT;

        default:
            EQUNIMPLEMENTED;
            return EVENT_NONE;
    }
}

bool ConnectionSet::_setupFDSet()
{
    if( !_dirty )
    {
#ifndef WIN32
        // TODO: verify that poll() really modifies _fdSet, and remove the copy
        // if it doesn't. The man page seems to hint that poll changes fds.
        _fdSet = _fdSetCopy;
#endif
        return true;
    }

    _dirty = false;
    _fdSet.size = 0;
    _fdSetConnections.size = 0;

#ifdef WIN32
    // add self connection
    HANDLE readHandle = _selfConnection->getNotifier();
    EQASSERT( readHandle );

    _fdSet.append( readHandle );
    _fdSetConnections.append( _selfConnection.get( ));

    // add regular connections
    _mutex.set();
    for( ConnectionVector::const_iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        readHandle = connection->getNotifier();

        if( !readHandle )
        {
            EQINFO << "Cannot select connection " << connection
                 << ", connection does not provide a read handle" << std::endl;
            _connection = connection;
            _mutex.unset();
            return false;
        }
        
        _fdSet.append( readHandle );
        _fdSetConnections.append( connection.get( ));
    }
    _mutex.unset();
#else
    pollfd fd;
    fd.events = POLLIN; // | POLLPRI;

    // add self 'connection'
    fd.fd      = _selfConnection->getNotifier();
    EQASSERT( fd.fd > 0 );
    fd.revents = 0;

    _fdSet.append( fd );
    _fdSetConnections.append( _selfConnection.get( ));

    // add regular connections
    _mutex.set();
    for( ConnectionVector::const_iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        fd.fd = connection->getNotifier();

        if( fd.fd <= 0 )
        {
            EQINFO << "Cannot select connection " << connection
                   << ", connection " << typeid( *connection.get( )).name() 
                   << " does not use a file descriptor" << std::endl;
            _connection = connection;
            _mutex.unset();
            return false;
        }

        EQVERB << "Listening on " << typeid( *connection.get( )).name() 
               << " @" << (void*)connection.get() << std::endl;
        fd.revents = 0;

        _fdSet.append( fd );
        _fdSetConnections.append( connection.get( ));
    }
    _mutex.unset();
    _fdSetCopy = _fdSet;
#endif

    return true;
}   

EQ_EXPORT std::ostream& operator << ( std::ostream& os,
                                      const ConnectionSet* set)
{
    const ConnectionVector& connections = set->getConnections();

    os << "connection set " << (void*)set << ", " << connections.size()
       << " connections";
    
    for( ConnectionVector::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        os << std::endl << "    " << (*i).get();
    }
    
    return os;
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const ConnectionSet::Event event )
{
    if( event >= ConnectionSet::EVENT_ALL )
        os << "unknown (" << static_cast<unsigned>( event ) << ')';
    else 
        os << ( event == ConnectionSet::EVENT_NONE ? "none" :       
                event == ConnectionSet::EVENT_CONNECT ? "connect" :        
                event == ConnectionSet::EVENT_DISCONNECT ? "disconnect" :     
                event == ConnectionSet::EVENT_DATA ? "data" :           
                event == ConnectionSet::EVENT_TIMEOUT ? "timeout" :        
                event == ConnectionSet::EVENT_INTERRUPT ? "interrupted" :      
                event == ConnectionSet::EVENT_ERROR ? "error" :          
                event == ConnectionSet::EVENT_SELECT_ERROR ? "select error" :   
                event == ConnectionSet::EVENT_INVALID_HANDLE ? "invalid handle":
                "ERROR" );

    return os;
}

}
}
