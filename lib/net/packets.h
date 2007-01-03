
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKETS_H
#define EQNET_PACKETS_H

#include <eq/net/base.h>
#include <eq/net/commands.h>
#include <eq/net/global.h>
#include <eq/net/nodeID.h>
#include <eq/net/object.h>

#include <eq/base/idPool.h> // for EQ_ID_*

#define EQ_ALIGN8  __attribute__ ((aligned (8)))

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
                command                  = CMD_NODE_CONNECT;
                size                     = sizeof( NodeConnectPacket ); 
                requestID                = EQ_ID_INVALID;
                connectionDescription[0] = '\0';
            }

        NodeID   nodeID;
        uint32_t requestID;
        char     connectionDescription[8] EQ_ALIGN8;
    };

    struct NodeLaunchedPacket : public NodePacket
    {
        NodeLaunchedPacket() 
            {
                command                  = CMD_NODE_LAUNCHED;
                size                     = sizeof( NodeLaunchedPacket ); 
                requestID                = EQ_ID_INVALID;
                connectionDescription[0] = '\0';
            }

        NodeID   nodeID;
        uint32_t launchID;
        uint32_t requestID;
        char     connectionDescription[8] EQ_ALIGN8;
    };
    
    struct NodeConnectReplyPacket : public NodePacket
    {
        NodeConnectReplyPacket( const NodeConnectPacket* request ) 
            {
                command                  = CMD_NODE_CONNECT_REPLY;
                size                     = sizeof( NodeConnectReplyPacket ); 
                requestID                = request->requestID;
                connectionDescription[0] = '\0';
            }
        NodeConnectReplyPacket( const NodeLaunchedPacket* request ) 
            {
                command                  = CMD_NODE_CONNECT_REPLY;
                size                     = sizeof( NodeConnectReplyPacket ); 
                requestID                = request->requestID;
                connectionDescription[0] = '\0';
            }

        NodeID   nodeID;
        uint32_t requestID;
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
        uint32_t requestID;
        uint32_t index;
        bool     appRequest;
    };

    struct NodeGetConnectionDescriptionReplyPacket : public NodePacket
    {
        NodeGetConnectionDescriptionReplyPacket(
            const NodeGetConnectionDescriptionPacket* request )
            {
                command    = CMD_NODE_GET_CONNECTION_DESCRIPTION_REPLY;
                size       = sizeof( NodeGetConnectionDescriptionReplyPacket );
                nodeID     = request->nodeID;
                requestID  = request->requestID;
                appRequest = request->appRequest;
                connectionDescription[0] = '\0';
            } 

        NodeID   nodeID;
        uint32_t requestID;
        uint32_t nextIndex;
        bool     appRequest;
        char     connectionDescription[8] EQ_ALIGN8;
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

    struct SessionGetObjectMasterPacket : public SessionPacket
    {
        SessionGetObjectMasterPacket()
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
        SessionGetObjectPacket()
            {
                command = CMD_SESSION_GET_OBJECT;
                size    = sizeof( SessionGetObjectPacket ); 
            }
        
        uint32_t requestID;
    };

    struct SessionRegisterObjectPacket : public SessionPacket
    {
        SessionRegisterObjectPacket()
            {
                command = CMD_SESSION_REGISTER_OBJECT;
                size    = sizeof( SessionRegisterObjectPacket ); 
            }
        
        uint32_t            requestID;
        uint32_t            objectID;
        Object::SharePolicy policy;
    };

    struct SessionUnregisterObjectPacket : public SessionPacket
    {
        SessionUnregisterObjectPacket()
            {
                command = CMD_SESSION_UNREGISTER_OBJECT;
                size    = sizeof( SessionUnregisterObjectPacket ); 
            }
        
        uint32_t            requestID;
        uint32_t            objectID;
        Object::SharePolicy policy;
    };

    struct SessionInitObjectPacket : public SessionPacket
    {
        SessionInitObjectPacket()
            {
                command = CMD_SESSION_INIT_OBJECT;
                size    = sizeof( SessionInitObjectPacket ); 
            }

        uint32_t            objectID;
        uint32_t            version;
        Object::SharePolicy policy;
        bool                threadSafe;
    };

    struct SessionInstanciateObjectPacket : public SessionPacket
    {
        SessionInstanciateObjectPacket()
            {
                command       = CMD_SESSION_INSTANCIATE_OBJECT;
                size          = sizeof( SessionInstanciateObjectPacket ); 
                objectData[0] = '\0';
                error         = false;
            }

        uint64_t objectDataSize;
        uint32_t objectID;
        uint32_t objectType;
        uint32_t version;
        Object::SharePolicy policy;
        bool     isMaster;
        bool     error;
        bool     threadSafe;
        char     objectData[8] EQ_ALIGN8;
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

    struct ObjectCommitPacket : public ObjectPacket
    {
        ObjectCommitPacket()
            {
                command        = CMD_OBJECT_COMMIT;
                size           = sizeof( ObjectCommitPacket ); 
            }
        
        uint32_t requestID;
    };

    struct ObjectSyncPacket : public ObjectPacket
    {
        ObjectSyncPacket()
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
        os << (NodePacket*)packet << " req " << packet->requestID << " node "
           << packet->nodeID << " cd " << packet->connectionDescription;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeConnectReplyPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " node "
           << packet->nodeID << " cd " << packet->connectionDescription;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeLaunchedPacket* packet )
    {
        os << (NodePacket*)packet << " launch id " << packet->launchID 
           << " req " << packet->requestID << " node " << packet->nodeID
           << " cd " << packet->connectionDescription;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                              const NodeGetConnectionDescriptionPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " nodeID " 
           << packet->nodeID << " i " << packet->index;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                         const NodeGetConnectionDescriptionReplyPacket* packet )
    {
        os << (NodePacket*)packet << " req " << packet->requestID << " ni "  
           << packet->nextIndex << " app? " << packet->appRequest << " desc " 
           << packet->connectionDescription;
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
                                 const SessionInitObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->objectID 
           << " v" << packet->version << " ts " << packet->threadSafe;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                 const SessionInstanciateObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " id " << packet->objectID 
           << " err " << packet->error << " v" << packet->version << " type "
           << packet->objectType << " master " << packet->isMaster;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                 const SessionRegisterObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " objectID " << packet->objectID 
           << " policy " << packet->policy;
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
                                       const ObjectSyncPacket* packet )
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

