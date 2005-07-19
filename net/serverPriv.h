
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_PRIV_H
#define EQNET_SERVER_PRIV_H

#include "server.h"
#include "basePriv.h"

#include "commands.h"
#include "idHash.h"

#include <iostream>

namespace eqNet
{
    namespace priv
    {
        class Connection;
        class Node;
        class Packet;
        class PipeConnection;
        class Session;

        std::ostream& operator << ( std::ostream& os, Session* session );

        /**
         * A Server is the central instance running multiple sessions.
         *
         * @sa Session
         */
        class Server : public eqNet::Server, public Base
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
            static int run( const char* hostname, const ushort port );

            /** 
             * Connects to a server and creates a new session.
             * 
             * @param address the server address.
             * @return the server.
             */
            static Session* createSession( const char* address );

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


            /** 
             * Entry function for the server-side session thread.
             * 
             * @param session the session.
             * @return the success value.
             */
            ssize_t runSession( Session* session );
        protected:
            Server();
            ~Server(){}
            
            /** The next unique session identifier. */
            uint64 _sessionID;
            /** The sessions on this server. */
            IDHash<Session*> _sessions;

            bool start( PipeConnection* connection );
            bool start( const char* hostname, const ushort port );

            bool _connect( const char* address );
            bool _connectRemote( const char* hostname, const ushort port );
            bool _connectLocal();

            int  _run();

            void _handleRequest( Connection *connection );

        private:
            State       _state;
            Connection* _connection;

            /** The command handler function table. */
            void (eqNet::priv::Server::*_cmdHandler[CMD_SERVER_ALL])(Connection* connection, const Packet* packet );

            // the command handler functions and helper functions
            void _handlePacket( Connection* connection, const Packet* packet );
            void _handleUnknown( Connection* connection, const Packet* packet );
            void _handleSessionCreate( Connection* connection, 
                                       const Packet* packet );
            Session* _createSession( const char* remoteAddress, 
                                     Connection* connection, Node** remoteNode);
            bool     _startSessionThread( Session* session );

            void _handleSessionCreated( Connection* connection, 
                                        const Packet* packet );
            void _handleSessionNew( Connection* connection, 
                                    const Packet* packet );

            Session* _createSession( const char* address );
            void _sendSessionCreate();

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
