
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_SET_H
#define EQNET_CONNECTION_SET_H

#include "network.h"

#include <strings.h>

namespace eqNet
{
    /**
     * A set of connections.
     *
     * @sa Network::select
     */
    class ConnectionSet
    {
    public:
        ConnectionSet();
        ~ConnectionSet();

        void addConnection( const Network* network, Connection* connection );
        void removeConnection( Connection* connection );
        void clear();
        
        Event select( const int timeout );
        
        enum Event
        {
            EVENT_NODE_CONNECT,
            EVENT_NEW_CONNECTION,
            EVENT_NODE_DISCONNECT,
            EVENT_MESSAGE
        };

        Node*    getNode();
        Network* getNetwork();
        Message* getMessage();

    private:
        pollfd* _fdSet;
        size_t  _fdSetSize;
        
        
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
};

#endif // EQNET_CONNECTION_SET_H
