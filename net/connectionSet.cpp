 
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionSet.h"
#include "connection.h"
#include "node.h"

#include <eq/base/base.h>
#include <algorithm>
#include <errno.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

ConnectionSet::ConnectionSet()
        : _fdSetSize(0),
          _fdSetCapacity(64),
          _fdSetDirty(true),
          _errno(0)
{
    _fdSet = new pollfd[_fdSetCapacity];

    // Whenever another threads modifies the connection list while the
    // connection set is waiting in a select, the select is interrupted by
    // sending a character through this pipe. Select() will recognize this and
    // restart with the modified fd set.
    if( pipe( _selfFD ) ==  -1 )
    {
        ERROR << "Could not create pipe: " << strerror( errno );
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

void ConnectionSet::addConnection( eqBase::RefPtr<Connection> connection, Node* node )
{
    ASSERT( connection->getState() == Connection::STATE_CONNECTED ||
            connection->getState() == Connection::STATE_LISTENING );
    //ASSERT( connection == node->getConnection( ));

    _connections.push_back( connection );
    _nodes[connection.get()] = node;
    _dirtyFDSet();
}

bool ConnectionSet::removeConnection( eqBase::RefPtr<Connection> connection )
{
    vector< eqBase::RefPtr<Connection> >::iterator eraseIter =
        find( _connections.begin(), _connections.end(), connection );
    if( eraseIter == _connections.end( ))
        return false;

    _nodes.erase( connection.get( ));
    _connections.erase( eraseIter );
    if( _connection == connection )
        _connection = NULL;

    _dirtyFDSet();
    return true;
}

void ConnectionSet::clear()
{
    _connection = NULL;
    _nodes.clear();
    _connections.clear();
    _dirtyFDSet();
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
        const int ret = poll( _fdSet, _fdSetSize, timeout );
        switch( ret )
        {
            case 0:
                event = EVENT_TIMEOUT;
                break;

            case -1: // ERROR
                _errno = errno;
                INFO << "Error during poll(): " << strerror( _errno ) << endl;
                event = EVENT_ERROR;
                break;

            default: // SUCCESS
                for( size_t i=0; i<_fdSetSize; i++ )
                {
                    if( _fdSet[i].revents == 0 )
                        continue;
                
                    const int fd        = _fdSet[i].fd;
                    const int pollEvent = _fdSet[i].revents;

                    // The connection set was modified in the select, restart
                    // the selection.
                    // TODO: decrease timeout accordingly.
                    if( fd == _selfFD[0] )
                    {
                        INFO << "FD set modified, restarting select" << endl;
                        ASSERT( pollEvent == POLLIN );
                        char c = '\0';
                        read( fd, &c, 1 );
                        ASSERT( c == 'd' );
                        break;
                    }

                    _connection = _fdSetConnections[fd];
                    
                    INFO << "selected connection #" << i << " of " << _fdSetSize
                         << ", poll event " << pollEvent << ", " 
                         << _connection.get() << endl;

                    switch( pollEvent )
                    {
                        case POLLERR:
                            _errno = 0;
                            INFO << "Error during poll()" << endl;
                            event = EVENT_ERROR;
                            break;

                        case POLLIN:
                        case POLLPRI: // data is ready for reading
                            if( _connection->getState() == 
                                Connection::STATE_LISTENING )
                                event = EVENT_CONNECT;
                            else
                                event = EVENT_DATA;
                            break;

                    case POLLHUP: // disconnect happened
                        event = EVENT_DISCONNECT;
                        break;

                    case POLLNVAL: // disconnected connection
                        event = EVENT_DISCONNECT;
                        break;
                }
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
    for( size_t i=0; i<_fdSetSize; i++ )
    {
        _fdSet[i].events  = events;
        _fdSet[i].revents = 0;
    }
}

void ConnectionSet::_buildFDSet()
{
    const size_t nConnections = _connections.size();

    if( _fdSetCapacity < nConnections+1 )
    {
        delete [] _fdSet;
        while( _fdSetCapacity < nConnections+1 )
            _fdSetCapacity = _fdSetCapacity << 1;
        _fdSet = new pollfd[_fdSetCapacity];
    }

    size_t    size   = 0;
    const int events = POLLIN; // | POLLPRI;
    _fdSetConnections.clear();

    // add self 'connection'
    _fdSet[size].fd      = _selfFD[0]; // read end of pipe
    _fdSet[size].events  = events;
    _fdSet[size].revents = 0;
    size++;

    // add regular connections
    for( size_t i=0; i<nConnections; i++ )
    {
        eqBase::RefPtr<Connection> connection = _connections[i];
        const int   fd                        = connection->getReadFD();

        if( fd == -1 )
        {
            WARN << "Cannot select connection " << i
                 << ", connection does not use a file descriptor" << endl;
            continue;
        }

        _fdSetConnections[fd] = connection.get();
        _fdSet[size].fd       = fd;
        _fdSet[size].events   = events;
        _fdSet[size].revents  = 0;
        size++;
    }
    _fdSetSize = size;
}   

std::ostream& eqNet::operator << ( std::ostream& os, ConnectionSet* set)
{
    const size_t nConnections = set->nConnections();
    
    os << "connection set " << (void*)set << ", " << nConnections
       << " connections";
    
    for( size_t i=0; i<nConnections; i++ )
    {
        eqBase::RefPtr<Connection> connection = set->getConnection(i);
        os << endl << "    " << connection.get() << ", " 
           << set->getNode( connection );
    }
    
    return os;
}
