
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_H
#define EQNET_CONNECTION_H

#include <eq/base/base.h>
#include "connectionDescription.h"

#include <poll.h>
#include <stdexcept>
#include <vector>

namespace eqNet
{
#   define DEFAULT_PORT 4242

    /**
     * The connection error exception, thrown by the Connection methods.
     */
    struct connection_error : public std::runtime_error
    {
        connection_error(const std::string& msg) : std::runtime_error(msg){}
    };

    /**
     * A base class to provide communication to other hosts.
     */
    class Connection
    {
    public:

        enum State {
            STATE_CLOSED,
            STATE_CONNECTING,
            STATE_CONNECTED,
            STATE_LISTENING
        };

        virtual ~Connection(){}

        /** 
         * Creates a new connection.
         *
         * This factory method creates a new concrete connection as described by
         * the connection description. The concrete connection may not support
         * all functionality of the Connection interface.
         * 
         * @param description The description of the connection.
         * @return the connection.
         */
        static Connection* create( ConnectionDescription &description );

        /** @name Connection Management */
        //@{
        /** 
         * Connect the connection, as described in the ConnectionDescription.
         *
         * @return <code>true</code> if the connection was successfully
         *         connected, <code>false</code> if not.
         */
        virtual bool connect(){ return false; }

        /** 
         * Put the connection into the listening state for a new incoming
         * connection.
         *
         * @return <code>true</code> if the connection is listening for new
         *         incoming connections, <code>false</code> if not.
         */
        virtual bool listen(){ return false; }

        /** 
         * Accepts the next incoming connection.
         * 
         * @return the accepted connection, or <code>NULL</code> if no
         *         connection was accepted.
         */
        virtual Connection* accept(){ return NULL; }

        virtual void close(){};
        //@}

        /** @name Messaging API */
        //@{
        /** 
         * Reads a message from the connection.
         * 
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to read.
         * @return the number of bytes received.
         */
        virtual size_t recv( const void* buffer, const size_t bytes ){return 0;}
        virtual size_t send( const void* buffer, const size_t bytes){return 0;}
        //@}

        State getState(){ return _state; }

        /** 
         * Polls a set of connections for an incoming event
         * 
         * @param connections the array of pointers to the connections.
         * @param nConnections the size of the Connection array.
         * @param timeout the amount of time to wait in milliseconds, if set to
         *                <code>-1</code> the method blocks indefinitely. 
         * @param event used to return the type of event which occured, see man
         *              poll.
         * @return the Connection which produced the event, or <code>NULL</code>
         *         if the functioned timed out or could not select a connection.
         */
        static Connection* select( const std::vector<Connection*> &connections,
            const int timeout, short &event );

    protected:
        Connection(ConnectionDescription &description);

        virtual int getReadFD() const { return -1; }

        ConnectionDescription _description; //!< The connection parameters
        State                 _state;       //!< The connections state
    };
};

#endif //EQNET_CONNECTION_H
