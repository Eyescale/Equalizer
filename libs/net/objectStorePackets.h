
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

#ifndef EQNET_ObjectStorePacketS_H
#define EQNET_ObjectStorePacketS_H

#include <eq/net/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
namespace net
{
    struct ObjectStoreAckRequestPacket : public ObjectStorePacket
    {
        ObjectStoreAckRequestPacket( const uint32_t requestID_ )
            {
                command   = CMD_OBJECTSTORE_ACK_REQUEST;
                size      = sizeof( ObjectStoreAckRequestPacket ); 
                requestID = requestID_;
            }
        
        uint32_t requestID;
    };

    struct ObjectStoreFindMasterNodeID : public ObjectStorePacket
    {
        ObjectStoreFindMasterNodeID()
                : requestID( EQ_ID_INVALID )
            {
                command   = CMD_OBJECTSTORE_FIND_MASTER_NODE_ID;
                size      = sizeof( ObjectStoreFindMasterNodeID ); 
            }
        
        base::UUID identifier;
        uint32_t   requestID;
    };

    struct ObjectStoreFindMasterNodeIDReply : public ObjectStorePacket
    {
        ObjectStoreFindMasterNodeIDReply( 
                          const ObjectStoreFindMasterNodeID* request )
                : requestID( request->requestID )
            {
                command   = CMD_OBJECTSTORE_FIND_MASTER_NODE_ID_REPLY;
                size      = sizeof( ObjectStoreFindMasterNodeIDReply ); 
            }
        NodeID     masterNodeID;
        uint32_t   requestID;
    };

    struct ObjectStoreAttachObjectPacket : public ObjectStorePacket
    {
        ObjectStoreAttachObjectPacket()
            {
                command = CMD_OBJECTSTORE_ATTACH_OBJECT;
                size    = sizeof( ObjectStoreAttachObjectPacket ); 
            }
        
        base::UUID  objectID;
        uint32_t    requestID;
        uint32_t    objectInstanceID;
    };

    struct ObjectStoreMapObjectPacket : public ObjectStorePacket
    {
        ObjectStoreMapObjectPacket()
            {
                command = CMD_OBJECTSTORE_MAP_OBJECT;
                size    = sizeof( ObjectStoreMapObjectPacket );
                minCachedVersion = VERSION_HEAD;
                maxCachedVersion = VERSION_NONE;
                useCache   = false;
            }

        uint128_t     requestedVersion;
        uint128_t     minCachedVersion;
        uint128_t     maxCachedVersion;
        base::UUID    objectID;
        uint32_t      requestID;
        uint32_t instanceID;
        uint32_t masterInstanceID;
        bool     useCache;
    };


    struct ObjectStoreMapObjectSuccessPacket : public ObjectStorePacket
    {
        ObjectStoreMapObjectSuccessPacket( 
            const ObjectStoreMapObjectPacket* request )
            {
                command    = CMD_OBJECTSTORE_MAP_OBJECT_SUCCESS;
                size       = sizeof( ObjectStoreMapObjectSuccessPacket ); 
                requestID  = request->requestID;
                objectID   = request->objectID;
                instanceID = request->instanceID;
            }
        
        NodeID nodeID;
        base::UUID objectID;
        uint32_t requestID;
        uint32_t instanceID;
        uint32_t changeType;
        uint32_t masterInstanceID;
    };

    struct ObjectStoreMapObjectReplyPacket : public ObjectStorePacket
    {
        ObjectStoreMapObjectReplyPacket( 
            const ObjectStoreMapObjectPacket* request )
                : objectID( request->objectID )
                , requestID( request->requestID )
                , useCache( request->useCache )
            {
                command   = CMD_OBJECTSTORE_MAP_OBJECT_REPLY;
                size      = sizeof( ObjectStoreMapObjectReplyPacket ); 
//                sessionID = request->sessionID;
                version   = request->requestedVersion;
                cachedVersion = VERSION_INVALID;
            }
        
        NodeID nodeID;
        const base::UUID objectID;
        uint128_t version;
        uint128_t cachedVersion;
        const uint32_t requestID;
        
        bool result;
        const bool useCache;
    };

    struct ObjectStoreUnmapObjectPacket : public ObjectStorePacket
    {
        ObjectStoreUnmapObjectPacket()
            {
                command = CMD_OBJECTSTORE_UNMAP_OBJECT;
                size    = sizeof( ObjectStoreUnmapObjectPacket ); 
            }
        
        base::UUID objectID;
    };

    struct ObjectStoreUnsubscribeObjectPacket : public ObjectStorePacket
    {
        ObjectStoreUnsubscribeObjectPacket()
            {
                command = CMD_OBJECTSTORE_UNSUBSCRIBE_OBJECT;
                size    = sizeof( ObjectStoreUnsubscribeObjectPacket ); 
            }
       
        base::UUID          objectID;
        uint32_t            requestID;
        uint32_t            masterInstanceID;
        uint32_t            slaveInstanceID;
    };

    struct ObjectStoreRegisterObjectPacket : public ObjectStorePacket
    {
        ObjectStoreRegisterObjectPacket()
            {
                command = CMD_OBJECTSTORE_REGISTER_OBJECT;
                size    = sizeof( ObjectStoreRegisterObjectPacket ); 
            }
        
        Object* object;
    };

    struct ObjectStoreDeregisterObjectPacket : public ObjectStorePacket
    {
        ObjectStoreDeregisterObjectPacket()
            {
                command = CMD_OBJECTSTORE_DEREGISTER_OBJECT;
                size    = sizeof( ObjectStoreDeregisterObjectPacket ); 
            }
        
        uint32_t requestID;
    };

    struct ObjectStoreDetachObjectPacket : public ObjectStorePacket
    {
        ObjectStoreDetachObjectPacket()
        {
            command   = CMD_OBJECTSTORE_DETACH_OBJECT;
            size      = sizeof( ObjectStoreDetachObjectPacket ); 
            requestID = EQ_ID_INVALID;
        }

        ObjectStoreDetachObjectPacket(const ObjectStoreUnsubscribeObjectPacket* request)
        {
            command   = CMD_OBJECTSTORE_DETACH_OBJECT;
            size      = sizeof( ObjectStoreDetachObjectPacket ); 
            requestID = request->requestID;
            objectID  = request->objectID;
            objectInstanceID = request->slaveInstanceID;
        }

        base::UUID          objectID;
        uint32_t            requestID;
        uint32_t            objectInstanceID;
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                    const ObjectStoreMapObjectPacket* packet )
    {
        os << (ObjectStorePacket*)packet << " id " << packet->objectID << "." 
           << packet->instanceID << " req " << packet->requestID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                             const ObjectStoreMapObjectSuccessPacket* packet )
    {
        os << (ObjectStorePacket*)packet << " id " << packet->objectID << "." 
           << packet->instanceID << " req " << packet->requestID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                               const ObjectStoreMapObjectReplyPacket* packet )
    {
        os << (ObjectStorePacket*)packet << " id " << packet->objectID << " req "
           << packet->requestID << " v" << packet->cachedVersion;
        return os;
    }
}
}
/** @endcond */

#endif // EQNET_ObjectStorePacketS_H

