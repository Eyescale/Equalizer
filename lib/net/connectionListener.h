
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTIONLISTENER_H
#define EQNET_CONNECTIONLISTENER_H

#include <eq/net/connection.h> // nested Connection::State enum

namespace eqNet
{
    /** A listener interface to connection changes. */
    class EQ_EXPORT ConnectionListener
    {
    public:
        virtual ~ConnectionListener() {}

        virtual void notifyStateChanged( Connection* connection ){}
    };
}

#endif // EQNET_CONNECTION_LISTENER_H
