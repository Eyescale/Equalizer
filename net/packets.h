
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
        DATATYPE_EQNET_OBJECT,
        DATATYPE_EQNET_USER,
        DATATYPE_CUSTOM = 1<<16
    };

    /**
     * Represents a packet.
     */
    struct Packet
    {
        uint64_t size;
        uint32_t datatype;
        uint32_t command;
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
        uint64_t    nElements;
    };

    struct NodeMapSessionPacket : public NodePacket
    {
        NodeMapSessionPacket()
            {
                command   = CMD_NODE_MAP_SESSION;
                size      = sizeof(NodeMapSessionPacket);
                sessionID = INVALID_ID;
            }

        uint32_t requestID;
        uint32_t sessionID;
        uint32_t nameLength;
    };

    struct NodeMapSessionReplyPacket : public NodePacket
    {
        NodeMapSessionReplyPacket( const NodeMapSessionPacket* requestPacket ) 
            {
                command   = CMD_NODE_MAP_SESSION_REPLY;
                size      = sizeof( NodeMapSessionReplyPacket );
                requestID = requestPacket->requestID;
            }
            
        uint32_t requestID;
        uint32_t sessionID;
        uint32_t nameLength;
    };

    struct NodeConnectPacket : public NodePacket
    {
        NodeConnectPacket() 
            {
                command     = CMD_NODE_CONNECT;
                size        = sizeof( NodeConnectPacket ); 
                wasLaunched = false;
            }

        bool     wasLaunched;
        uint64_t launchID;
    };

    //------------------------------------------------------------
    // Session
    //------------------------------------------------------------
    struct SessionPacket : public NodePacket
    {
        SessionPacket( const uint32_t sessionID )
            {
                datatype        = DATATYPE_EQNET_SESSION;
                this->sessionID = sessionID;
            }
        uint32_t sessionID;
    };

    struct SessionGenIDsPacket : public SessionPacket
    {
        SessionGenIDsPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_GEN_IDS;
                size    = sizeof( SessionGenIDsPacket ); 
            }

        uint32_t requestID;
        uint32_t range;
    };

    struct SessionGenIDsReplyPacket : public SessionPacket
    {
        SessionGenIDsReplyPacket( SessionGenIDsPacket* request )
                : SessionPacket(request->sessionID)
            {
                command   = CMD_SESSION_GEN_IDS_REPLY;
                size      = sizeof( SessionGenIDsReplyPacket ); 
                requestID = request->requestID;
            }

        uint32_t requestID;
        uint32_t id;
    };

    struct SessionSetIDMasterPacket : public SessionPacket
    {
        SessionSetIDMasterPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_SET_ID_MASTER;
                size    = sizeof( SessionSetIDMasterPacket ); 
            }

        uint32_t start;
        uint32_t range;
        uint32_t connectionDescriptionLength;
    };

    struct SessionGetIDMasterPacket : public SessionPacket
    {
        SessionGetIDMasterPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_GET_ID_MASTER;
                size    = sizeof( SessionGetIDMasterPacket ); 
            }

        uint32_t requestID;
        uint32_t id;
    };

    struct SessionGetIDMasterReplyPacket : public SessionPacket
    {
        SessionGetIDMasterReplyPacket( SessionGetIDMasterPacket* request ) 
                : SessionPacket( request->sessionID )
            {
                command   = CMD_SESSION_GET_ID_MASTER_REPLY;
                size      = sizeof( SessionGetIDMasterPacket ); 
                requestID = request->requestID;
            }

        uint32_t requestID;
        uint32_t start;
        uint32_t end;
        uint32_t connectionDescriptionLength;
    };

    //------------------------------------------------------------
    // Object
    //------------------------------------------------------------
    struct ObjectPacket : public SessionPacket
    {
        ObjectPacket( const uint32_t sessionID, const uint32_t objectID )
                : SessionPacket( sessionID )
            {
                datatype       = DATATYPE_EQNET_OBJECT; 
                this->objectID = objectID;
                ASSERT( objectID != INVALID_ID );
                ASSERT( objectID != 0 );
            }
        uint32_t objectID;
    };

    //------------------------------------------------------------
    // User
    //------------------------------------------------------------
    struct UserPacket : public SessionPacket
    {
        UserPacket( const uint32_t sessionID, const uint32_t userID )
                : SessionPacket( sessionID )
            {
                datatype     = DATATYPE_EQNET_USER; 
                this->userID = userID;
            }
        uint32_t userID;
    };


    inline std::ostream& operator << ( std::ostream& os, 
                                       const Packet* packet )
    {
        os << "packet dt " << packet->datatype << " cmd "<< packet->command;
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
        os << (NodePacket*)packet << " req " << packet->requestID
           << " sessionID " << packet->sessionID << " l " << packet->nameLength;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeMapSessionReplyPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID
           << " sessionID " << packet->sessionID << " l " << packet->nameLength;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeConnectPacket* packet )
    {
        os << (NodePacket*)packet << " wasLaunched " << packet->wasLaunched 
           << " id " << packet->launchID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionPacket* packet )
    {
        os << (NodePacket*)packet << " session id " << packet->sessionID;
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

