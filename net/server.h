
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_H
#define EQNET_SERVER_H

#include "idHash.h"
#include "nodePriv.h"

#include <iostream>

namespace eqNet
{
    namespace priv
    {
        class Connection;
        class PipeConnection;
        class Session;

        std::ostream& operator << ( std::ostream& os, Session* session );

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

            friend inline std::ostream& operator << 
                (std::ostream& os, Server* server);
        };

        inline std::ostream& operator << ( std::ostream& os, Server* server )
        {
            os << "    Server " << server->getID() << "(" << (void*)server
               << "): " << server->_sessions.size() << " session[s]" 
               << std::endl;
            
            for( IDHash<Session*>::iterator iter = server->_sessions.begin();
                 iter != server->_sessions.end(); iter++ )
            {
                Session* session = (*iter).second;
                os << session;
            }

            return os;
        }
    }
}

#endif //EQNET_SERVER_H
