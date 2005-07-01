
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionSet.h"
#include "connection.h"

#include <eq/base/base.h>
#include <errno.h>

using namespace eqBase;
using namespace eqNet::priv;
using namespace std;

ConnectionSet::ConnectionSet()
        : _fdSetSize(0),
          _fdSetCapacity(64),
          _fdSetDirty(false),
          _node(NULL),
          _network(NULL),
          _message(NULL),
          _errno(0)
{
    _fdSet = new pollfd[_fdSetCapacity];
}

ConnectionSet::~ConnectionSet()
{}

void ConnectionSet::addConnection( Connection* connection, 
    Network* network )
{
    _connections[connection] = network;
    _fdSetDirty = true;
}

void ConnectionSet::removeConnection( Connection* connection )
{
    const size_t nDeleted = _connections.erase( connection );
    _fdSetDirty = true;
    ASSERT( nDeleted==1 );
}

void ConnectionSet::clear()
{
    _connections.clear();
    _fdSetDirty = true;
}
        
ConnectionSet::Event ConnectionSet::select( const int timeout )
{
    _setupFDSet();

    // poll for a result
    const int ret = poll( _fdSet, _fdSetSize, timeout );
    switch( ret )
    {
        case 0:
            return EVENT_TIMEOUT;

        case -1: // ERROR
            _errno = errno;
            INFO << "Error during poll(): " << strerror( _errno ) << endl;
            return EVENT_ERROR;

        default: // SUCCESS
            for( size_t i=0; i<_fdSetSize; i++ )
            {
                if( _fdSet[i].revents == 0 )
                    continue;

                Connection* connection = _fdSetConnections[_fdSet[i].fd];
                _network               = _connections[connection];
                const int   event      = _fdSet[i].revents;

                INFO << "selected connection #" << i << " of " << _fdSetSize
                     << ", event " << event << endl;
                switch( event )
                {
                    case POLLERR:
                        _errno = 0;
                        INFO << "Error during poll()" << endl;
                        return EVENT_ERROR;

                    case POLLIN:
                    case POLLPRI: // data is ready for reading
//                         if( _message )
//                             delete _message;
//                         if( !_network->readMessage( connection, _message, 
//                                 _node ))
//                         {
//                             WARN << "Error during message read" << endl;
//                             _errno = 0; // ???
//                             return EVENT_ERROR;
//                         }
                        return EVENT_MESSAGE;

                    case POLLHUP: // disconnect happened
                        WARN << "Unhandled: Connection disconnect" << endl;
                        return EVENT_NODE_DISCONNECT;

                    case POLLNVAL: // disconnected connection
                        WARN << "Unhandled: Disconnected connection" << endl;
                        return EVENT_NODE_DISCONNECT;
                }
            }
            break;
    }

}
     
void ConnectionSet::_setupFDSet()
{
    if( _fdSetDirty )
    {
        _buildFDSet();
        _fdSetDirty = false;
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

    if( _fdSetCapacity < nConnections )
    {
        delete [] _fdSet;
        _fdSetCapacity = _fdSetCapacity << 1;
        _fdSet = new pollfd[_fdSetCapacity];
    }

    size_t           i      = 0;
    static const int events = POLLIN; // | POLLPRI;
    _fdSetConnections.clear();

    for( PtrHash<Connection*, Network*>::iterator iter = _connections.begin();
         iter != _connections.end(); iter++ )
    {
        Connection* connection = (*iter).first;
        const int   fd         = connection->getReadFD();

        if( fd == -1 )
        {
            WARN << "Cannot select connection " << i
                 << ", connection does not use file descriptor" << endl;
            continue;
        }

        _fdSetConnections[fd] = connection;
        _fdSet[i].fd          = fd;
        _fdSet[i].events      = events;
        _fdSet[i].revents      = 0;
        i++;
    }

    _fdSetSize = i;
}   
