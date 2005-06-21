
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_H
#define EQNET_SERVER_H

#include <eq/net/node.h>

#include <vector>

namespace eqNet
{
    class Node;

    namespace internal
    {
        class Connection;

        /**
         * A Server is the central instance running multiple sessions.
         *
         * @sa Session
         */
        class Server : public Node
        {
        public:
            /** 
             * Runs the server using a single, pre-connected Connection.
             * 
             * @param connection the connection.
             * @return the success value of the run.
             */
            static int run( Connection* connection );

            /** 
             * Connects with an existing server and returns the local proxy.
             * 
             * @param connection the connection.
             * @return the server.
             */
            static Server* connect( Connection* connection );

        protected:
            Server( Connection* connection );
            ~Server(){}


            std::vector<Connection*> _connections;

            void _init();
            int  _run();

            void _handleRequest( Connection *connection );
        };
    }
}

#endif //EQNET_SERVER_H
