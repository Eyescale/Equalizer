
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQNET_PACKETS_H
#define EQNET_PACKETS_H

#include <eq/net/commands.h> // used for CMD_ enums
#include <eq/net/types.h>

#include <eq/base/idPool.h> // for EQ_ID_*

namespace eq
{
namespace net
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
        Packet(){}
        uint64_t size;
        uint32_t datatype;
        uint32_t command;
        
#if 0
        union
        {
            Foo foo;
            uint64_t paddingPacket; // pad to multiple-of-8
        };
#endif

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

/** @cond IGNORE */
    struct NodeStopPacket : public NodePacket
    {
        NodeStopPacket()
            {
                command = CMD_NODE_STOP;
                size    = sizeof( NodeStopPacket );
            }
    };

    struct NodeRegisterSessionPacket : public NodePacket
    {
        NodeRegisterSessionPacket()
            {
                command   = CMD_NODE_REGISTER_SESSION;
                size      = sizeof(NodeRegisterSessionPacket);
            }

        uint32_t requestID;
    };

    struct NodeRegisterSessionReplyPacket : public NodePacket
    {
        NodeRegisterSessionReplyPacket(const NodeRegisterSessionPacket* request)
            {
                command   = CMD_NODE_REGISTER_SESSION_REPLY;
                size      = sizeof( NodeRegisterSessionReplyPacket );
                requestID = request->requestID;
            }
            
        uint32_t requestID;
        uint32_t sessionID;
    };

    struct NodeMapSessionPacket : public NodePacket
    {
        NodeMapSessionPacket()
            {
                command   = CMD_NODE_MAP_SESSION;
                size      = sizeof(NodeMapSessionPacket);
            }

        uint32_t requestID;
        uint32_t sessionID;
    };

    struct NodeMapSessionReplyPacket : public NodePacket
    {
        NodeMapSessionReplyPacket( const NodeMapSessionPacket* requestPacket ) 
            {
                command   = CMD_NODE_MAP_SESSION_REPLY;
                size      = sizeof( NodeMapSessionReplyPacket );
                requestID = requestPacket->requestID;
                sessionID = requestPacket->sessionID;
            }
            
        uint32_t requestID;
        uint32_t sessionID;
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

        NodeID   nodeID;
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

        NodeID   nodeID;
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
                nodeID      = request->nodeID;
                requestID   = request->requestID;
                nodeData[0] = '\0';
            } 

        NodeID   nodeID;
        uint32_t requestID;
        uint32_t type;        
        EQ_ALIGN8( char nodeData[8] );
    };

    struct NodeAcquireSendTokenPacket : public NodePacket
    {
        NodeAcquireSendTokenPacket()
            {
                command = CMD_NODE_ACQUIRE_SEND_TOKEN;
                size    = sizeof( NodeAcquireSendTokenPacket );
            }

        uint32_t requestID;
    };

    struct NodeAcquireSendTokenReplyPacket : public NodePacket
    {
        NodeAcquireSendTokenReplyPacket(
            const NodeAcquireSendTokenPacket* request )

            {
                command = CMD_NODE_ACQUIRE_SEND_TOKEN_REPLY;
                size    = sizeof( NodeAcquireSendTokenReplyPacket );
                requestID = request->requestID;
            }

        uint32_t requestID;
    };

    struct NodeReleaseSendTokenPacket : public NodePacket
    {
        NodeReleaseSendTokenPacket()
            {
                command = CMD_NODE_RELEASE_SEND_TOKEN;
                size    = sizeof( NodeReleaseSendTokenPacket );
            }
    };
/** @endcond */

    //------------------------------------------------------------
    // Session
    //------------------------------------------------------------
    struct SessionPacket : public NodePacket
    {
        SessionPacket() { datatype = DATATYPE_EQNET_SESSION; }
        uint32_t sessionID;
        uint32_t paddingSessionPacket; // pad to multiple-of-8
    };

/** @cond IGNORE */
    struct SessionAckRequestPacket : public SessionPacket
    {
        SessionAckRequestPacket( const uint32_t requestID_ )
            {
                command   = CMD_SESSION_ACK_REQUEST;
                size      = sizeof( SessionAckRequestPacket ); 
                requestID = requestID_;
            }
        
        uint32_t requestID;
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
                requested = request->range;
            }

        uint32_t requestID;
        uint32_t id;
        uint32_t requested;
        uint32_t allocated;
    };

    struct SessionSetIDMasterPacket : public SessionPacket
    {
        SessionSetIDMasterPacket()
            {
                command   = CMD_SESSION_SET_ID_MASTER;
                size      = sizeof( SessionSetIDMasterPacket ); 
                requestID = EQ_ID_INVALID;
            }

        NodeID   masterID;
        uint32_t id;
        uint32_t requestID;
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
                id        = request->id;
            }

        NodeID   masterID;
        uint32_t requestID;
        uint32_t id;
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
        uint32_t            objectInstanceID;
    };

    struct SessionMapObjectPacket : public SessionPacket
    {
        SessionMapObjectPacket()
            {
                command = CMD_SESSION_MAP_OBJECT;
                size    = sizeof( SessionMapObjectPacket ); 
            }
        
        NodeID   masterNodeID;
        uint32_t requestID;
        uint32_t objectID;
        uint32_t version;
    };

    struct SessionSubscribeObjectPacket : public SessionPacket
    {
        SessionSubscribeObjectPacket( const SessionMapObjectPacket* mapPacket )
            {
                command = CMD_SESSION_SUBSCRIBE_OBJECT;
                size    = sizeof( SessionSubscribeObjectPacket );
                requestID  = mapPacket->requestID;
                version    = mapPacket->version;
                objectID   = mapPacket->objectID;
            }
        
        uint32_t requestID;
        uint32_t objectID;
        uint32_t version;
        uint32_t instanceID;
    };

    struct SessionSubscribeObjectSuccessPacket : public SessionPacket
    {
        SessionSubscribeObjectSuccessPacket( 
            const SessionSubscribeObjectPacket* request )
            {
                command    = CMD_SESSION_SUBSCRIBE_OBJECT_SUCCESS;
                size       = sizeof( SessionSubscribeObjectSuccessPacket ); 
                requestID  = request->requestID;
                objectID   = request->objectID;
                instanceID = request->instanceID;
            }
        
        uint32_t requestID;
        uint32_t objectID;
        uint32_t instanceID;
        uint32_t changeType;
        uint32_t masterInstanceID;
    };

    struct SessionSubscribeObjectReplyPacket : public SessionPacket
    {
        SessionSubscribeObjectReplyPacket( 
            const SessionSubscribeObjectPacket* request )
            {
                command   = CMD_SESSION_SUBSCRIBE_OBJECT_REPLY;
                size      = sizeof( SessionSubscribeObjectReplyPacket ); 
                requestID = request->requestID;
                objectID  = request->objectID;
                version   = request->version;
            }
        
        uint32_t requestID;
        uint32_t objectID;
        uint32_t version;
        bool     result;
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
        uint32_t            masterInstanceID;
        uint32_t            slaveInstanceID;
    };

    struct SessionDetachObjectPacket : public SessionPacket
    {
        SessionDetachObjectPacket()
        {
            command   = CMD_SESSION_DETACH_OBJECT;
            size      = sizeof( SessionDetachObjectPacket ); 
            requestID = EQ_ID_INVALID;
        }

        SessionDetachObjectPacket(const SessionUnsubscribeObjectPacket* request)
        {
            command   = CMD_SESSION_DETACH_OBJECT;
            size      = sizeof( SessionDetachObjectPacket ); 
            requestID = request->requestID;
            objectID  = request->objectID;
            objectInstanceID = request->slaveInstanceID;
        }

        uint32_t            requestID;
        uint32_t            objectID;
        uint32_t            objectInstanceID;
    };
/** @endcond */

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

/** @cond IGNORE */
    struct ObjectInstanceDataPacket : public ObjectPacket
    {
        ObjectInstanceDataPacket()
            {
                command = CMD_OBJECT_INSTANCE_DATA;
                size    = sizeof( ObjectInstanceDataPacket ); 
                data[0] = '\0';
            }

        uint64_t dataSize;
        uint32_t sequence;
        EQ_ALIGN8( uint8_t data[8] );
    };

    struct ObjectInstancePacket : public ObjectPacket
    {
        ObjectInstancePacket()
            {
                command = CMD_OBJECT_INSTANCE;
                size    = sizeof( ObjectInstancePacket ); 
                data[0] = '\0';
            }

        uint64_t dataSize;
        uint32_t version;
        uint32_t sequence;
        EQ_ALIGN8( uint8_t data[8] );
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
        
        uint64_t deltaSize;
        EQ_ALIGN8( uint8_t     delta[8] );
    };

    struct ObjectDeltaPacket : public ObjectPacket
    {
        ObjectDeltaPacket()
            {
                command        = CMD_OBJECT_DELTA;
                size           = sizeof( ObjectDeltaPacket ); 
                delta[0]       = '\0';
            }
        
        uint64_t deltaSize;
        uint32_t version;
        EQ_ALIGN8( uint8_t     delta[8] );
    };

    struct ObjectNewMasterPacket : public ObjectPacket
    {
        ObjectNewMasterPacket()
            {
                command = CMD_OBJECT_NEW_MASTER;
                size    = sizeof( ObjectNewMasterPacket );
            };

        uint32_t newMasterID;
        uint32_t newMasterInstanceID;
        uint32_t changeType;
    };

    struct ObjectVersionPacket : public ObjectPacket
    {
        ObjectVersionPacket()
            {
                command = CMD_OBJECT_VERSION;
                size    = sizeof( ObjectVersionPacket );
            };

        uint32_t version;
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
/** @endcond */

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
           << " sessionID " << packet->sessionID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeMapSessionReplyPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID
           << " sessionID " << packet->sessionID;
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
        os << (SessionPacket*)packet << " ID " << packet->id << " master "
           << packet->masterID;
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
           << "." << packet->instanceID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectInstanceDataPacket* packet )
    {
        os << (ObjectPacket*)packet << " size " << packet->dataSize;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectInstancePacket* packet )
    {
        os << (ObjectPacket*)packet << " v" << packet->version
           << " size " << packet->dataSize;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectDeltaDataPacket* packet )
    {
        os << (ObjectPacket*)packet << " size " << packet->deltaSize;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectDeltaPacket* packet )
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
}

#endif // EQNET_PACKETS_H

