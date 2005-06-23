
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_H
#define EQNET_SERVER_H

#include "nodePriv.h"

#include <vector>

namespace eqNet
{
    namespace priv
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
            static int run( PipeConnection* connection );

            /** 
             * Connects with an existing server and returns the local proxy.
             * 
             * @param connection the connection.
             * @return the server.
             */
            static Server* connect( Connection* connection );

        protected:
            Server( PipeConnection* connection );
            ~Server(){}
            
            /** The next unique session identifier. */
            uint _sessionID;
            /** The sessions on this server. */
            IDHash<Session*> _sessions;

            void _init();
            int  _run();

            void _handleRequest( Connection *connection );
        };
    }
}

#endif //EQNET_SERVER_H
