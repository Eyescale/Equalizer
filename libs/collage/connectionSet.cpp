
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "eventConnection.h"

#include <co/base/os.h>
#include <co/base/stdExt.h>
#include <co/base/thread.h>

#include <algorithm>
#include <errno.h>

#ifdef _WIN32
#  define SELECT_TIMEOUT WAIT_TIMEOUT
#  define SELECT_ERROR   WAIT_FAILED
#  define MAX_CONNECTIONS (MAXIMUM_WAIT_OBJECTS - 1)
#else
#  define SELECT_TIMEOUT  0
#  define SELECT_ERROR   -1
#  define MAX_CONNECTIONS EQ_100KB  // Arbitrary
#endif

namespace co
{

ConnectionSet::ConnectionSet()
#ifdef _WIN32
        : _thread( 0 ),
#else
        :
#endif
          _selfConnection( new EventConnection )
        , _error( 0 )
        , _dirty( true )
{
    // Whenever another threads modifies the connection list while the
    // connection set is waiting in a select, the select is interrupted using
    // this connection.
    EQCHECK( _selfConnection->connect( ));
}

ConnectionSet::~ConnectionSet()
{
    clear();

    _connection = 0;

    _selfConnection->close();
    _selfConnection = 0;
}

#ifdef _WIN32

/** Handles connections exceeding MAXIMUM_WAIT_OBJECTS */
class ConnectionSetThread : public co::base::Thread
{
public:
    ConnectionSetThread( ConnectionSet* parent )
        : set( new ConnectionSet )
        , notifier( CreateEvent( 0, false, false, 0 ))
        , event( ConnectionSet::EVENT_NONE )
        , _parent( parent )
    {}

    virtual ~ConnectionSetThread()
        {
            delete set;
            set = 0;
        }

    ConnectionSet* set;
    HANDLE         notifier;

    co::base::Monitor< ConnectionSet::Event > event;

protected:
    virtual void run()
        {
            while ( !set->isEmpty( ))
            {
                event = set->select( INFINITE );
                if( event != ConnectionSet::EVENT_INTERRUPT &&
                    event != ConnectionSet::EVENT_NONE )
                {
                    SetEvent( notifier );
                    event.waitEQ( ConnectionSet::EVENT_NONE );
                }
            }
        }

private:
    ConnectionSet* const _parent;
};

#endif // _WIN32

void ConnectionSet::setDirty()
{
    if( _dirty )
        return;

    EQVERB << "FD set modified, restarting select" << std::endl;
    _dirty = true;
    interrupt();
}

void ConnectionSet::interrupt()
{
    _selfConnection->set();
}

void ConnectionSet::addConnection( ConnectionPtr connection )
{
    EQASSERT( connection->isConnected() || connection->isListening( ));

    { 
        co::base::ScopedMutex<> mutex( _mutex );
        _allConnections.push_back( connection );

#ifdef _WIN32
        EQASSERT( _allConnections.size() < MAX_CONNECTIONS * MAX_CONNECTIONS );
        if( _connections.size() < MAX_CONNECTIONS - _threads.size( ))
        {   // can handle it ourself
            _connections.push_back( connection );
            connection->addListener( this );
        }
        else
        {
            // add to existing thread
            for( Threads::const_iterator i = _threads.begin();
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
#endif // _WIN32
    }

    setDirty();
}

bool ConnectionSet::removeConnection( ConnectionPtr connection )
{
    {
        co::base::ScopedMutex<> mutex( _mutex );
        Connections::iterator i = stde::find( _allConnections, connection );
        if( i == _allConnections.end( ))
            return false;

        if( _connection == connection )
            _connection = 0;

        Connections::iterator j = stde::find( _connections, connection );
        if( j == _connections.end( ))
        {
#ifdef _WIN32
            Threads::iterator k = _threads.begin();
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

    setDirty();
    return true;
}

void ConnectionSet::clear()
{
    _connection = 0;

#ifdef _WIN32
    for( Threads::iterator i = _threads.begin(); i != _threads.end(); ++i )
    {
        Thread* thread = *i;
        thread->set->clear();
        thread->event = EVENT_NONE;
        thread->join();
        delete thread;
    }
    _threads.clear();
#endif

    for( Connections::iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        (*i)->removeListener( this );
    }

    _allConnections.clear();
    _connections.clear();
    setDirty();
    _fdSet.clear();
    _fdSetResult.clear();
}

ConnectionSet::Event ConnectionSet::select( const int timeout )
{
    EQ_TS_SCOPED( _selectThread );
    while( true )
    {
        _connection = 0;
        _error      = 0;
#ifdef _WIN32
        if( _thread )
        {
            _thread->event = EVENT_NONE; // unblock previous thread
            _thread = 0;
        }
#endif

        if( !_setupFDSet( ))
            return EVENT_INVALID_HANDLE;

        // poll for a result
#ifdef _WIN32
        const DWORD waitTime = timeout < 0 ? INFINITE : timeout;
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
#ifdef _WIN32
                if( !_thread )
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

                EQERROR << "Error during select: " << co::base::sysError
                        << std::endl;
                return EVENT_SELECT_ERROR;

            default: // SUCCESS
                {
                    Event event = _getSelectResult( ret );

                    if( event == EVENT_NONE )
                         break;

                    if( _connection == _selfConnection.get( ))
                    {
                        _connection = 0;
                        _selfConnection->reset();
                        return EVENT_INTERRUPT;
                    }
                    if( event == EVENT_DATA && _connection->isListening( ))
                        event = EVENT_CONNECT; 
                    return event;
                }
        }
    }
}
     
#ifdef _WIN32
ConnectionSet::Event ConnectionSet::_getSelectResult( const uint32_t index )
{
    const uint32_t i = index - WAIT_OBJECT_0;
    EQASSERT( i < MAXIMUM_WAIT_OBJECTS );

    // Bug: WaitForMultipleObjects returns occasionally 16 with _fdSet size 2,
    //   when used by the RSPConnection
    // WAR: Catch this and ignore the result, this seems to have no side-effects
    if( i >= _fdSetResult.getSize( ))
        return EVENT_NONE;

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
}
#else // _WIN32
ConnectionSet::Event ConnectionSet::_getSelectResult( const uint32_t )
{
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
}
#endif // else not _WIN32

bool ConnectionSet::_setupFDSet()
{
    if( !_dirty )
    {
#ifndef _WIN32
        // TODO: verify that poll() really modifies _fdSet, and remove the copy
        // if it doesn't. The man page seems to hint that poll changes fds.
        _fdSet = _fdSetCopy;
#endif
        return true;
    }

    _dirty = false;
    _fdSet.setSize( 0 );
    _fdSetResult.setSize( 0 );

#ifdef _WIN32
    // add self connection
    HANDLE readHandle = _selfConnection->getNotifier();
    EQASSERT( readHandle );
    _fdSet.append( readHandle );

    Result res;
    res.connection = _selfConnection.get();
    _fdSetResult.append( res );

    // add regular connections
    _mutex.set();
    for( Connections::const_iterator i = _connections.begin();
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
        result.connection = connection.get();
        _fdSetResult.append( result );
    }

    for( Threads::const_iterator i = _threads.begin(); i != _threads.end(); ++i)
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
    fd.fd = _selfConnection->getNotifier();
    EQASSERT( fd.fd > 0 );
    fd.revents = 0;
    _fdSet.append( fd );

    Result result;
    result.connection = _selfConnection.get();
    _fdSetResult.append( result );

    // add regular connections
    _mutex.set();
    for( Connections::const_iterator i = _connections.begin();
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

std::ostream& operator << ( std::ostream& os,
                                      const ConnectionSet* set)
{
    const Connections& connections = set->getConnections();

    os << "connection set " << (void*)set << ", " << connections.size()
       << " connections";
    
    for( Connections::const_iterator i = connections.begin(); 
         i != connections.end(); ++i )
    {
        os << std::endl << "    " << (*i).get();
    }
    
    return os;
}

std::ostream& operator << ( std::ostream& os, 
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
