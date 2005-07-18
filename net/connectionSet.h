
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_SET_H
#define EQNET_CONNECTION_SET_H

#include <eq/base/hash.h>

#include <poll.h>

namespace eqNet
{
    namespace priv
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
                EVENT_NODE_CONNECT,    //!< A known node connected
                EVENT_NEW_CONNECTION,  //!< A connection from an unknow source
                EVENT_NODE_DISCONNECT, //!< A node disconnected
                EVENT_MESSAGE,         //!< A new message is waiting
                EVENT_TIMEOUT,         //!< The selection request timed out
                EVENT_ERROR            //!< An error occured during selection
            };

            ConnectionSet();
            ~ConnectionSet();

            void        addConnection( Connection* connection, 
                                       Network* network, Node* node );
            void        removeConnection( Connection* connection );
            Connection* getConnection( Node* node )
                { return _connections[node]; }
            void clear();

            eqBase::PtrHash<Node*, Connection*>::iterator begin()
                { return _connections.begin(); }
            eqBase::PtrHash<Node*, Connection*>::iterator end()
                { return _connections.end(); }

            size_t size() const { return _connections.size(); }
        
            Event select( const int timeout );
        
            int      getErrno(){   return _errno; }

        private:
            pollfd* _fdSet;
            size_t  _fdSetSize;
            size_t  _fdSetCapacity;
            bool    _fdSetDirty;
            Sgi::hash_map<int, Connection*> _fdSetConnections;
        
            int      _errno;

            eqBase::PtrHash<Node*, Connection*>    _connections;
            eqBase::PtrHash<Connection*, Node*>    _nodes;
            eqBase::PtrHash<Connection*, Network*> _networks;

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
}

#endif // EQNET_CONNECTION_SET_H
