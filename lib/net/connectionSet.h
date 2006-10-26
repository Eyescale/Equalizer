
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_SET_H
#define EQNET_CONNECTION_SET_H

#include "connection.h"

#include <eq/base/hash.h>
#include <eq/base/refPtr.h>

#include <poll.h>

namespace eqNet
{
    class Connection;
    class Message;
    class Network;
        
    /**
     * A set of connections. 
     *
     * From the set, a connection with pending events can be selected.
     */
    class ConnectionSet
    {
    public:
        enum Event
        {
            EVENT_NONE,            //!< No event has occured
            EVENT_CONNECT,         //!< A new connection
            EVENT_DISCONNECT,      //!< A disconnect
            EVENT_DATA,            //!< Data can be read
            EVENT_TIMEOUT,         //!< The selection request timed out
            EVENT_ERROR            //!< An error occured during selection
        };

        ConnectionSet();
        ~ConnectionSet();

        void addConnection( eqBase::RefPtr<Connection> connection );
        bool removeConnection( eqBase::RefPtr<Connection> connection );
        void clear();
        size_t nConnections() const { return _connections.size(); }
        eqBase::RefPtr<Connection> getConnection( const size_t i ) const
            { return _connections[i]; }

        /** 
         * Selects a Connection which is ready for I/O.
         *
         * Depending on the event, the error number and connection are set.
         * 
         * @param timeout the timeout to wait for an event in milliseconds,
         *                or <code>-1</code> if the call should block
         *                indefinitly.
         * @return The event occured during selection.
         */
        Event select( const int timeout = -1 );

        int                        getErrno()     { return _errno; }
        eqBase::RefPtr<Connection> getConnection(){ return _connection; }

    private:
        std::vector<pollfd> _fdSet;
        bool                _fdSetDirty;
        stde::hash_map<int, Connection*> _fdSetConnections;

        /** The fd to reset a running select, see comment in constructor. */
        int      _selfFD[2];
        /** True if the connection set is in select(). */
        bool     _inSelect;

        int      _errno;

        std::vector< eqBase::RefPtr<Connection> > _connections;

        // result values
        eqBase::RefPtr<Connection> _connection;

        void _dirtyFDSet();
        void _setupFDSet();
        void _buildFDSet();
    };

    /** 
     * Prints the connection set to a std::ostream.
     * 
     * @param os the output stream.
     * @param set the connection set.
     * @return the output stream.
     */
    std::ostream& operator << ( std::ostream& os, ConnectionSet* set );
}

#endif // EQNET_CONNECTION_SET_H
