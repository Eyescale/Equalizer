
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

        static Connection* create( ConnectionDescription &description );

        virtual bool connect(){ return false; }
        virtual bool listen(){ return false; }
        virtual Connection* accept(){ return NULL; }

        virtual size_t read( const void* buffer, const size_t bytes ){return 0;}
        virtual size_t write( const void* buffer, const size_t bytes){return 0;}

        virtual void close(){};

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
        Connection();

        virtual int getReadFD() const { return -1; }

        ConnectionDescription _description; //!< The connection parameters
        State                 _state;       //!< The connections state
    };
};

#endif //EQNET_CONNECTION_H
