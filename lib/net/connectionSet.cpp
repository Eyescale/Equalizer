
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
#  define EQ_SOCKET_ERROR getErrorString( _error )
#else
#  define EQ_SOCKET_ERROR strerror( _error )
#endif

using namespace eqBase;
using namespace eqNet;
using namespace std;

ConnectionSet::ConnectionSet()
        : _error(0)
{
    // Whenever another threads modifies the connection list while the
    // connection set is waiting in a select, the select is interrupted by
    // sending a character through this connection. select() will recognize
    // this and restart with the modified fd set.
    _selfConnection = new PipeConnection;
    if( !_selfConnection->connect( ))
    {
        EQERROR << "Could not create connection" << endl;
        return;
    }
}

ConnectionSet::~ConnectionSet()
{
    _connection = 0;

    _selfConnection->close();
    _selfConnection = 0;
}


void ConnectionSet::_dirtyFDSet()
{
    const char c = SELF_MODIFIED;
    _selfConnection->send( &c, 1, true );
}

void ConnectionSet::interrupt()
{
    const char c = SELF_INTERRUPT;
    _selfConnection->send( &c, 1, true );
}

void ConnectionSet::addConnection( RefPtr<Connection> connection )
{
    EQASSERT( connection->getState() == Connection::STATE_CONNECTED ||
            connection->getState() == Connection::STATE_LISTENING );

    _mutex.set();
    _connections.push_back( connection );
    _mutex.unset();
    _dirtyFDSet();
}

bool ConnectionSet::removeConnection( eqBase::RefPtr<Connection> connection )
{
    _mutex.set();
    vector< eqBase::RefPtr<Connection> >::iterator eraseIter =
        find( _connections.begin(), _connections.end(), connection );

    if( eraseIter == _connections.end( ))
    {
        _mutex.unset();
        return false;
    }

    _connections.erase( eraseIter );
    _mutex.unset();

    if( _connection == connection )
        _connection = 0;

    _dirtyFDSet();
    return true;
}

void ConnectionSet::clear()
{
    _connection = 0;
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
        const DWORD ret = WaitForMultipleObjectsEx( _fdSet.size(), &_fdSet[0],
                                                    FALSE, waitTime, TRUE );
#else
        const int ret = poll( &_fdSet[0], _fdSet.size(), timeout );
#endif
        switch( ret )
        {
            case Connection::SELECT_TIMEOUT:
                return EVENT_TIMEOUT;

            case Connection::SELECT_ERROR:
#ifdef WIN32
                _error = GetLastError();
#else
                if( errno == EINTR ) // Interrupted system call (gdb) - ignore
                    break;

                _error = errno;
#endif

                EQERROR << "Error during select: " << EQ_SOCKET_ERROR << endl;
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
                    
                    if( event == EVENT_DATA && 
                        _connection->getState() == Connection::STATE_LISTENING )

                        event = EVENT_CONNECT;

                    EQVERB << "selected connection " << _connection.get() 
                           << " of " << _fdSetConnections.size()+1 << ", event "
                           << event << endl;
                    return event;
                }
        }
    }
}
     
ConnectionSet::Event ConnectionSet::_getSelectResult( const uint32_t index )
{
#ifdef WIN32
    const uint32_t i      = index - WAIT_OBJECT_0;
    const HANDLE   handle = _fdSet[i];

    _connection = _fdSetConnections[handle];

    return EVENT_DATA;
#else
    for( size_t i=0; i<_fdSet.size(); ++i )
    {
        if( _fdSet[i].revents == 0 )
            continue;

        const int fd         = _fdSet[i].fd;
        const int pollEvents = _fdSet[i].revents;

        EQASSERT( fd > 0 );
        _connection = _fdSetConnections[ fd ];
        EQASSERT( _connection.isValid( ));

        EQVERB << "Got event on connection @" << (void*)_connection.get()<<endl;

        if( pollEvents & POLLERR )
        {
            EQINFO << "Error during poll()" << endl;
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

        EQERROR << "Unhandled poll event(s): " << pollEvents <<endl;
        ::abort();
    }
    return EVENT_NONE;
#endif
}

ConnectionSet::Event ConnectionSet::_handleSelfCommand()
{
    char c = 0;
    _connection->recv( &c, 1 );
    _connection = 0;

    switch( c ) 
    {
        case SELF_MODIFIED:
            // The connection set was modified in the select, restart selection
            // TODO: decrease timeout accordingly.
            EQINFO << "FD set modified, restarting select" << endl;
            return EVENT_NONE; // handled

        case SELF_INTERRUPT:
            return EVENT_INTERRUPT;

        default:
            EQUNIMPLEMENTED;
            return EVENT_NONE;
    }
}

bool ConnectionSet::_setupFDSet()
{
    // The fdSet has to be rebuild every time since a connection may have been
    // closed in the meantime.
    _fdSetConnections.clear();

#ifdef WIN32
    _fdSet.clear();
    // add self connection
    HANDLE readHandle = _selfConnection->getReadNotifier();
    EQASSERT( readHandle );

    _fdSetConnections[readHandle] = _selfConnection.get();
    _fdSet.push_back( readHandle );

    // add regular connections
    _mutex.set();
    for( vector< RefPtr<Connection> >::const_iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        eqBase::RefPtr<Connection> connection = *i;
        readHandle = connection->getReadNotifier();

        if( !readHandle )
        {
            EQWARN << "Cannot select connection " << connection
                 << ", connection does not provide a read handle" << endl;
            _connection = connection;
		    _mutex.unset();
            return false;
        }
        
        _fdSetConnections[readHandle] = connection.get();
        _fdSet.push_back( readHandle );
    }
    _mutex.unset();
#else
    _fdSet.clear();

    pollfd fd;
    fd.events = POLLIN; // | POLLPRI;

    // add self 'connection'
    fd.fd      = _selfConnection->getReadNotifier();
    EQASSERT( fd.fd > 0 );
    fd.revents = 0;
    _fdSetConnections[fd.fd] = _selfConnection.get();
    _fdSet.push_back( fd );

    // add regular connections
    _mutex.set();
    for( vector< RefPtr<Connection> >::const_iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        eqBase::RefPtr<Connection> connection = *i;
        fd.fd = connection->getReadNotifier();

        if( fd.fd <= 0 )
        {
            EQWARN << "Cannot select connection " << connection
                   << ", connection " << typeid( *connection.get( )).name() 
                   << " does not use a file descriptor" << endl;
            _connection = connection;
		    _mutex.unset();
            return false;
        }

        EQVERB << "Listening on " << typeid( *connection.get( )).name() 
               << " @" << (void*)connection.get() << endl;
        _fdSetConnections[fd.fd] = connection.get();
        fd.revents = 0;
        _fdSet.push_back( fd );
    }
    _mutex.unset();
#endif
    return true;
}   

std::ostream& eqNet::operator << ( std::ostream& os, ConnectionSet* set)
{
    const size_t nConnections = set->nConnections();
    
    os << "connection set " << (void*)set << ", " << nConnections
       << " connections";
    
    for( size_t i=0; i<nConnections; i++ )
    {
        eqBase::RefPtr<Connection> connection = set->getConnection(i);
        os << endl << "    " << connection.get();
    }
    
    return os;
}
