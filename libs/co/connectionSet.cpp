
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <lunchbox/buffer.h>
#include <lunchbox/os.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/stdExt.h>
#include <lunchbox/thread.h>

#include <algorithm>
#include <errno.h>

#ifdef _WIN32
#  include <lunchbox/monitor.h>
#  define SELECT_TIMEOUT WAIT_TIMEOUT
#  define SELECT_ERROR   WAIT_FAILED
#  define MAX_CONNECTIONS (MAXIMUM_WAIT_OBJECTS - 1)
#else
#  include <poll.h>
#  define SELECT_TIMEOUT  0
#  define SELECT_ERROR   -1
#  define MAX_CONNECTIONS EQ_100KB  // Arbitrary
#endif

namespace co
{
namespace
{
#ifdef _WIN32

/** Handles connections exceeding MAXIMUM_WAIT_OBJECTS */
class Thread : public lunchbox::Thread
{
public:
    Thread( ConnectionSet* parent )
        : set( new ConnectionSet )
        , notifier( CreateEvent( 0, false, false, 0 ))
        , event( ConnectionSet::EVENT_NONE )
        , _parent( parent )
    {}

    virtual ~Thread()
        {
            delete set;
            set = 0;
        }

    ConnectionSet* set;
    HANDLE         notifier;

    lunchbox::Monitor< ConnectionSet::Event > event;

protected:
    virtual void run()
        {
            while ( !set->isEmpty( ))
            {
                event = set->select();
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

typedef std::vector< Thread* > Threads;
typedef Threads::const_iterator ThreadsCIter;
typedef Threads::iterator ThreadsIter;

union Result
{
    Connection* connection;
    Thread* thread;
};
#else
union Result
{
    Connection* connection;
};

#endif // _WIN32

}

namespace detail
{
class ConnectionSet
{
public:
    ConnectionSet()
           : selfConnection( new EventConnection )
#ifdef _WIN32
           , thread( 0 )
#endif
           , error( 0 )
           , dirty( true )
        {
            // Whenever another threads modifies the connection list while the
            // connection set is waiting in a select, the select is interrupted
            // using this connection.
            LBCHECK( selfConnection->connect( ));
        }

    ~ConnectionSet()
        {
            connection = 0;
            selfConnection->close();
            selfConnection = 0;
        }

    /** Mutex protecting changes to the set. */
    lunchbox::Lock lock;

    /** The connections of this set */
    Connections allConnections;

    /** The connections to handle */
    Connections connections;

    // Note: std::vector had to much overhead here
#ifdef _WIN32
    lunchbox::Buffer< HANDLE > fdSet;
#else
    lunchbox::Buffer< pollfd > fdSetCopy; // 'const' set
    lunchbox::Buffer< pollfd > fdSet;     // copy of _fdSetCopy used to poll
#endif
    lunchbox::Buffer< Result > fdSetResult;

    /** The connection to reset a running select, see constructor. */
    lunchbox::RefPtr< EventConnection > selfConnection;

#ifdef _WIN32
    /** Threads used to handle more than MAXIMUM_WAIT_OBJECTS connections */
    Threads threads;

    /** Result thread. */
    Thread* thread;

#endif

    // result values
    ConnectionPtr connection;
    int error;

    /** FD sets need rebuild. */
    bool dirty;
};
}

ConnectionSet::ConnectionSet()
        : _impl( new detail::ConnectionSet )
{}
ConnectionSet::~ConnectionSet()
{
    clear();
    delete _impl;
}

size_t ConnectionSet::getSize() const
{
    return _impl->connections.size();
}

bool ConnectionSet::isEmpty() const
{
    return _impl->connections.empty();
}

const Connections& ConnectionSet::getConnections() const
{
    return _impl->allConnections;
}

int ConnectionSet::getError() const
{
    return _impl->error;
}

ConnectionPtr ConnectionSet::getConnection()
{
    return _impl->connection;
}

void ConnectionSet::setDirty()
{
    if( _impl->dirty )
        return;

    LBVERB << "FD set modified, restarting select" << std::endl;
    _impl->dirty = true;
    interrupt();
}

void ConnectionSet::notifyStateChanged( Connection* )
{
    _impl->dirty = true;
}

void ConnectionSet::interrupt()
{
    _impl->selfConnection->set();
}

void ConnectionSet::addConnection( ConnectionPtr connection )
{
    LBASSERT( connection->isConnected() || connection->isListening( ));

    { 
        lunchbox::ScopedWrite mutex( _impl->lock );
        _impl->allConnections.push_back( connection );

#ifdef _WIN32
        LBASSERT( _impl->allConnections.size() <
                  MAX_CONNECTIONS * MAX_CONNECTIONS );
        if( _impl->connections.size() < MAX_CONNECTIONS - _impl->threads.size())
        {
            // can handle it ourself
            _impl->connections.push_back( connection );
            connection->addListener( this );
        }
        else
        {
            // add to existing thread
            for( ThreadsCIter i = _impl->threads.begin();
                 i != _impl->threads.end(); ++i )
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
            thread->set->addConnection( _impl->connections.back( ));
            _impl->connections.pop_back();

            _impl->threads.push_back( thread );
            thread->start();
        }
#else
        _impl->connections.push_back( connection );
        connection->addListener( this );

        LBASSERT( _impl->connections.size() < MAX_CONNECTIONS );
#endif // _WIN32
    }

    setDirty();
}

bool ConnectionSet::removeConnection( ConnectionPtr connection )
{
    {
        lunchbox::ScopedWrite mutex( _impl->lock );
        ConnectionsIter i = stde::find( _impl->allConnections, connection );
        if( i == _impl->allConnections.end( ))
            return false;

        if( _impl->connection == connection )
            _impl->connection = 0;

        ConnectionsIter j = stde::find( _impl->connections, connection );
        if( j == _impl->connections.end( ))
        {
#ifdef _WIN32
            Threads::iterator k = _impl->threads.begin();
            for( ; k != _impl->threads.end(); ++k )
            {
                Thread* thread = *k;
                if( thread->set->removeConnection( connection ))
                {
                    if( !thread->set->isEmpty( ))
                        return true;

                    if( thread == _impl->thread )
                        _impl->thread = 0;

                    thread->event = EVENT_NONE;
                    thread->join();
                    delete thread;
                    break;
                }
            }

            LBASSERT( k != _impl->threads.end( ));
            _impl->threads.erase( k );
#else
            LBUNREACHABLE;
#endif
        }
        else
        {
            _impl->connections.erase( j );
            connection->removeListener( this );
        }

        _impl->allConnections.erase( i );
    }

    setDirty();
    return true;
}

void ConnectionSet::clear()
{
    _impl->connection = 0;

#ifdef _WIN32
    for( ThreadsIter i =_impl->threads.begin(); i != _impl->threads.end(); ++i )
    {
        Thread* thread = *i;
        thread->set->clear();
        thread->event = EVENT_NONE;
        thread->join();
        delete thread;
    }
    _impl->threads.clear();
#endif

    for( ConnectionsIter i = _impl->connections.begin();
         i != _impl->connections.end(); ++i )
    {
        (*i)->removeListener( this );
    }

    _impl->allConnections.clear();
    _impl->connections.clear();
    setDirty();
    _impl->fdSet.clear();
    _impl->fdSetResult.clear();
}

ConnectionSet::Event ConnectionSet::select( const uint32_t timeout )
{
    LB_TS_SCOPED( _selectThread );
    while( true )
    {
        _impl->connection = 0;
        _impl->error      = 0;
#ifdef _WIN32
        if( _impl->thread )
        {
            _impl->thread->event = EVENT_NONE; // unblock previous thread
            _impl->thread = 0;
        }
#endif

        if( !_setupFDSet( ))
            return EVENT_INVALID_HANDLE;

        // poll for a result
#ifdef _WIN32
        LBASSERT( LB_TIMEOUT_INDEFINITE == INFINITE );
        const DWORD ret = WaitForMultipleObjectsEx( _impl->fdSet.getSize(),
                                                    _impl->fdSet.getData(),
                                                    FALSE, timeout, TRUE );
#else
        const int pollTimeout = timeout == LB_TIMEOUT_INDEFINITE ?
                                -1 : int( timeout );
        const int ret = poll( _impl->fdSet.getData(), _impl->fdSet.getSize(),
                              pollTimeout );
#endif
        switch( ret )
        {
            case SELECT_TIMEOUT:
                return EVENT_TIMEOUT;

            case SELECT_ERROR:
#ifdef _WIN32
                if( !_impl->thread )
                    _impl->error = GetLastError();

                if( _impl->error == WSA_INVALID_HANDLE )
                {
                    _impl->dirty = true;
                    break;
                }
#else
                if( errno == EINTR ) // Interrupted system call (gdb) - ignore
                    break;

                _impl->error = errno;
#endif

                LBERROR << "Error during select: " << lunchbox::sysError
                        << std::endl;
                return EVENT_SELECT_ERROR;

            default: // SUCCESS
                {
                    Event event = _getSelectResult( ret );

                    if( event == EVENT_NONE )
                         break;

                    if( _impl->connection == _impl->selfConnection.get( ))
                    {
                        _impl->connection = 0;
                        _impl->selfConnection->reset();
                        return EVENT_INTERRUPT;
                    }
                    if( event == EVENT_DATA && _impl->connection->isListening())
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
    LBASSERT( i < MAXIMUM_WAIT_OBJECTS );

    // Bug: WaitForMultipleObjects returns occasionally 16 with fdSet size 2,
    //   when used by the RSPConnection
    // WAR: Catch this and ignore the result, this seems to have no side-effects
    if( i >= _impl->fdSetResult.getSize( ))
        return EVENT_NONE;

    if( i > _impl->connections.size( ))
    {
        _impl->thread = _impl->fdSetResult[i].thread;
        LBASSERT( _impl->thread->event != EVENT_NONE );
        LBASSERT( _impl->fdSet[ i ] == _impl->thread->notifier );
        
        ResetEvent( _impl->thread->notifier ); 
        _impl->connection = _impl->thread->set->getConnection();
        _impl->error = _impl->thread->set->getError();
        return _impl->thread->event.get();
    }
    // else locally handled connection

    _impl->connection = _impl->fdSetResult[i].connection;
    LBASSERT( _impl->fdSet[i] == _impl->connection->getNotifier() ||
              _impl->connection->isClosed( ));
    return EVENT_DATA;
}
#else // _WIN32
ConnectionSet::Event ConnectionSet::_getSelectResult( const uint32_t )
{
    for( size_t i = 0; i < _impl->fdSet.getSize(); ++i )
    {
        const pollfd& pollFD = _impl->fdSet[i];
        if( pollFD.revents == 0 )
            continue;

        const int pollEvents = pollFD.revents;
        LBASSERT( pollFD.fd > 0 );

        _impl->connection = _impl->fdSetResult[i].connection;
        LBASSERT( _impl->connection.isValid( ));

        LBVERB << "Got event on connection @" << (void*)_impl->connection.get()
               << std::endl;

        if( pollEvents & POLLERR )
        {
            LBINFO << "Error during poll(): " << lunchbox::sysError << std::endl;
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

        LBERROR << "Unhandled poll event(s): " << pollEvents << std::endl;
        ::abort();
    }
    return EVENT_NONE;
}
#endif // else not _WIN32

bool ConnectionSet::_setupFDSet()
{
    if( !_impl->dirty )
    {
#ifndef _WIN32
        // TODO: verify that poll() really modifies _fdSet, and remove the copy
        // if it doesn't. The man page seems to hint that poll changes fds.
        _impl->fdSet = _impl->fdSetCopy;
#endif
        return true;
    }

    _impl->dirty = false;
    _impl->fdSet.setSize( 0 );
    _impl->fdSetResult.setSize( 0 );

#ifdef _WIN32
    // add self connection
    HANDLE readHandle = _impl->selfConnection->getNotifier();
    LBASSERT( readHandle );
    _impl->fdSet.append( readHandle );

    Result res;
    res.connection = _impl->selfConnection.get();
    _impl->fdSetResult.append( res );

    // add regular connections
    _impl->lock.set();
    for( ConnectionsCIter i = _impl->connections.begin();
         i != _impl->connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        readHandle = connection->getNotifier();

        if( !readHandle )
        {
            LBINFO << "Cannot select connection " << connection
                 << ", connection does not provide a read handle" << std::endl;
            _impl->connection = connection;
            _impl->lock.unset();
            return false;
        }
        
        _impl->fdSet.append( readHandle );

        Result result;
        result.connection = connection.get();
        _impl->fdSetResult.append( result );
    }

    for( ThreadsCIter i=_impl->threads.begin(); i != _impl->threads.end(); ++i )
    {
        Thread* thread = *i;
        readHandle = thread->notifier;
        LBASSERT( readHandle );
        _impl->fdSet.append( readHandle );

        Result result;
        result.thread = thread;
        _impl->fdSetResult.append( result );
    }
    _impl->lock.unset();
#else
    pollfd fd;
    fd.events = POLLIN; // | POLLPRI;

    // add self 'connection'
    fd.fd = _impl->selfConnection->getNotifier();
    LBASSERT( fd.fd > 0 );
    fd.revents = 0;
    _impl->fdSet.append( fd );

    Result result;
    result.connection = _impl->selfConnection.get();
    _impl->fdSetResult.append( result );

    // add regular connections
    _impl->lock.set();
    for( ConnectionsCIter i = _impl->connections.begin();
         i != _impl->connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        fd.fd = connection->getNotifier();

        if( fd.fd <= 0 )
        {
            LBINFO << "Cannot select connection " << connection
                   << ", connection " << typeid( *connection.get( )).name() 
                   << " doesn't have a file descriptor" << std::endl;
            _impl->connection = connection;
            _impl->lock.unset();
            return false;
        }

        LBVERB << "Listening on " << typeid( *connection.get( )).name() 
               << " @" << (void*)connection.get() << std::endl;
        fd.revents = 0;

        _impl->fdSet.append( fd );

        result.connection = connection.get();
        _impl->fdSetResult.append( result );
    }
    _impl->lock.unset();
    _impl->fdSetCopy = _impl->fdSet;
#endif

    return true;
}   

std::ostream& operator << ( std::ostream& os, const ConnectionSet* set )
{
    const Connections& connections = set->getConnections();

    os << "connection set " << (void*)set << ", " << connections.size()
       << " connections";
    
    for( ConnectionsCIter i = connections.begin(); i != connections.end(); ++i )
    {
        os << std::endl << "    " << (*i).get();
    }
    
    return os;
}

std::ostream& operator << ( std::ostream& os, const ConnectionSet::Event event )
{
    if( event >= ConnectionSet::EVENT_ALL )
        os << "unknown (" << unsigned( event ) << ')';
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
