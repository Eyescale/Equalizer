
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_PRIV_H
#define EQ_PACKETS_PRIV_H

#include "commands.h"
#include "connectionDescription.h"
#include "global.h"
#include "message.h"

#include <sys/param.h>

namespace eqNet
{
    enum
    {
        DATATYPE_EQNET_NODE,
        DATATYPE_EQNET_SESSION,
        DATATYPE_EQNET_USER,
        DATATYPE_CUSTOM = 1<<16
    };

    /**
     * Represents a packet.
     */
    struct Packet
    {
        uint64  size;
        uint    datatype;
        uint    command;
    };

    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
    struct NodePacket: public Packet
    {
        NodePacket(){ datatype = DATATYPE_EQNET_NODE; }
    };

    struct NodeStopPacket : public NodePacket
    {
        NodeStopPacket()
            {
                command = CMD_NODE_STOP;
                size    = sizeof( NodeStopPacket );
            }
    };

    struct NodeMessagePacket : public NodePacket
    {
        NodeMessagePacket()
            { 
                command  = CMD_NODE_MESSAGE;
                size     = sizeof( NodeMessagePacket ); 
            }
        MessageType type;
        uint64      nElements;
    };

    struct NodeMapSessionPacket : public NodePacket
    {
        NodeMapSessionPacket()
            {
                command = CMD_NODE_MAP_SESSION;
                size = sizeof(NodeMapSessionPacket);
            }
        uint requestID;
        uint nameLength;
    };

    struct NodeMapSessionReplyPacket : public NodePacket
    {
        NodeMapSessionReplyPacket( const NodeMapSessionPacket* requestPacket ) 
            {
                command   = CMD_NODE_MAP_SESSION_REPLY;
                size      = sizeof( NodeMapSessionReplyPacket );
                requestID = requestPacket->requestID;
            }
            
        uint requestID;
        uint reply;
    };

    struct NodeSessionPacket : public NodePacket
    {
        NodeSessionPacket() 
            {
                command  = CMD_NODE_SESSION;
                size     = sizeof( NodeSessionPacket ); 
            }

        uint requestID;
        uint sessionID;
    };

    struct NodeConnectPacket : public NodePacket
    {
        NodeConnectPacket() 
            {
                command     = CMD_NODE_CONNECT;
                size        = sizeof( NodeConnectPacket ); 
                wasLaunched = false;
            }

        bool   wasLaunched;
        uint64 launchID;
    };

    //------------------------------------------------------------
    // Session
    //------------------------------------------------------------
    struct SessionPacket : public NodePacket
    {
        SessionPacket( const uint sessionID )
            {
                datatype        = DATATYPE_EQNET_SESSION;
                this->sessionID = sessionID;
            }
        uint sessionID;
    };

    struct SessionNewUserPacket : public SessionPacket
    {
        SessionNewUserPacket( const uint sessionID ) : SessionPacket(sessionID)
            {
                command  = CMD_SESSION_NEW_USER;
                size     = sizeof( SessionNewUserPacket ); 
            }
        uint userID;
    };

    //------------------------------------------------------------
    // User
    //------------------------------------------------------------
    struct UserPacket : public SessionPacket
    {
        UserPacket( const uint sessionID, const uint userID )
                : SessionPacket( sessionID )
            {
                datatype     = DATATYPE_EQNET_USER; 
                this->userID = userID;
            }
        uint userID;
    };


    inline std::ostream& operator << ( std::ostream& os, 
                                       const Packet* packet )
    {
        os << "Packet dt " << packet->datatype << " cmd "<< packet->command;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodePacket* packet )
    {
        os << (Packet*)packet;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeMapSessionPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " l " 
           << packet->nameLength;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionPacket* packet )
    {
        os << (NodePacket*)packet << " ssn " << packet->sessionID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const UserPacket* packet )
    {
        os << (SessionPacket*)packet << " nod " << packet->userID;
        return os;
    }
}

#endif // EQNET_PACKETS_PRIV_H

