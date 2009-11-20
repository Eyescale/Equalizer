
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

#define SELF_INTERRUPT 42

#ifdef WIN32
#  define SELECT_TIMEOUT WAIT_TIMEOUT
#  define SELECT_ERROR   WAIT_FAILED
#  define BIG_CLUSTER_SUPPORT
#  define MAX_CONNECTIONS (MAXIMUM_WAIT_OBJECTS - 1)
#else
#  define SELECT_TIMEOUT  0
#  define SELECT_ERROR   -1
#  define MAX_CONNECTIONS EQ_100KB  // Arbitrary
#endif

#ifdef BIG_CLUSTER_SUPPORT
#  ifndef WIN32
#    error "BIG_CLUSTER_SUPPORT only needed and implemented on Windows"
#  endif
#endif

namespace eq
{
namespace net
{

ConnectionSet::ConnectionSet()
#ifdef WIN32
        : _thread( 0 ),
#else
        :
#endif
          _selfConnection( new PipeConnection )
        , _selfCommand( 0 )
        , _error( 0 )
        , _dirty( true )
{
    // Whenever another threads modifies the connection list while the
    // connection set is waiting in a select, the select is interrupted by
    // sending a character through this connection. select() will recognize
    // this and restart with the modified fd set.
    // OPT: On Win32, we could just use an event handle
    if( !_selfConnection->connect( ))
    {
        EQERROR << "Could not create self connection" << std::endl;
        return;
    }
    _selfConnection->recvNB( &_selfCommand, sizeof( _selfCommand ));
}

ConnectionSet::~ConnectionSet()
{
    clear();

    _connection = 0;

    _selfConnection->close();
    _selfConnection = 0;
}


void ConnectionSet::_dirtyFDSet()
{
    if( _dirty )
        return;

    EQVERB << "FD set modified, restarting select" << std::endl;
    _dirty = true;
    interrupt();
}

void ConnectionSet::interrupt()
{
    if( !_selfConnection->hasData( ))
    {
        const char c = SELF_INTERRUPT;
        _selfConnection->send( &c, 1, true );
    }
}

void ConnectionSet::addConnection( ConnectionPtr connection )
{
    EQASSERT( connection->isConnected() || connection->isListening( ));

    { 
        base::ScopedMutex mutex( _mutex );
        _allConnections.push_back( connection );

#ifdef BIG_CLUSTER_SUPPORT
        EQASSERT( _allConnections.size() < MAX_CONNECTIONS * MAX_CONNECTIONS );
        if( _connections.size() < MAX_CONNECTIONS - _threads.size( ))
        {   // can handle it ourself
            _connections.push_back( connection );
            connection->addListener( this );
        }
        else
        {
            // add to existing thread
            for( ThreadVector::const_iterator i = _threads.begin();
                 i != _threads.end(); ++i )
            {
                Thread* thread = *i;
                if( thread->set->getSize() > MAX_CONNECTIONS )
                    continue;

                thread->set->addConnection( connection );
                return;
            }

            // add to new thread
            Thread* thread = new Thread( this );
            thread->set->addConnection( connection );
            thread->set->addConnection( _connections.back( ));
            _connections.pop_back();

            _threads.push_back( thread );
            thread->start();
        }
#else
        _connections.push_back( connection );
        connection->addListener( this );

        EQASSERT( _connections.size() < MAX_CONNECTIONS );
#endif
    }

    _dirtyFDSet();
}

bool ConnectionSet::removeConnection( ConnectionPtr connection )
{
    {
        base::ScopedMutex mutex( _mutex );
        ConnectionVector::iterator i = find( _allConnections.begin(),
                                             _allConnections.end(), connection);
        if( i == _allConnections.end( ))
            return false;

        if( _connection == connection )
            _connection = 0;

        ConnectionVector::iterator j = find( _connections.begin(),
                                             _connections.end(), connection );
        if( j == _connections.end( ))
        {
#ifdef BIG_CLUSTER_SUPPORT
            ThreadVector::iterator k = _threads.begin();
            for( ; k != _threads.end(); ++k )
            {
                Thread* thread = *k;
                if( thread->set->removeConnection( connection ))
                {
                    if( !thread->set->isEmpty( ))
                        return true;

                    if( thread == _thread )
                        _thread = 0;

                    thread->event = EVENT_NONE;
                    thread->join();
                    delete thread;
                    break;
                }
            }

            EQASSERT( k != _threads.end( ));
            _threads.erase( k );
#else
            EQUNREACHABLE;
#endif
        }
        else
        {
            _connections.erase( j );
            connection->removeListener( this );
        }

        _allConnections.erase( i );
    }

    _dirtyFDSet();
    return true;
}

void ConnectionSet::clear()
{
    _connection = 0;

#ifdef BIG_CLUSTER_SUPPORT
    for( ThreadVector::iterator i = _threads.begin(); i != _threads.end(); ++i )
    {
        Thread* thread = *i;
        thread->set->clear();
        thread->event = EVENT_NONE;
        thread->join();
        delete thread;
    }
    _threads.clear();
#endif

    for( ConnectionVector::iterator i = _connections.begin(); 
         i != _connections.end(); ++i )
    {
        (*i)->removeListener( this );
    }

    _allConnections.clear();
    _connections.clear();
    _dirtyFDSet();
    _fdSet.clear();
    _fdSetResult.clear();
}

#ifdef BIG_CLUSTER_SUPPORT
ConnectionSet::Thread::Thread( ConnectionSet* parent )
        : set( new ConnectionSet )
        , notifier( CreateEvent( 0, false, false, 0 ))
        , event( EVENT_NONE )
        , _parent( parent )
{
   
}

ConnectionSet::Thread::~Thread()
{
    delete set;
    set = 0;
}


void* ConnectionSet::Thread::run()
{
    while ( !set->isEmpty( ))
    {
        event = set->select( INFINITE );
        if( event != EVENT_INTERRUPT && event != EVENT_NONE )
        {
            SetEvent( notifier );
            event.waitEQ( EVENT_NONE );
        }
    }

    return EXIT_SUCCESS;
}
#endif

ConnectionSet::Event ConnectionSet::select( const int timeout )
{
    while( true )
    {
        _connection = 0;
        _error      = 0;
#ifdef BIG_CLUSTER_SUPPORT
        if( _thread )
        {
            _thread->event = EVENT_NONE; // unblock previous thread
            _thread = 0;
        }
#endif

        if( !_setupFDSet( ))
            return EVENT_INVALID_HANDLE;

        // poll for a result
#ifdef WIN32
        const DWORD waitTime = timeout > 0 ? timeout : INFINITE;
        const DWORD ret = WaitForMultipleObjectsEx( _fdSet.getSize(),
                                                    _fdSet.getData(),
                                                    FALSE, waitTime, TRUE );
#else
        const int ret = poll( _fdSet.getData(), _fdSet.getSize(), timeout );
#endif
        switch( ret )
        {
            case SELECT_TIMEOUT:
                return EVENT_TIMEOUT;

            case SELECT_ERROR:
#ifdef WIN32
#  ifdef BIG_CLUSTER_SUPPORT
                if( !_thread )
#  endif
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

                EQERROR << "Error during select: " << base::sysError
                        << std::endl;
                return EVENT_SELECT_ERROR;

            default: // SUCCESS
                {
                    Event event = _getSelectResult( ret );

                    if( event == EVENT_NONE )
                         break;

                    if( _connection == _selfConnection.get( ))
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
                           << getSize() << ", event " << event << std::endl;
                    return event;
                }
        }
    }
}
     
ConnectionSet::Event ConnectionSet::_getSelectResult( const uint32_t index )
{
#ifdef WIN32
    const uint32_t i = index - WAIT_OBJECT_0;
    EQASSERT( i < MAXIMUM_WAIT_OBJECTS );
    EQASSERT( i < _fdSetResult.getSize( ));

    if( i > _connections.size( ))
    {
        _thread = _fdSetResult[i].thread;
        EQASSERT( _thread->event != EVENT_NONE );
        EQASSERT( _fdSet[ i ] == _thread->notifier );
        
        ResetEvent( _thread->notifier ); 
        _connection = _thread->set->getConnection();
        _error = _thread->set->getError();
        return _thread->event.get();
    }
    // else locally handled connection

    _connection = _fdSetResult[i].connection;
    EQASSERT( _fdSet[i] == _connection->getNotifier( ));

    return EVENT_DATA;
#else
    for( size_t i = 0; i < _fdSet.getSize(); ++i )
    {
        const pollfd& pollFD = _fdSet[i];
        if( pollFD.revents == 0 )
            continue;

        const int pollEvents = pollFD.revents;
        EQASSERT( pollFD.fd > 0 );

        _connection = _fdSetResult[i].connection;
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
    EQASSERT( _connection == _selfConnection.get( ));
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
    _fdSet.setSize( 0 );
    _fdSetResult.setSize( 0 );

#ifdef WIN32
    // add self connection
    HANDLE readHandle = _selfConnection->getNotifier();
    EQASSERT( readHandle );
    _fdSet.append( readHandle );

    Result result;
    result.connection = _selfConnection.get();;
    _fdSetResult.append( result );

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

        Result result;
        result.connection = connection.get();;
        _fdSetResult.append( result );
    }
    for( ThreadVector::const_iterator i = _threads.begin();
         i != _threads.end(); ++i )
    {
        Thread* thread = *i;
        readHandle = thread->notifier;
        EQASSERT( readHandle );
        _fdSet.append( readHandle );

        Result result;
        result.thread = thread;
        _fdSetResult.append( result );
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

    Result result;
    result.connection = _selfConnection.get();
    _fdSetResult.append( result );

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

        result.connection = connection.get();
        _fdSetResult.append( result );
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
