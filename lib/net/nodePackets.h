
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

#include <eq/net/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
namespace net
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
                : requestID( EQ_ID_INVALID )
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
                conn.ref();
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
                conn.ref();
                connectionData[0] = 0;
            }

        Connection* connection;
        const uint32_t requestID;
        EQ_ALIGN8( char connectionData[8] );
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

}
}
/** @endcond */

#endif // EQNET_NODEPACKETS_H

