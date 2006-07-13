 
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionSet.h"
#include "connection.h"

#include <eq/base/base.h>
#include <algorithm>
#include <errno.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

ConnectionSet::ConnectionSet()
        : _fdSetDirty(true),
          _inSelect(false),
          _errno(0)
{
    // Whenever another threads modifies the connection list while the
    // connection set is waiting in a select, the select is interrupted by
    // sending a character through this pipe. Select() will recognize this and
    // restart with the modified fd set.
    if( pipe( _selfFD ) ==  -1 )
    {
        EQERROR << "Could not create pipe: " << strerror( errno );
        return;
    }
}

ConnectionSet::~ConnectionSet()
{
    _connection = NULL;
    close( _selfFD[0] );
    close( _selfFD[1] );
}


void ConnectionSet::_dirtyFDSet()
{
    _fdSetDirty = true;
    if( !_inSelect )
        return;

    const char c = 'd';
    write( _selfFD[1], &c, 1 );
}

void ConnectionSet::addConnection( RefPtr<Connection> connection )
{
    EQASSERT( connection->getState() == Connection::STATE_CONNECTED ||
            connection->getState() == Connection::STATE_LISTENING );

    _connections.push_back( connection );
    _dirtyFDSet();
}

bool ConnectionSet::removeConnection( eqBase::RefPtr<Connection> connection )
{
    vector< eqBase::RefPtr<Connection> >::iterator eraseIter =
        find( _connections.begin(), _connections.end(), connection );
    if( eraseIter == _connections.end( ))
        return false;

    _connections.erase( eraseIter );
    if( _connection == connection )
        _connection = NULL;

    _dirtyFDSet();
    return true;
}

void ConnectionSet::clear()
{
    EQASSERT( !_inSelect );
    _connection = NULL;
    _connections.clear();
    _dirtyFDSet();
    _fdSet.clear();
    _fdSetConnections.clear();
}
        
ConnectionSet::Event ConnectionSet::select( const int timeout )
{
    _inSelect = true;
    Event event = EVENT_NONE;

    while( event == EVENT_NONE )
    {
        _connection = NULL;
        _setupFDSet();

        // poll for a result
        const int ret = poll( &_fdSet[0], _fdSet.size(), timeout );
        switch( ret )
        {
            case 0:
                event = EVENT_TIMEOUT;
                break;

            case -1: // ERROR
//#ifndef NDEBUG
                if( errno == EINTR ) // Interrupted system call (gdb) - ignore
                {
                    event = EVENT_NONE;
                    break;
                }
//#endif
                _errno = errno;
                EQINFO << "Error during poll(): " << strerror( _errno ) << endl;
                event = EVENT_ERROR;
                break;

            default: // SUCCESS
                for( size_t i=0; i<_fdSet.size(); i++ )
                {
                    if( _fdSet[i].revents == 0 )
                        continue;
                
                    const int fd         = _fdSet[i].fd;
                    const int pollEvents = _fdSet[i].revents;

                    // The connection set was modified in the select, restart
                    // the selection.
                    // TODO: decrease timeout accordingly.
                    if( fd == _selfFD[0] )
                    {
                        EQINFO << "FD set modified, restarting select" << endl;
                        EQASSERT( pollEvents == POLLIN );
                        char c = '\0';
                        read( fd, &c, 1 );
                        EQASSERT( c == 'd' );
                        break;
                    }

                    _connection = _fdSetConnections[fd];
                    
                    EQVERB << "selected connection #" << i << " of " 
                           << _fdSet.size() << ", poll event " << pollEvents
                           << ", " << _connection.get() << endl;

                    if( pollEvents & POLLERR )
                    {
                        _errno = 0;
                        EQINFO << "Error during poll()" << endl;
                        event = EVENT_ERROR;
                        break;
                    }

                    if( pollEvents & POLLHUP ) // disconnect happened
                    {
                        event = EVENT_DISCONNECT;
                        break;
                    }
    
                    if( pollEvents & POLLNVAL ) // disconnected connection
                    {
                        event = EVENT_DISCONNECT;
                        break;
                    }

                    // Note: Intuitively I would handle the read before HUP to
                    // read remaining data of the connection, but at least on
                    // OS X both events happen simultaneously and no more data
                    // can be read.
                    if( pollEvents & POLLIN || pollEvents & POLLPRI )
                    {  
                        // data is ready for reading
                        if( _connection->getState() == 
                            Connection::STATE_LISTENING )
                            event = EVENT_CONNECT;
                        else
                            event = EVENT_DATA;
                        break;
                    }

                    EQERROR << "Unhandled poll event(s): " << pollEvents <<endl;
                    abort();
                }
        }
    }
    _inSelect = false;
    return event;
}
     
void ConnectionSet::_setupFDSet()
{
    if( _fdSetDirty )
    {
        _fdSetDirty = false;
        _buildFDSet();
        return;
    }

    static const int events = POLLIN; // | POLLPRI;
    for( size_t i=0; i<_fdSet.size(); i++ )
    {
        _fdSet[i].events  = events;
        _fdSet[i].revents = 0;
    }
}

void ConnectionSet::_buildFDSet()
{
    _fdSet.clear();
    _fdSetConnections.clear();

    pollfd fd;
    fd.events = POLLIN; // | POLLPRI;

    // add self 'connection'
    fd.fd      = _selfFD[0];
    fd.revents = 0;
    _fdSet.push_back( fd );

    // add regular connections
    const size_t nConnections = _connections.size();
    for( size_t i=0; i<nConnections; i++ )
    {
        eqBase::RefPtr<Connection> connection = _connections[i];
        fd.fd = connection->getReadFD();

        if( fd.fd == -1 )
        {
            EQWARN << "Cannot select connection " << i
                 << ", connection does not use a file descriptor" << endl;
            continue;
        }

        _fdSetConnections[fd.fd] = connection.get();
        fd.revents = 0;
        _fdSet.push_back( fd );
    }
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
