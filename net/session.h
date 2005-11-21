
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/base/base.h>
#include <eq/base/requestHandler.h>

#include "base.h"
#include "commands.h"
#include "global.h"
#include "idHash.h"
#include "node.h"

namespace eqNet
{
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
         */
        Session();

        /** 
         * Returns the name of the session.
         * 
         * @return the name of the session.
         */
        const std::string& getName() const { return _name; }

        /** 
         * Returns the realization state of the session.
         * 
         * @return the realization state of the session.
         */
        bool isRealized();

        /**
         * Realizes the session.
         *
         * The constructor for an existing, but unrealized session will block
         * until the session has been realized.
         *
         * @return <code>true</code> if the session was realized,
         *         <code>false</code> if the session was already realized.
         */
        bool realize();

        /**
         * @name Managing users
         */
        //*{
        /**
         * Creates a new user in this session.
         *
         * @return the user.
         * @sa User
         */
        // __eq_generate_distributed__
        User* createUser();

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
        uint getID() const { return _id; }
        
        /** 
         * Handles a command packet.
         * 
         * @param node the node which sent the packet.
         * @param packet the packet.
         */
        void handlePacket( Node* node, const SessionPacket* packet );

        /** 
         * Serializes this session and sends it to a node.
         * 
         * @param node the receiving node.
         */
        void pack( Node* node ) const;

        /** 
         * Sets the mapping information of this session, used internally by
         * Node::mapSession().
         * 
         * @param server the node hosting the session.
         * @param id the session's identifier.
         * @param name the name of the session.
         */
        void map( Node* server, const uint id, const std::string& name );

    protected:
        /** Registers requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** 
         * Sends a packet to the session's node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool send( const Packet& packet ) { return _node->send( packet ); }

        /** The session's identifier. */
        uint _id;
        
    private:
        /** The node hosting the session. */
        Node* _node;

        /** The session's name. */
        std::string _name;

        /** The unique user identifier counter. */
        uint _userID;

        /** The current users of this session. */
        IDHash<User*> _users;

        /** The command handler function table. */
        void (eqNet::Session::*_cmdHandler[CMD_SESSION_CUSTOM])( Node* node, const Packet* packet );

        // the command handler functions and helper functions
        void _cmdCreateUser( Node* node, const Packet* packet );

        friend std::ostream& operator << ( std::ostream& os, Session* session );
    };
    std::ostream& operator << ( std::ostream& os, Session* session );
}
#endif // EQNET_SESSION_PRIV_H

