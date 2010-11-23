
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

#ifndef EQNET_SESSIONPACKETS_H
#define EQNET_SESSIONPACKETS_H

#include <eq/net/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
namespace net
{
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

    struct SessionSetIDMasterPacket : public SessionPacket
    {
        SessionSetIDMasterPacket()
                : requestID( EQ_ID_INVALID )
            {
                command   = CMD_SESSION_SET_ID_MASTER;
                size      = sizeof( SessionSetIDMasterPacket ); 
            }
        
        base::UUID identifier;
        NodeID     masterID;
        uint32_t   requestID;
    };

    struct SessionUnsetIDMasterPacket : public SessionPacket
    {
        SessionUnsetIDMasterPacket()
                : requestID( EQ_ID_INVALID )
            {
                command   = CMD_SESSION_UNSET_ID_MASTER;
                size      = sizeof( SessionUnsetIDMasterPacket ); 
            }

        base::UUID identifier;
        uint32_t   requestID;
    };

    struct SessionGetIDMasterPacket : public SessionPacket
    {
        SessionGetIDMasterPacket()
            {
                command = CMD_SESSION_GET_ID_MASTER;
                size    = sizeof( SessionGetIDMasterPacket ); 
            }

        base::UUID identifier;
        uint32_t   requestID;
    };

    struct SessionGetIDMasterReplyPacket : public SessionPacket
    {
        SessionGetIDMasterReplyPacket( const SessionGetIDMasterPacket* request )
            {
                command   = CMD_SESSION_GET_ID_MASTER_REPLY;
                size      = sizeof( SessionGetIDMasterReplyPacket );
                requestID = request->requestID;
                identifier = request->identifier;
            }
        
        base::UUID identifier;
        NodeID     masterID;
        uint32_t   requestID;
    };

    struct SessionAttachObjectPacket : public SessionPacket
    {
        SessionAttachObjectPacket()
            {
                command = CMD_SESSION_ATTACH_OBJECT;
                size    = sizeof( SessionAttachObjectPacket ); 
            }
        
        base::UUID  objectID;
        uint32_t    requestID;
        uint32_t    objectInstanceID;
    };

    struct SessionMapObjectPacket : public SessionPacket
    {
        SessionMapObjectPacket()
            {
                command = CMD_SESSION_MAP_OBJECT;
                size    = sizeof( SessionMapObjectPacket );
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


    struct SessionMapObjectSuccessPacket : public SessionPacket
    {
        SessionMapObjectSuccessPacket( 
            const SessionMapObjectPacket* request )
            {
                command    = CMD_SESSION_MAP_OBJECT_SUCCESS;
                size       = sizeof( SessionMapObjectSuccessPacket ); 
                sessionID  = request->sessionID;
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

    struct SessionMapObjectReplyPacket : public SessionPacket
    {
        SessionMapObjectReplyPacket( 
            const SessionMapObjectPacket* request )
                : requestID( request->requestID )
                , objectID( request->objectID )
                , useCache( request->useCache )
            {
                command   = CMD_SESSION_MAP_OBJECT_REPLY;
                size      = sizeof( SessionMapObjectReplyPacket ); 
                sessionID = request->sessionID;
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

    struct SessionUnmapObjectPacket : public SessionPacket
    {
        SessionUnmapObjectPacket()
            {
                command = CMD_SESSION_UNMAP_OBJECT;
                size    = sizeof( SessionUnmapObjectPacket ); 
            }
        
        base::UUID objectID;
    };

    struct SessionUnsubscribeObjectPacket : public SessionPacket
    {
        SessionUnsubscribeObjectPacket()
            {
                command = CMD_SESSION_UNSUBSCRIBE_OBJECT;
                size    = sizeof( SessionUnsubscribeObjectPacket ); 
            }
       
        base::UUID          objectID;
        uint32_t            requestID;
        uint32_t            masterInstanceID;
        uint32_t            slaveInstanceID;
    };

    struct SessionRegisterObjectPacket : public SessionPacket
    {
        SessionRegisterObjectPacket()
            {
                command = CMD_SESSION_REGISTER_OBJECT;
                size    = sizeof( SessionRegisterObjectPacket ); 
            }
        
        Object* object;
    };

    struct SessionDeregisterObjectPacket : public SessionPacket
    {
        SessionDeregisterObjectPacket()
            {
                command = CMD_SESSION_DEREGISTER_OBJECT;
                size    = sizeof( SessionDeregisterObjectPacket ); 
            }
        
        uint32_t requestID;
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

        base::UUID          objectID;
        uint32_t            requestID;
        uint32_t            objectInstanceID;
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionGetIDMasterPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->identifier;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                   const SessionGetIDMasterReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->identifier
           << " master " << packet->masterID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                    const SessionMapObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->objectID << "." 
           << packet->instanceID << " req " << packet->requestID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                             const SessionMapObjectSuccessPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->objectID << "." 
           << packet->instanceID << " req " << packet->requestID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                               const SessionMapObjectReplyPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->objectID << " req "
           << packet->requestID << " v" << packet->cachedVersion;
        return os;
    }
}
}
/** @endcond */

#endif // EQNET_SESSIONPACKETS_H

