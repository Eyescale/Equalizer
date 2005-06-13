
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_H
#define EQNET_CONNECTION_H

#include <eq/base/base.h>
#include <eq/net/connectionDescription.h>

#include <stdexcept>
#include <netinet/in.h>

namespace eqNet
{
    class ConnectionDescription;

    enum State {
        STATE_CLOSED,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_LISTENING
    };

    #define DEFAULT_PORT 4242

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
        virtual ~Connection(){}

        static Connection* create( ConnectionDescription &description );

        virtual bool connect(){ return false; }
        virtual bool listen(){ return false; }
        virtual Connection* accept(){ return NULL; }

        virtual size_t read( const void* buffer, const size_t bytes ){return 0;}
        virtual size_t write( const void* buffer, const size_t bytes){return 0;}

        virtual void close(){};

    protected:
        Connection();

        ConnectionDescription _description; //!< The connection parameters
        State                 _state;       //!< The connections state
    };
};

#endif //EQNET_CONNECTION_H
