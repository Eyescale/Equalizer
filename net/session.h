
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/base/base.h>
#include <eq/base/idPool.h>
#include <eq/base/requestHandler.h>

#include "base.h"
#include "commands.h"
#include "global.h"
#include "idHash.h"
#include "node.h"

namespace eqNet
{
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
         * Sets the mapping information of this session, used internally by
         * Node::mapSession().
         * 
         * @param server the node hosting the session.
         * @param id the session's identifier.
         * @param name the name of the session.
         */
        void map( Node* server, const uint id, const std::string& name,
                  const bool isMaster );

        /**
         * @name Operations
         */
        //*{
        uint genIDs( const uint range );
        void freeIDs( const uint start, const uint range );
        //*}
        
    protected:
        /** Registers requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** 
         * Sends a packet to the session's node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool send( const Packet& packet ) { return _server->send( packet ); }

        /** The session's identifier. */
        uint _id;
        
    private:
        /** The node hosting the session. */
        eqBase::RefPtr<Node> _server;

        /** The session's name. */
        std::string _name;

        /** The state (master/client) of this session instance. */
        bool _isMaster;

        /** The identifier pool. */
        eqBase::IDPool _idPool;

        /** The command handler function table. */
        void (eqNet::Session::*_cmdHandler[CMD_SESSION_CUSTOM])( Node* node, const Packet* packet );

        void _cmdGenIDs( Node* node, const Packet* packet );
        void _cmdGenIDsReply( Node* node, const Packet* packet );

    };
    std::ostream& operator << ( std::ostream& os, Session* session );
}
#endif // EQNET_SESSION_PRIV_H

