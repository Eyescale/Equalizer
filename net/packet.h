
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKET_PRIV_H
#define EQNET_PACKET_PRIV_H

#include "connectionDescription.h"
#include "global.h"

#include "networkPriv.h"

#include <sys/param.h>

namespace eqNet
{
    enum NetworkProtocol;

    namespace priv
    {
        enum 
        {
            DATATYPE_SERVER,
            DATATYPE_SESSION,
            DATATYPE_NETWORK,
            DATATYPE_NODE,
            DATATYPE_USER = 1<<16
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
        // server
        //------------------------------------------------------------
        struct ServerPacket: public Packet
        {
            ServerPacket(){ datatype = DATATYPE_SERVER; }
            uint64  serverID;
        };

        struct SessionCreatePacket : public ServerPacket
        {
            SessionCreatePacket()
                { 
                    command  = CMD_SESSION_CREATE;
                    serverID = INVALID_ID;
                    size     = sizeof( SessionCreatePacket ); 
                }

            char requestorAddress[MAXHOSTNAMELEN+1];
        };            

        struct SessionCreatedPacket : public ServerPacket
        {
            SessionCreatedPacket() 
                {
                    command  = CMD_SESSION_CREATED;
                    size     = sizeof( SessionCreatedPacket ); 
                }
            
            uint sessionID;
            uint localNodeID;
        };

        struct SessionNewPacket : public ServerPacket
        {
            SessionNewPacket() 
                {
                    command  = CMD_SESSION_NEW;
                    size     = sizeof( SessionNewPacket ); 
                }

            uint sessionID;
        };

        //------------------------------------------------------------
        // Session
        //------------------------------------------------------------
        struct SessionPacket : public ServerPacket
        {
            SessionPacket(){ datatype = DATATYPE_SESSION; }
            uint sessionID;
        };

        struct NodeNewPacket : public SessionPacket
        {
            NodeNewPacket() 
                {
                    command  = CMD_NODE_NEW;
                    size     = sizeof( NodeNewPacket ); 
                }

            uint nodeID;
        };

        struct NetworkNewPacket : public SessionPacket
        {
            NetworkNewPacket() 
                {
                    command  = CMD_NETWORK_NEW;
                    size     = sizeof( NetworkNewPacket ); 
                }

            uint            networkID;
            Network::State  state;
            NetworkProtocol protocol;
        };

        //------------------------------------------------------------
        // Network
        //------------------------------------------------------------
        struct NetworkPacket : public SessionPacket
        {
            NetworkPacket(){ datatype = DATATYPE_NETWORK; }
            uint networkID;
        };

        struct NetworkAddNodePacket : public NetworkPacket
        {
            NetworkAddNodePacket() 
                {
                    command  = CMD_NETWORK_ADD_NODE;
                    size     = sizeof( NetworkAddNodePacket ); 
                }

            uint                  nodeID;
            ConnectionDescription connectionDescription;
            Network::NodeState    nodeState;
        };

        //------------------------------------------------------------
        // Node
        //------------------------------------------------------------
        struct NodePacket : public SessionPacket
        {
            NodePacket(){ datatype = DATATYPE_NODE; }
            uint nodeID;
        };


        inline std::ostream& operator << ( std::ostream& os, 
                                           const Packet* packet )
        {
            os << "Packet dt " << packet->datatype << " cmd "<< packet->command;
            return os;
        }
        inline std::ostream& operator << ( std::ostream& os, 
                                           const ServerPacket* packet )
        {
            os << (Packet*)packet << " srv " << packet->serverID;
            return os;
        }
        inline std::ostream& operator << ( std::ostream& os, 
                                           const SessionPacket* packet )
        {
            os << (ServerPacket*)packet << " ssn " << packet->sessionID;
            return os;
        }
        inline std::ostream& operator << ( std::ostream& os, 
                                           const NetworkPacket* packet )
        {
            os << (SessionPacket*)packet << " nwk " << packet->networkID;
            return os;
        }
        inline std::ostream& operator << ( std::ostream& os, 
                                           const NodePacket* packet )
        {
            os << (SessionPacket*)packet << " nod " << packet->nodeID;
            return os;
        }
    }
}

#endif // EQNET_PACKET_PRIV_H

