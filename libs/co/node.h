
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef CO_NODE_H
#define CO_NODE_H

#include <co/dispatcher.h>        // base class
#include <co/connection.h>        // used in inline template method
#include <co/nodeType.h>          // for NODETYPE_CO_NODE enum
#include <co/types.h>

namespace co
{
namespace detail { class Node; }

    /**
     * Manages a node.
     *
     * A node represents a separate entity in a peer-to-peer network, typically
     * a process on a cluster node or on a shared-memory system. It should have
     * at least one Connection through which is reachable. A Node provides the
     * basic communication facilities through message passing.
     */
    class Node : public Dispatcher, public lunchbox::Referenced
    {
    public:
        /** Construct a new Node. */
        CO_API Node();

        /** @name Data Access. */
        //@{
        bool operator == ( const Node* n ) const;

        bool isReachable() const;
        bool isConnected() const;
        bool isClosed() const;
        bool isClosing() const;
        bool isListening() const;
        //@}

        /** @name Connectivity information. */
        //@{
        /** @return true if the node is local (listening), false otherwise. */
        bool isLocal() const { return isListening(); }

        /** 
         * Adds a new description how this node can be reached.
         * 
         * @param cd the connection description.
         */
        CO_API void addConnectionDescription( ConnectionDescriptionPtr cd );
        
        /** 
         * Removes a connection description.
         * 
         * @param cd the connection description.
         * @return true if the connection description was removed, false
         *         otherwise.
         */
        CO_API bool removeConnectionDescription(ConnectionDescriptionPtr cd);

        /** @return the number of stored connection descriptions. */
        CO_API ConnectionDescriptions getConnectionDescriptions() const;

        /** @return the connection to this node. */
        ConnectionPtr getConnection() const;

        /** @return the established multicast connection to this node. */
        ConnectionPtr getMulticast() const;

        /** @return the first usable multicast connection to this node, or 0. */
        ConnectionPtr useMulticast();
        //@}

        /** @name Messaging API */
        //@{
        /** 
         * Sends a packet to this node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool send( const Packet& packet )
        {
            ConnectionPtr connection = _getConnection();
            return connection ? connection->send( packet ) : false;
        }

        /** 
         * Sends a packet with a string to the node.
         * 
         * The data is send as a new packet containing the original packet and
         * the string, so that it is received as one packet by the node.
         *
         * It is assumed that the last 8 bytes in the packet are usable for the
         * string.  This is used for optimising the send of short strings and on
         * the receiver side to access the string. The node implementation gives
         * examples of this usage.
         *
         * @param packet the packet.
         * @param string the string.
         * @return the success status of the transaction.
         */
        bool send( Packet& packet, const std::string& string )
        {
            ConnectionPtr connection = _getConnection();
            return connection ? connection->send( packet, string ) : false;
        }

        /** 
         * Sends a packet with additional data to the node.
         * 
         * The data is send as a new packet containing the original packet and
         * the string, so that it is received as one packet by the node.
         *
         * It is assumed that the last item in the packet is of sizeof(T) and
         * usable for the data.
         *
         * @param packet the packet.
         * @param data the vector containing the data.
         * @return the success status of the transaction.
         */
        template< class T >
        bool send( Packet& packet, const std::vector<T>& data )
        {
            ConnectionPtr connection = _getConnection();
            return connection ? connection->send( packet, data ) : false;
        }

        /** 
         * Sends a packet with additional data to the node.
         * 
         * The data is send as a new packet containing the original packet and
         * the data, so that it is received as one packet by the node.
         *
         * It is assumed that the last 8 bytes in the packet are usable for the
         * data.  This is used for optimising the send of short data and on
         * the receiver side to access the data. The node implementation gives
         * examples of this usage.
         *
         * @param packet the packet.
         * @param data the data.
         * @param size the size of the data in bytes.
         * @return the success status of the transaction.
         */
        bool send( Packet& packet, const void* data, const uint64_t size )
        {
            ConnectionPtr connection = _getConnection();
            return connection ? connection->send( packet, data, size ) : false;
        }

        /** 
         * Multicasts a packet to the multicast group of this node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool multicast( const Packet& packet )
        {
            ConnectionPtr connection = useMulticast();
            return connection ? connection->send( packet ) : false;
        }
        //@}

        CO_API const NodeID& getNodeID() const;

        /** Serialize the node's information. */
        CO_API std::string serialize() const;
        /** Deserialize the node information, consumes given data. */
        CO_API bool deserialize( std::string& data );

        /** @return last receive time. */
        int64_t getLastReceiveTime() const;

        /** @return the type of the node, used during connect(). */
        virtual uint32_t getType() const { return NODETYPE_CO_NODE; }

    protected:
        /** Destructs this node. */
        CO_API virtual ~Node();

        /** 
         * Factory method to create a new node.
         * 
         * @param type the type the node type
         * @return the node.
         * @sa getType()
         */
        CO_API virtual NodePtr createNode( const uint32_t type );

    private:
        detail::Node* const _impl;
        friend std::ostream& operator << ( std::ostream& os, const Node& node );

        /** Ensures the connectivity of this node. */
        CO_API ConnectionPtr _getConnection();

        /** @internal @name Methods for LocalNode */
        //@{
        void _addMulticast( NodePtr node, ConnectionPtr connection );
        void _removeMulticast( ConnectionPtr connection );
        void _connectMulticast( NodePtr node );
        void _connectMulticast( NodePtr node, ConnectionPtr connection );
        void _setListening();
        void _setClosing();
        void _setClosed();
        void _connect( ConnectionPtr connection );
        void _disconnect();
        void _setLastReceive( const int64_t time );
        friend class LocalNode;
        //@}
    };

    CO_API std::ostream& operator << ( std::ostream& os, const Node& node );
}

#endif // CO_NODE_H
