
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_H
#define EQNET_SERVER_H

#include <vector>

namespace eqNet
{
    class Connection;
    class Node;

    /**
     * A Server is the central instance running multiple sessions.
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
        static int run( Connection* connection );


    protected:
        Server(){}
        Server( Connection* connection );
        ~Server(){}

        std::vector<Connection*> _connections;
        std::vector<Node*>       _nodes;

        void _init();
        int  _run();

        void _handleRequest( Connection *connection );
    };
};

#endif //EQNET_SERVER_H
