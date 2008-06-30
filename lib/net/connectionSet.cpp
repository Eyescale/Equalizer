
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
#  define EQ_SOCKET_ERROR getErrorString( _error ) << '(' << _error << ')'
#else
#  define EQ_SOCKET_ERROR strerror( _error )
#endif

using namespace eqBase;
using namespace std;

namespace eqNet
{

ConnectionSet::ConnectionSet()
        : _error(0)
        , _dirty( true )
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

void ConnectionSet::addConnection( ConnectionPtr connection )
{
    EQASSERT( connection->getState() == Connection::STATE_CONNECTED ||
            connection->getState() == Connection::STATE_LISTENING );

    _mutex.set();
    _connections.push_back( connection );
    connection->addListener( this );
    _mutex.unset();
    _dirtyFDSet();
}

bool ConnectionSet::removeConnection( ConnectionPtr connection )
{
    {
        ScopedMutex< SpinLock > mutex( _mutex );
        connection->removeListener( this );

        ConnectionVector::iterator i = find( _connections.begin(),
                                             _connections.end(), connection );
        if( i == _connections.end( ))
            return false;

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

                    EQVERB << "selected connection " << _connection << " of "
                           << _fdSetConnections.size()+1 << ", event " << event
                           << endl;
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
    for( vector< pollfd >::const_iterator i = _fdSet.begin();
         i != _fdSet.end(); ++i )
    {
        const pollfd& pollFD = *i;
        if( pollFD.revents == 0 )
            continue;

        const int fd         = pollFD.fd;
        const int pollEvents = pollFD.revents;

        EQASSERT( fd > 0 );
        EQASSERT( _fdSetConnections.find( fd ) != _fdSetConnections.end( ));
        EQASSERT( _fdSetConnections[ fd ].isValid( ));

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
            _dirty = true;
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
    if( !_dirty )
    {
#ifndef WIN32
        // TODO: verify that poll() really modifies _fdSet, and remove the copy
        // if it doesn't. The man page seems to hint that poll changes fds.
        _fdSet = _fdSetCopy;
#endif
        return true;
    }

    _fdSetConnections.clear();

#ifdef WIN32
    _fdSet.clear();
    // add self connection
    HANDLE readHandle = _selfConnection->getReadNotifier();
    EQASSERT( readHandle );

    _fdSetConnections[readHandle] = _selfConnection;
    _fdSet.push_back( readHandle );

    // add regular connections
    _mutex.set();
    for( vector< ConnectionPtr >::const_iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        readHandle = connection->getReadNotifier();

        if( !readHandle )
        {
            EQWARN << "Cannot select connection " << connection
                 << ", connection does not provide a read handle" << endl;
            _connection = connection;
		    _mutex.unset();
            return false;
        }
        
        _fdSetConnections[readHandle] = connection;
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
    _fdSetConnections[fd.fd] = _selfConnection;
    _fdSet.push_back( fd );

    // add regular connections
    _mutex.set();
    for( vector< ConnectionPtr >::const_iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
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
        _fdSetConnections[fd.fd] = connection;
        fd.revents = 0;
        _fdSet.push_back( fd );
    }
    _mutex.unset();
    _fdSetCopy = _fdSet;
#endif

    _dirty = false;
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
        os << endl << "    " << (*i).get();
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
