
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
         * @sa Network::select
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

            void addConnection( Connection* connection, Network* network );
            void removeConnection( Connection* connection );
            void clear();
        
            Event select( const int timeout );
        
            Node*    getNode(){    return _node; }
            Network* getNetwork(){ return _network; }
            Message* getMessage(){ return _message; }
            int      getErrno(){   return _errno; }

        private:
            pollfd* _fdSet;
            size_t  _fdSetSize;
            size_t  _fdSetCapacity;
            bool    _fdSetDirty;
            Sgi::hash_map<int, Connection*> _fdSetConnections;
        
            Node*    _node;
            Network* _network;
            Message* _message;
            int      _errno;

            eqBase::PtrHash<Connection*, Network*> _connections;

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
