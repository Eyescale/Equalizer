
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/base/base.h>
#include "base.h"
#include "global.h"

namespace eqNet
{
    namespace priv
    {
        class Session;
    }

    class Node;
    class User;

    /**
     * Manages a session.
     *
     * A session provides unique identifiers for a number of nodes.
     */
    class Session : public Base
    {
    public:
        /** 
         * Constructs a new session.
         * 
         * @param id the identifier of the session.
         * @param node the node hosting the session.
         */
        Session(const uint id, Node* node );

        /**
         * @name Managing users
         */
        //*{
        /**
         * Adds a new user to this session.
         *
         * @param node the node on which the user will reside.
         * @return the user.
         * @sa User, Network::addUser
         */
        User* newUser( Node* node );

        /**
         * Returns the number of users in this session.
         *
         * @returns the number of users in this Session. 
         */
        uint nUsers();

        /**
         * Get the user id of the numbered user in this session.
         *
         * @param index the index of the user.
         * @return the user identifier.
         */
        User* getUser( const uint index );

        /**
         * Removes a user from this session.
         *
         * @param user the user to remove
         * @return <code>true</code> if the user was removed, <code>false</code>
         *         if not.
         * @throws invalid_argument if the user identifier is not known.
         */
        bool deleteUser( User* user );
        //*}

        /**
         * @name Session State Management
         */
        //*{
        /**
         * Initialise this session.
         *
         * Initialising this session initialises all networks in this
         * session. Afterwards, the nodes have to be started before
         * they can communicate with other nodes in this session.
         *
         * @return <code>true</code> if all networks in this session
         *         were successfully initialised, <code>false</code>
         *         if not.
         * @sa Network::init, start
         */
        bool init();

        /**
         * Exits this session.
         *
         * Exiting this session de-initializes all networks in this session.
         *
         * @sa Network::exit, stop
         */
        void exit();

        /**
         * Start all nodes of all initialized networks in this session.
         *
         * @return <code>true</code> if all node in this session were
         *         successfully started , <code>false</code> if not.
         * @sa Network::start, init
         */
        bool start();

        /**
         * Stops all nodes of all initialized networks in this session.
         *
         * @sa Network::stop, exit
         */
        void stop();
        //*}
        
        /** 
         * Returns the identifier of this session.
         * 
         * @return the identifier.
         */
        uint getID() { return _id; }
        
        /** 
         * Handles a command packet.
         * 
         * @param node the node which sent the packet.
         * @param packet the packet.
         */
        void handlePacket( Node* node, const SessionPacket* packet );

    private:

        /** The session identifier. */
        uint _id;

        /** The command handler function table. */
        void (eqNet::priv::Session::*_cmdHandler[CMD_SESSION_ALL])( Node* node, const SessionPacket* packet );

        // the command handler functions and helper functions
        void _cmdNewUser( Node* node, const Packet* packet );

        friend inline std::ostream& operator << (std::ostream& os,
                                                 Session* session);
    };
    std::ostream& operator << ( std::ostream& os, Session* session );
}
#endif // EQNET_SESSION_PRIV_H

