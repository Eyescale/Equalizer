
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_PRIV_H
#define EQNET_SERVER_PRIV_H

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

            enum State {
                STATE_STOPPED,
                STATE_STARTED,
                STATE_RUNNING
            };

            /** 
             * Runs the server using a single, pre-connected Connection.
             * 
             * @param connection the connection.
             * @return the success value of the run.
             */
            static int run( PipeConnection* connection );

            /** 
             * Runs the standalone server on the specified address.
             * 
             * @param address the address, if <code>NULL</code> the server will
             *                listen on all addresses of the machine using the
             *                default port.
             * @return the success value of the run.
             */
            static int run( const char* address );

            /** 
             * Connects with an existing server and returns the local proxy.
             * 
             * @param address the server address.
             * @return the server.
             */
            static Server* connect( const char* address );

            /** 
             * Get a numbered session.
             * 
             * @param index the index of the session.
             * @return the session, or <code>NULL</code> if the index is out of
             *         range.
             */
            Session* getSession( const uint index );

            /** 
             * Returns the state of the server.
             * 
             * @return the server state.
             */
            State getState(){ return _state; }

        protected:
            Server();
            ~Server(){}
            
            /** The next unique session identifier. */
            uint _sessionID;
            /** The sessions on this server. */
            IDHash<Session*> _sessions;

            bool start( PipeConnection* connection );
            bool start( const char* address );

            bool _connect( const char* address );

            int  _run();

            void _handleRequest( Connection *connection );

        private:
            State _state;

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

#endif //EQNET_SERVER_PRIV_H
