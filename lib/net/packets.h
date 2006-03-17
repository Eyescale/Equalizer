
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_PRIV_H
#define EQ_PACKETS_PRIV_H

#include "commands.h"
#include "connectionDescription.h"
#include "global.h"
#include "message.h"
#include "nodeID.h"

#include <sys/param.h>

namespace eqNet
{
    enum
    {
        DATATYPE_EQNET_NODE,
        DATATYPE_EQNET_SESSION,
        DATATYPE_EQNET_OBJECT,
        DATATYPE_EQNET_MOBJECT,
        DATATYPE_EQNET_VERSIONED_OBJECT,
        DATATYPE_EQNET_USER,
        DATATYPE_CUSTOM = 1<<10
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

    // String transmission: the packets define a 8-char string at the end of the
    // packet. When the packet is sent using Node::send( Packet&, string& ), the
    // whole string is appended to the packet, so that the receiver has to do
    // nothing special to receive and use the full packet.

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
                sessionID = EQ_INVALID_ID;
                name[0]   = '\0';
            }

        uint32_t requestID;
        uint32_t sessionID;
        char     name[8];
    };

    struct NodeMapSessionReplyPacket : public NodePacket
    {
        NodeMapSessionReplyPacket( const NodeMapSessionPacket* requestPacket ) 
            {
                command   = CMD_NODE_MAP_SESSION_REPLY;
                size      = sizeof( NodeMapSessionReplyPacket );
                requestID = requestPacket->requestID;
                name[0]   = '\0';
            }
            
        uint32_t requestID;
        uint32_t sessionID;
        char     name[8];
    };

    struct NodeUnmapSessionPacket : public NodePacket
    {
        NodeUnmapSessionPacket()
            {
                command   = CMD_NODE_UNMAP_SESSION;
                size      = sizeof(NodeUnmapSessionPacket);
                sessionID = EQ_INVALID_ID;
            }

        uint32_t requestID;
        uint32_t sessionID;
    };

    struct NodeUnmapSessionReplyPacket : public NodePacket
    {
        NodeUnmapSessionReplyPacket(const NodeUnmapSessionPacket* requestPacket)
            {
                command   = CMD_NODE_UNMAP_SESSION_REPLY;
                size      = sizeof( NodeUnmapSessionReplyPacket );
                requestID = requestPacket->requestID;
            }
            
        uint32_t requestID;
        bool     result;
    };

    struct NodeConnectPacket : public NodePacket
    {
        NodeConnectPacket() 
            {
                command                  = CMD_NODE_CONNECT;
                size                     = sizeof( NodeConnectPacket ); 
                wasLaunched              = false;
                connectionDescription[0] = '\0';
            }

        bool     wasLaunched;
        uint64_t launchID;
        NodeID   nodeID;
        char     connectionDescription[8];
    };

    struct NodeGetConnectionDescriptionPacket : public NodePacket
    {
        NodeGetConnectionDescriptionPacket()
            {
                command = CMD_NODE_GET_CONNECTION_DESCRIPTION;
                size    = sizeof( NodeGetConnectionDescriptionPacket );
            }

        NodeID   nodeID;
        uint32_t index;
    };

    struct NodeGetConnectionDescriptionReplyPacket : public NodePacket
    {
        NodeGetConnectionDescriptionReplyPacket()
            {
                command = CMD_NODE_GET_CONNECTION_DESCRIPTION_REPLY;
                size    = sizeof( NodeGetConnectionDescriptionReplyPacket );
                connectionDescription[0] = '\0';
            }

        NodeID   nodeID;
        uint32_t nextIndex;
        char     connectionDescription[8];
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
        NodeID   masterID;
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
        NodeID   masterID;
    };

    struct SessionGetMobjectMasterPacket : public SessionPacket
    {
        SessionGetMobjectMasterPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_GET_MOBJECT_MASTER;
                size    = sizeof( SessionGetMobjectMasterPacket ); 
            }

        uint32_t mobjectID;
    };

    struct SessionGetMobjectMasterReplyPacket : public SessionPacket
    {
        SessionGetMobjectMasterReplyPacket( const SessionGetMobjectMasterPacket*
                                            request ) 
                : SessionPacket( request->sessionID )
            {
                command   = CMD_SESSION_GET_MOBJECT_MASTER_REPLY;
                size      = sizeof( SessionGetMobjectMasterReplyPacket );
                mobjectID = request->mobjectID;
            }

        uint32_t mobjectID;
        uint32_t start;
        uint32_t end;
        NodeID   masterID;
    };

    struct SessionGetMobjectPacket : public SessionPacket
    {
        SessionGetMobjectPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_GET_MOBJECT;
                size    = sizeof( SessionGetMobjectPacket ); 
            }
        
        uint32_t requestID;
        uint32_t mobjectID;
    };

    struct SessionInitMobjectPacket : public SessionPacket
    {
        SessionInitMobjectPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_INIT_MOBJECT;
                size    = sizeof( SessionInitMobjectPacket ); 
            }

        uint32_t mobjectID;
    };

    struct SessionInstanciateMobjectPacket : public SessionPacket
    {
        SessionInstanciateMobjectPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command        = CMD_SESSION_INSTANCIATE_MOBJECT;
                size           = sizeof( SessionInstanciateMobjectPacket ); 
                isMaster       = false;
                mobjectData[0] = '\0';
            }

        bool     isMaster;
        uint32_t mobjectID;
        uint32_t mobjectType;
        uint64_t mobjectDataSize;
        char     mobjectData[8];
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
                EQASSERT( objectID != EQ_INVALID_ID );
                EQASSERT( objectID != 0 );
            }
        uint32_t objectID;
    };

    //------------------------------------------------------------
    // Mobject
    //------------------------------------------------------------
    struct MobjectPacket : public ObjectPacket
    {
        MobjectPacket( const uint32_t sessionID, const uint32_t objectID )
                : ObjectPacket( sessionID, objectID )
            {
                datatype       = DATATYPE_EQNET_MOBJECT; 
            }
    };

    //------------------------------------------------------------
    // versioned object
    //------------------------------------------------------------
    struct VersionedObjectPacket : public MobjectPacket
    {
        VersionedObjectPacket( const uint32_t sessionID, 
                               const uint32_t objectID )
                : MobjectPacket( sessionID, objectID )
            {
                datatype       = DATATYPE_EQNET_VERSIONED_OBJECT; 
            }
    };

    struct VersionedObjectSyncPacket : public VersionedObjectPacket
    {
        VersionedObjectSyncPacket( const uint32_t sessionID, 
                                   const uint32_t objectID )
                : VersionedObjectPacket( sessionID, objectID )
            {
                command        = CMD_VERSIONED_OBJECT_SYNC;
                size           = sizeof( VersionedObjectSyncPacket ); 
                delta[0]       = '\0';
            }
        
        uint32_t version;
        uint64_t deltaSize;
        char     delta[8];
    };

    //------------------------------------------------------------
    // Barrier
    //------------------------------------------------------------
    struct BarrierEnterPacket : public MobjectPacket
    {
        BarrierEnterPacket( const uint32_t sessionID, const uint32_t objectID )
                : eqNet::MobjectPacket( sessionID, objectID )
            {
                command = CMD_BARRIER_ENTER;
                size    = sizeof( BarrierEnterPacket );
            }
    };

    struct BarrierEnterReplyPacket : public MobjectPacket
    {
        BarrierEnterReplyPacket( const uint32_t sessionID, 
                                 const uint32_t objectID )
                : eqNet::MobjectPacket( sessionID, objectID )
            {
                command = CMD_BARRIER_ENTER_REPLY;
                size    = sizeof( BarrierEnterReplyPacket );
            }
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
           << " sessionID " << packet->sessionID << " name " << packet->name;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeMapSessionReplyPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID
           << " sessionID " << packet->sessionID << " name " << packet->name;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeConnectPacket* packet )
    {
        os << (NodePacket*)packet << " wasLaunched " << packet->wasLaunched 
           << " id " << packet->launchID << " cd " 
           << packet->connectionDescription;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                              const NodeGetConnectionDescriptionPacket* packet )
    {
        os << (NodePacket*)packet << " nodeID " << packet->nodeID << " i "  
           << packet->index;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                         const NodeGetConnectionDescriptionReplyPacket* packet )
    {
        os << (NodePacket*)packet << " nodeID " << packet->nodeID << " ni "  
           << packet->nextIndex << " desc " << packet->connectionDescription;
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
        os << (SessionPacket*)packet << " user id " << packet->userID;
        return os;
    }


    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionGenIDsReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " id start " << packet->id;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                             const SessionGetMobjectMasterReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->mobjectID
           << " master " << packet->masterID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionGetMobjectPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->mobjectID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                 const SessionInstanciateMobjectPacket* packet )
    {
        os << (SessionPacket*)packet << " mobj id " << packet->mobjectID <<
            " type " << packet->mobjectType << " master " << packet->isMaster;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " obj id " << packet->objectID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const VersionedObjectSyncPacket* packet )
    {
        os << (VersionedObjectPacket*)packet << " version " << packet->version
           << " size " << packet->deltaSize;
        return os;
    }

}

#endif // EQNET_PACKETS_PRIV_H

