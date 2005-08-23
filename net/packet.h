
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKET_PRIV_H
#define EQNET_PACKET_PRIV_H

#include "commands.h"
#include "connectionDescription.h"
#include "global.h"
#include "message.h"

#include <sys/param.h>

namespace eqNet
{
    enum
    {
        DATATYPE_EQ_NODE,
        DATATYPE_EQ_SESSION,
        DATATYPE_EQ_USER,
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
        NodePacket(){ datatype = DATATYPE_EQ_NODE; }
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

    struct NodeCreateSessionReplyPacket : public NodePacket
    {
        NodeCreateSessionReplyPacket() 
            {
                command  = CMD_NODE_CREATE_SESSION_REPLY;
                size     = sizeof( NodeCreateSessionReplyPacket ); 
            }
            
        uint requestID;
        uint reply;
    };

    struct NodeNewSessionPacket : public NodePacket
    {
        NodeNewSessionPacket() 
            {
                command  = CMD_NODE_NEW_SESSION;
                size     = sizeof( NodeNewSessionPacket ); 
            }

        uint sessionID;
    };

    //------------------------------------------------------------
    // Session
    //------------------------------------------------------------
    struct SessionPacket : public NodePacket
    {
        SessionPacket(){ datatype = DATATYPE_EQ_SESSION; }
        uint sessionID;
    };

    struct SessionNewUserPacket : public SessionPacket
    {
        SessionNewUserPacket()
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
        UserPacket(){ datatype = DATATYPE_EQ_USER; }
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

#endif // EQNET_PACKET_PRIV_H

