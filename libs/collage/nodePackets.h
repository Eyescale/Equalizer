
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef EQNET_NODEPACKETS_H
#define EQNET_NODEPACKETS_H

#include <co/packets.h> // base structs

/** @cond IGNORE */
namespace co
{
    struct NodeStopPacket : public NodePacket
    {
        NodeStopPacket()
            {
                command = CMD_NODE_STOP;
                size    = sizeof( NodeStopPacket );
            }
    };

    struct NodeConnectPacket : public NodePacket
    {
        NodeConnectPacket()
                : requestID( EQ_UNDEFINED_UINT32 )
                , fill( 0 )
            {
                command     = CMD_NODE_CONNECT;
                size        = sizeof( NodeConnectPacket ); 
                nodeData[0] = '\0';
            }

        NodeID   nodeID;
        uint32_t requestID;
        uint32_t nodeType;
        uint32_t fill;
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
        uint32_t nodeType;
        EQ_ALIGN8( char nodeData[8] );
    };

    struct NodeConnectAckPacket : public NodePacket
    {
        NodeConnectAckPacket() 
            {
                command     = CMD_NODE_CONNECT_ACK;
                size        = sizeof( NodeConnectAckPacket ); 
            }
    };

    struct NodeIDPacket : public NodePacket
    {
        NodeIDPacket() 
            {
                command     = CMD_NODE_ID;
                size        = sizeof( NodeIDPacket ); 
                data[0] = '\0';
            }

        NodeID   id;
        uint32_t nodeType;
        EQ_ALIGN8( char data[8] );
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
        uint32_t nodeType; 
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

    struct NodeAddListenerPacket : public NodePacket
    {
        NodeAddListenerPacket( ConnectionPtr conn )
                : connection( conn.get( ))
            {
                command = CMD_NODE_ADD_LISTENER;
                size    = sizeof( NodeAddListenerPacket );
                connectionData[0] = 0;
            }

        Connection* connection;
        EQ_ALIGN8( char connectionData[8] );
    };

    struct NodeRemoveListenerPacket : public NodePacket
    {
        NodeRemoveListenerPacket( ConnectionPtr conn, const uint32_t request )
                : connection( conn.get( ))
                , requestID( request )
            {
                command = CMD_NODE_REMOVE_LISTENER;
                size    = sizeof( NodeRemoveListenerPacket );
                connectionData[0] = 0;
            }

        Connection* connection;
        const uint32_t requestID;
        EQ_ALIGN8( char connectionData[8] );
    };

    struct NodeAckRequestPacket : public NodePacket
    {
        NodeAckRequestPacket( const uint32_t requestID_ )
            {
                command   = CMD_NODE_ACK_REQUEST;
                size      = sizeof( NodeAckRequestPacket ); 
                requestID = requestID_;
            }
        
        uint32_t requestID;
    };

    struct NodeFindMasterNodeIDPacket : public NodePacket
    {
        NodeFindMasterNodeIDPacket()
                : requestID( EQ_UNDEFINED_UINT32 )
            {
                command   = CMD_NODE_FIND_MASTER_NODE_ID;
                size      = sizeof( NodeFindMasterNodeIDPacket ); 
            }
        
        co::base::UUID identifier;
        uint32_t   requestID;
    };

    struct NodeFindMasterNodeIDReplyPacket : public NodePacket
    {
        NodeFindMasterNodeIDReplyPacket( 
                          const NodeFindMasterNodeIDPacket* request )
                : requestID( request->requestID )
            {
                command   = CMD_NODE_FIND_MASTER_NODE_ID_REPLY;
                size      = sizeof( NodeFindMasterNodeIDReplyPacket ); 
            }
        NodeID     masterNodeID;
        uint32_t   requestID;
    };

    struct NodeAttachObjectPacket : public NodePacket
    {
        NodeAttachObjectPacket()
            {
                command = CMD_NODE_ATTACH_OBJECT;
                size    = sizeof( NodeAttachObjectPacket ); 
            }
        
        co::base::UUID  objectID;
        uint32_t    requestID;
        uint32_t    objectInstanceID;
    };

    struct NodeMapObjectPacket : public NodePacket
    {
        NodeMapObjectPacket()
            {
                command = CMD_NODE_MAP_OBJECT;
                size    = sizeof( NodeMapObjectPacket );
                minCachedVersion = VERSION_HEAD;
                maxCachedVersion = VERSION_NONE;
                useCache   = false;
            }

        uint128_t     requestedVersion;
        uint128_t     minCachedVersion;
        uint128_t     maxCachedVersion;
        co::base::UUID    objectID;
        uint32_t      requestID;
        uint32_t instanceID;
        uint32_t masterInstanceID;
        bool     useCache;
    };


    struct NodeMapObjectSuccessPacket : public NodePacket
    {
        NodeMapObjectSuccessPacket( 
            const NodeMapObjectPacket* request )
            {
                command    = CMD_NODE_MAP_OBJECT_SUCCESS;
                size       = sizeof( NodeMapObjectSuccessPacket ); 
                requestID  = request->requestID;
                objectID   = request->objectID;
                instanceID = request->instanceID;
            }
        
        NodeID nodeID;
        co::base::UUID objectID;
        uint32_t requestID;
        uint32_t instanceID;
        uint32_t changeType;
        uint32_t masterInstanceID;
    };

    struct NodeMapObjectReplyPacket : public NodePacket
    {
        NodeMapObjectReplyPacket( 
            const NodeMapObjectPacket* request )
                : objectID( request->objectID )
                , requestID( request->requestID )
                , useCache( request->useCache )
            {
                command   = CMD_NODE_MAP_OBJECT_REPLY;
                size      = sizeof( NodeMapObjectReplyPacket ); 
//                sessionID = request->sessionID;
                version   = request->requestedVersion;
                cachedVersion = VERSION_INVALID;
            }
        
        NodeID nodeID;
        const co::base::UUID objectID;
        uint128_t version;
        uint128_t cachedVersion;
        const uint32_t requestID;
        
        bool result;
        const bool useCache;
    };

    struct NodeUnmapObjectPacket : public NodePacket
    {
        NodeUnmapObjectPacket()
            {
                command = CMD_NODE_UNMAP_OBJECT;
                size    = sizeof( NodeUnmapObjectPacket ); 
            }
        
        co::base::UUID objectID;
    };

    struct NodeUnsubscribeObjectPacket : public NodePacket
    {
        NodeUnsubscribeObjectPacket()
            {
                command = CMD_NODE_UNSUBSCRIBE_OBJECT;
                size    = sizeof( NodeUnsubscribeObjectPacket ); 
            }
       
        co::base::UUID      objectID;
        uint32_t            requestID;
        uint32_t            masterInstanceID;
        uint32_t            slaveInstanceID;
    };

    struct NodeRegisterObjectPacket : public NodePacket
    {
        NodeRegisterObjectPacket()
            {
                command = CMD_NODE_REGISTER_OBJECT;
                size    = sizeof( NodeRegisterObjectPacket ); 
            }
        
        Object* object;
    };

    struct NodeDeregisterObjectPacket : public NodePacket
    {
        NodeDeregisterObjectPacket()
            {
                command = CMD_NODE_DEREGISTER_OBJECT;
                size    = sizeof( NodeDeregisterObjectPacket ); 
            }
        
        uint32_t requestID;
    };

    struct NodeDetachObjectPacket : public NodePacket
    {
        NodeDetachObjectPacket()
                : requestID( EQ_UNDEFINED_UINT32 )
        {
            command   = CMD_NODE_DETACH_OBJECT;
            size      = sizeof( NodeDetachObjectPacket ); 
        }

        NodeDetachObjectPacket(const NodeUnsubscribeObjectPacket* request)
                : requestID( request->requestID )
        {
            command   = CMD_NODE_DETACH_OBJECT;
            size      = sizeof( NodeDetachObjectPacket ); 
            objectID  = request->objectID;
            objectInstanceID = request->slaveInstanceID;
        }

        co::base::UUID      objectID;
        uint32_t            requestID;
        uint32_t            objectInstanceID;
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeConnectPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " type "
           << packet->nodeType << " data " << packet->nodeData;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeConnectReplyPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " type "
           << packet->nodeType << " data " << packet->nodeData;
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
    inline std::ostream& operator << ( std::ostream& os, 
                                    const NodeMapObjectPacket* packet )
    {
        os << (NodePacket*)packet << " id " << packet->objectID << "." 
           << packet->instanceID << " req " << packet->requestID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                             const NodeMapObjectSuccessPacket* packet )
    {
        os << (NodePacket*)packet << " id " << packet->objectID << "." 
           << packet->instanceID << " req " << packet->requestID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                               const NodeMapObjectReplyPacket* packet )
    {
        os << (NodePacket*)packet << " id " << packet->objectID << " req "
           << packet->requestID << " v" << packet->cachedVersion;
        return os;
    }


}
/** @endcond */

#endif // EQNET_NODEPACKETS_H

