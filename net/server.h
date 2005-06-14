
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_H
#define EQNET_SERVER_H

namespace eqNet
{
    class Connection;

    /**
     * A Server is the central instance insuring that all identifiers used
     * within a Session are unique.
     *
     * @sa Session
     */
    class Server
    {
    public:
        /** 
         * Runs the server using a single, pre-connected Connection.
         * 
         * @param connection the connection.
         * @return the success value of the run.
         */
        static int run( Connection *connection );

    protected:
        Server(){}
        ~Server(){}
    };
};

#endif //EQNET_SERVER_H
