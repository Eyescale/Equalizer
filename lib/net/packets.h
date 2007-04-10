
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKETS_H
#define EQNET_PACKETS_H

#include <eq/net/commands.h> // used for CMD_ enums
#include <eq/net/nodeID.h>   // member
#include <eq/net/objectCM.h> // for ObjectCM::Type enum

#include <eq/base/idPool.h> // for EQ_ID_*

#ifdef WIN32
#  define EQ_ALIGN8( var ) __declspec (align (8)) var;
#else
#  define EQ_ALIGN8( var ) var __attribute__ ((aligned (8)));
#endif

namespace eqNet
{
    enum
    {
        DATATYPE_EQNET_NODE,
        DATATYPE_EQNET_SESSION,
        DATATYPE_EQNET_OBJECT,
        DATATYPE_EQNET_CUSTOM = 1<<7
    };

    /**
     * Represents a packet.
     */
    struct Packet
    {
        uint64_t size;
        uint32_t datatype;
        uint32_t command;
        
        static size_t minSize;
        bool exceedsMinSize() const { return (size > minSize); }
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
                sessionID = EQ_ID_INVALID;
                name[0]   = '\0';
            }

        uint32_t requestID;
        uint32_t sessionID;
        EQ_ALIGN8( char name[8] );
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
        EQ_ALIGN8( char     name[8] );
    };

    struct NodeUnmapSessionPacket : public NodePacket
    {
        NodeUnmapSessionPacket()
            {
                command   = CMD_NODE_UNMAP_SESSION;
                size      = sizeof(NodeUnmapSessionPacket);
                sessionID = EQ_ID_INVALID;
            }

        uint32_t requestID;
        uint32_t sessionID;
    };

    struct NodeUnmapSessionReplyPacket : public NodePacket
    {
        NodeUnmapSessionReplyPacket( const NodeUnmapSessionPacket* request )
            {
                command   = CMD_NODE_UNMAP_SESSION_REPLY;
                size      = sizeof( NodeUnmapSessionReplyPacket );
                requestID = request->requestID;
            }
            
        uint32_t requestID;
        bool     result;
    };

    struct NodeConnectPacket : public NodePacket
    {
        NodeConnectPacket() 
            {
                command     = CMD_NODE_CONNECT;
                size        = sizeof( NodeConnectPacket ); 
                requestID   = EQ_ID_INVALID;
                launchID    = EQ_ID_INVALID;
                nodeData[0] = '\0';
            }

        uint32_t requestID;
        uint32_t type;
        uint32_t launchID;
        EQ_ALIGN8( char nodeData[8] );
    };

    struct NodeConnectReplyPacket : public NodePacket
    {
        NodeConnectReplyPacket( const NodeConnectPacket* request ) 
            {
                command     = CMD_NODE_CONNECT_REPLY;
                size        = sizeof( NodeConnectReplyPacket ); 
                requestID   = request->requestID;
                nodeData[0] = '\0';
            }

        uint32_t requestID;
        uint32_t type;
        EQ_ALIGN8( char nodeData[8] );
    };

    struct NodeDisconnectPacket : public NodePacket
    {
        NodeDisconnectPacket() 
            {
                command                  = CMD_NODE_DISCONNECT;
                size                     = sizeof( NodeDisconnectPacket ); 
            }

        uint32_t requestID;
    };

    struct NodeGetNodeDataPacket : public NodePacket
    {
        NodeGetNodeDataPacket()
            {
                command = CMD_NODE_GET_NODE_DATA;
                size    = sizeof( NodeGetNodeDataPacket );
            }

        NodeID   nodeID;
        uint32_t requestID;
    };

    struct NodeGetNodeDataReplyPacket : public NodePacket
    {
        NodeGetNodeDataReplyPacket(
            const NodeGetNodeDataPacket* request )
            {
                command     = CMD_NODE_GET_NODE_DATA_REPLY;
                size        = sizeof( NodeGetNodeDataReplyPacket );
                requestID   = request->requestID;
                nodeData[0] = '\0';
            } 

        uint32_t requestID;
        uint32_t type;        
        EQ_ALIGN8( char nodeData[8] );
    };

    //------------------------------------------------------------
    // Session
    //------------------------------------------------------------
    struct SessionPacket : public NodePacket
    {
        SessionPacket() { datatype = DATATYPE_EQNET_SESSION; }
        uint32_t sessionID;
    };

    struct SessionGenIDsPacket : public SessionPacket
    {
        SessionGenIDsPacket() 
            {
                command = CMD_SESSION_GEN_IDS;
                size    = sizeof( SessionGenIDsPacket ); 
            }

        uint32_t requestID;
        uint32_t range;
    };

    struct SessionGenIDsReplyPacket : public SessionPacket
    {
        SessionGenIDsReplyPacket( const SessionGenIDsPacket* request )
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
        SessionSetIDMasterPacket()
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
        SessionGetIDMasterPacket()
            {
                command = CMD_SESSION_GET_ID_MASTER;
                size    = sizeof( SessionGetIDMasterPacket ); 
            }

        uint32_t requestID;
        uint32_t id;
    };

    struct SessionGetIDMasterReplyPacket : public SessionPacket
    {
        SessionGetIDMasterReplyPacket( const SessionGetIDMasterPacket* request )
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

    struct SessionGetObjectPacket : public SessionPacket
    {
        SessionGetObjectPacket()
            {
                command = CMD_SESSION_GET_OBJECT;
                size    = sizeof( SessionGetObjectPacket ); 
            }
        
        uint32_t requestID;
    };

    struct SessionAttachObjectPacket : public SessionPacket
    {
        SessionAttachObjectPacket()
            {
                command = CMD_SESSION_ATTACH_OBJECT;
                size    = sizeof( SessionAttachObjectPacket ); 
            }
        
        uint32_t            requestID;
        uint32_t            objectID;
    };

    struct SessionDetachObjectPacket : public SessionPacket
    {
        SessionDetachObjectPacket()
            {
                command = CMD_SESSION_DETACH_OBJECT;
                size    = sizeof( SessionDetachObjectPacket ); 
            }
        
        uint32_t            requestID;
    };

    struct SessionMapObjectPacket : public SessionPacket
    {
        SessionMapObjectPacket()
            {
                command = CMD_SESSION_MAP_OBJECT;
                size    = sizeof( SessionMapObjectPacket ); 
            }
        
        uint32_t            requestID;
    };

    struct SessionUnmapObjectPacket : public SessionPacket
    {
        SessionUnmapObjectPacket()
            {
                command = CMD_SESSION_UNMAP_OBJECT;
                size    = sizeof( SessionUnmapObjectPacket ); 
            }
        
        uint32_t            requestID;
    };

    struct SessionSubscribeObjectPacket : public SessionPacket
    {
        SessionSubscribeObjectPacket()
            {
                command = CMD_SESSION_SUBSCRIBE_OBJECT;
                size    = sizeof( SessionSubscribeObjectPacket ); 
            }
        
        uint32_t            requestID;
        uint32_t            objectID;
        uint32_t            instanceID;
    };

    struct SessionSubscribeObjectSuccessPacket : public SessionPacket
    {
        SessionSubscribeObjectSuccessPacket( 
            const SessionSubscribeObjectPacket* request )
            {
                command    = CMD_SESSION_SUBSCRIBE_OBJECT_SUCCESS;
                size       = sizeof( SessionSubscribeObjectSuccessPacket ); 
                requestID  = request->requestID;
                instanceID = request->instanceID;
            }
        
        uint64_t       dataSize;
        uint32_t       requestID;
        uint32_t       instanceID;
        uint32_t       version;
        ObjectCM::Type cmType;
        EQ_ALIGN8( char data[8] );
    };

    struct SessionSubscribeObjectReplyPacket : public SessionPacket
    {
        SessionSubscribeObjectReplyPacket( 
            const SessionSubscribeObjectPacket* request )
            {
                command   = CMD_SESSION_SUBSCRIBE_OBJECT_REPLY;
                size      = sizeof( SessionSubscribeObjectReplyPacket ); 
                requestID = request->requestID;
            }
        
        uint32_t            requestID;
    };

    struct SessionUnsubscribeObjectPacket : public SessionPacket
    {
        SessionUnsubscribeObjectPacket()
            {
                command = CMD_SESSION_UNSUBSCRIBE_OBJECT;
                size    = sizeof( SessionUnsubscribeObjectPacket ); 
            }
        
        uint32_t            requestID;
        uint32_t            objectID;
    };

    struct SessionUnsubscribeObjectReplyPacket : public SessionPacket
    {
        SessionUnsubscribeObjectReplyPacket( 
            const SessionUnsubscribeObjectPacket* request )
            {
                command   = CMD_SESSION_UNSUBSCRIBE_OBJECT_REPLY;
                size      = sizeof( SessionUnsubscribeObjectReplyPacket ); 
                requestID = request->requestID;
            }
        
        uint32_t            requestID;
    };

    //------------------------------------------------------------
    // Object
    //------------------------------------------------------------
    struct ObjectPacket : public SessionPacket
    {
        ObjectPacket()
            {
                datatype   = DATATYPE_EQNET_OBJECT; 
                instanceID = EQ_ID_ANY;
            }
        uint32_t objectID;
        uint32_t instanceID;
    };

    struct ObjectInstanceDataPacket : public ObjectPacket
    {
        ObjectInstanceDataPacket()
            {
                command = CMD_OBJECT_INSTANCE_DATA;
                size    = sizeof( ObjectInstanceDataPacket ); 
                data[0] = '\0';
            }

        uint64_t dataSize;
        uint32_t version;
        EQ_ALIGN8( char data[8] );
    };

    struct ObjectCommitPacket : public ObjectPacket
    {
        ObjectCommitPacket()
            {
                command        = CMD_OBJECT_COMMIT;
                size           = sizeof( ObjectCommitPacket ); 
            }
        
        uint32_t requestID;
    };

    struct ObjectDeltaDataPacket : public ObjectPacket
    {
        ObjectDeltaDataPacket()
            {
                command        = CMD_OBJECT_DELTA_DATA;
                size           = sizeof( ObjectDeltaDataPacket ); 
                delta[0]       = '\0';
            }
        
        uint32_t version;
        uint64_t deltaSize;
        EQ_ALIGN8( char     delta[8] );
    };

    //------------------------------------------------------------
    // Barrier
    //------------------------------------------------------------
    struct BarrierEnterPacket : public ObjectPacket
    {
        BarrierEnterPacket()
            {
                command = CMD_BARRIER_ENTER;
                size    = sizeof( BarrierEnterPacket );
            }
        uint32_t version;
    };

    struct BarrierEnterReplyPacket : public ObjectPacket
    {
        BarrierEnterReplyPacket()
            {
                command = CMD_BARRIER_ENTER_REPLY;
                size    = sizeof( BarrierEnterReplyPacket );
            }
    };

    //------------------------------------------------------------
    // ostream operators
    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const Packet* packet )
    {
        os << "packet dt " << packet->datatype << " cmd "
           << packet->command;
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
        os << (NodePacket*)packet << " req " << packet->requestID << " type "
           << packet->type << " launchID " << packet->launchID << " data "
           << packet->nodeData;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeConnectReplyPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " type "
           << packet->type << " data " << packet->nodeData;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                              const NodeGetNodeDataPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " nodeID " 
           << packet->nodeID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeGetNodeDataReplyPacket* packet)
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " type "
           << packet->type << " data " << packet->nodeData;
        return os;
    }

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionPacket* packet )
    {
        os << (NodePacket*)packet << " session id " << packet->sessionID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionGenIDsReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " id start " << packet->id;
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
                                 const SessionMapObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " requestID " << packet->requestID;
        return os;
    }

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " objectID " << packet->objectID
           << " inst " << packet->instanceID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectDeltaDataPacket* packet )
    {
        os << (ObjectPacket*)packet << " v" << packet->version
           << " size " << packet->deltaSize;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const BarrierEnterPacket* packet )
    {
        os << (ObjectPacket*)packet << " v" << packet->version;
        return os;
    }
}

#endif // EQNET_PACKETS_H

