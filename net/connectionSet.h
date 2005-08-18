
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_SET_H
#define EQNET_CONNECTION_SET_H

#include <eq/base/hash.h>

#include <poll.h>

namespace eqNet
{
    class Connection;
    class Message;
    class Network;
    class Node;
        
    /**
     * A set of connections. 
     *
     * The set associates a Network and a Node with each Connection. From
     * this set, a connection with pending events can be selected.
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

        void addConnection( Connection* connection, Node* node );
        void removeConnection( Connection* connection );
        void clear();

        /** 
         * Selects a Connection which is ready for I/O.
         *
         * Depending on the event, the error number, connection and node are
         * set by this method.
         * 
         * @param timeout the timeout to wait for an event in milliseconds,
         *                or <code>-1</code> if the call should block
         *                indefinitly.
         * @return The event occured during selection.
         */
        Event select( const int timeout = -1 );

        int         getErrno()     { return _errno; }
        Connection* getConnection(){ return _connection; }
        Node*       getNode()      { return _node; }

    private:
        pollfd* _fdSet;
        size_t  _fdSetSize;
        size_t  _fdSetCapacity;
        bool    _fdSetDirty;
        Sgi::hash_map<int, Connection*> _fdSetConnections;

        /** The fd to reset a running select, see comment in constructor. */
        int      _selfFD[2];
        /** True if the connection set is in select(). */
        bool     _inSelect;

        int      _errno;

        std::vector<Connection*>            _connections;
        eqBase::PtrHash<Connection*, Node*> _nodes;

        // result values
        Connection* _connection;
        Node*       _node;

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
    inline std::ostream& operator << ( std::ostream& os, 
                                       ConnectionSet* set)
    {
        // TODO
        return os;
    }
}

#endif // EQNET_CONNECTION_SET_H
