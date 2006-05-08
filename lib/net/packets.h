
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_PRIV_H
#define EQ_PACKETS_PRIV_H

#include "base.h"
#include "commands.h"
#include "global.h"
#include "message.h"
#include "nodeID.h"

#include <sys/param.h>

#define EQ_ALIGN8  __attribute__ ((aligned (8)))

namespace eqNet
{
    enum
    {
        DATATYPE_EQNET_NODE,
        DATATYPE_EQNET_SESSION,
        DATATYPE_EQNET_OBJECT,
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
        char     name[8] EQ_ALIGN8;
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
        char     name[8] EQ_ALIGN8;
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
        char     connectionDescription[8] EQ_ALIGN8;
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
        char     connectionDescription[8] EQ_ALIGN8;
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
                size      = sizeof( SessionGetIDMasterReplyPacket );
                requestID = request->requestID;
            }

        uint32_t requestID;
        uint32_t start;
        uint32_t end;
        NodeID   masterID;
    };

    struct SessionGetObjectMasterPacket : public SessionPacket
    {
        SessionGetObjectMasterPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_GET_OBJECT_MASTER;
                size    = sizeof( SessionGetObjectMasterPacket ); 
            }

        uint32_t objectID;
    };

    struct SessionGetObjectMasterReplyPacket : public SessionPacket
    {
        SessionGetObjectMasterReplyPacket( const SessionGetObjectMasterPacket*
                                           request ) 
                : SessionPacket( request->sessionID )
            {
                command  = CMD_SESSION_GET_OBJECT_MASTER_REPLY;
                size     = sizeof( SessionGetObjectMasterReplyPacket );
                objectID = request->objectID;
            }

        uint32_t objectID;
        uint32_t start;
        uint32_t end;
        NodeID   masterID;
    };

    struct SessionGetObjectPacket : public SessionPacket
    {
        SessionGetObjectPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_GET_OBJECT;
                size    = sizeof( SessionGetObjectPacket ); 
                pending = false;
            }
        
        uint32_t requestID;
        bool     pending;
    };

    struct SessionInitObjectPacket : public SessionPacket
    {
        SessionInitObjectPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command = CMD_SESSION_INIT_OBJECT;
                size    = sizeof( SessionInitObjectPacket ); 
            }

        uint32_t objectID;
    };

    struct SessionInstanciateObjectPacket : public SessionPacket
    {
        SessionInstanciateObjectPacket( const uint32_t sessionID ) 
                : SessionPacket( sessionID )
            {
                command        = CMD_SESSION_INSTANCIATE_OBJECT;
                size           = sizeof( SessionInstanciateObjectPacket ); 
                objectData[0] = '\0';
            }

        bool     isMaster;
        uint32_t objectID;
        uint32_t objectType;
        uint64_t objectDataSize;
        char     objectData[8] EQ_ALIGN8;
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

    struct ObjectSyncPacket : public ObjectPacket
    {
        ObjectSyncPacket( const uint32_t sessionID, 
                                   const uint32_t objectID )
                : ObjectPacket( sessionID, objectID )
            {
                command        = CMD_OBJECT_SYNC;
                size           = sizeof( ObjectSyncPacket ); 
                delta[0]       = '\0';
            }
        
        uint32_t version;
        uint64_t deltaSize;
        char     delta[8] EQ_ALIGN8;
    };

    //------------------------------------------------------------
    // Barrier
    //------------------------------------------------------------
    struct BarrierEnterPacket : public ObjectPacket
    {
        BarrierEnterPacket( const uint32_t sessionID, const uint32_t objectID )
                : eqNet::ObjectPacket( sessionID, objectID )
            {
                command = CMD_BARRIER_ENTER;
                size    = sizeof( BarrierEnterPacket );
            }
    };

    struct BarrierEnterReplyPacket : public ObjectPacket
    {
        BarrierEnterReplyPacket( const uint32_t sessionID, 
                                 const uint32_t objectID )
                : eqNet::ObjectPacket( sessionID, objectID )
            {
                command = CMD_BARRIER_ENTER_REPLY;
                size    = sizeof( BarrierEnterReplyPacket );
            }
    };

//     //------------------------------------------------------------
//     // User
//     //------------------------------------------------------------
//     struct UserPacket : public SessionPacket
//     {
//         UserPacket( const uint32_t sessionID, const uint32_t userID )
//                 : SessionPacket( sessionID )
//             {
//                 datatype     = DATATYPE_EQNET_USER; 
//                 this->userID = userID;
//             }
//         uint32_t userID;
//     };


    inline std::ostream& operator << ( std::ostream& os, 
                                       const Packet* packet )
    {
        os << "packet " << (void*)packet << " dt " << packet->datatype
           << " cmd "<< packet->command;
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
//     inline std::ostream& operator << ( std::ostream& os, 
//                                        const UserPacket* packet )
//     {
//         os << (SessionPacket*)packet << " user id " << packet->userID;
//         return os;
//     }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionGenIDsReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " id start " << packet->id;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                    const SessionGetObjectMasterPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->objectID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                             const SessionGetObjectMasterReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->objectID
           << " master " << packet->masterID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                   const SessionGetIDMasterPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->id;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                   const SessionGetIDMasterReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " IDs " << packet->start << "-" 
           << packet->end << " master " << packet->masterID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                 const SessionInstanciateObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " mobj id " << packet->objectID <<
            " type " << packet->objectType << " master " << packet->isMaster;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " obj id " << packet->objectID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectSyncPacket* packet )
    {
        os << (ObjectPacket*)packet << " version " << packet->version
           << " size " << packet->deltaSize;
        return os;
    }

}

#endif // EQNET_PACKETS_PRIV_H

