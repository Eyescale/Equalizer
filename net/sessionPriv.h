
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_PRIV_H
#define EQNET_SESSION_PRIV_H

#include "session.h"
#include "basePriv.h"

#include "commands.h"
#include "idHash.h"

#include <iostream>

namespace eqNet
{
    class Node;

    namespace priv
    {
        class Node;
        class NodeList;
        class User;
        struct Packet;
        struct SessionPacket;

        inline std::ostream& operator << ( std::ostream& os, const User* user );

        class Session : public Base, public eqNet::Session
        {
        public:
            /** 
             * Returns the session instance.
             * 
             * @param sessionID the identifier of the session.
             * @return the session.
             */
            static Session* get(const uint sessionID );

            /** @name Managing users */
            //*{
            /**
             * Adds a new user to this session.
             * 
             * @return the user.
             */
            // __eq_generate_distributed__
            User* newUser();

            /**
             * Returns the local node.
             *
             * @return the local node.
             */
            Node* getLocalNode(){ return _localNode; }

            /** 
             * Gets a user using its identifier.
             * 
             * @param userID the user identifier.
             * @return the user.
             */
            User* getUserByID( const uint userID ){ return _users[userID]; }
            //*}

            /** 
             * Creates a new session.
             * 
             * @param id the session id.
             * @param node the node for this session.
             */
            Session( const uint id, Node* node );

            /** 
             * Sets the node identifier of the local node.
             * 
             * @param nodeID the local node identifier.
             */
            void setLocalNode( const uint nodeID );

            /** 
             * Serializes this session and sends the result to the specified
             * nodes.
             * 
             * @param nodes the node list of the receivers.
             */
            void pack( const NodeList& nodes );
            
        private:
            /** The list of users in this session. */
            IDHash<User*> _users;

            /** The next unique user identifier. */
            uint _userID;

            /** The node serving this session. */
            Node* _node;

            /** The local node. */
            Node* _localNode;

            /** The command handler function table. */
            void (eqNet::priv::Session::*_cmdHandler[CMD_SESSION_ALL])( Connection* connection, Node* node, Packet* packet );

            // the command handler functions and helper functions
            void _cmdNewUser( Connection* connection, Node* node, 
                              Packet* packet );

            friend inline std::ostream& operator << 
                (std::ostream& os, Session* session);
        };

        inline std::ostream& operator << ( std::ostream& os, Session* session )
        {
            if( !session )
            {
                os << "NULL session";
                return os;
            }

            os << "    session " << session->getID() << "(" << (void*)session
               << "): " << session->_users.size() << " user[s], " 
               << " local " << session->_localNode;
            
            for( IDHash<User*>::iterator iter = session->_users.begin();
                 iter != session->_users.end(); iter++ )
            {
                User* user = (*iter).second;
                os << std::endl << "    " << user;
            }

            return os;
        }
    }
}
#endif // EQNET_SESSION_PRIV_H

