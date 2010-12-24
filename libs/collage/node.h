
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <co/connection.h>        // member - ConnectionPtr
#include <co/nodeType.h>          // for NODETYPE_CO_NODE enum
#include <co/types.h>

#include <co/base/spinLock.h>         // member

namespace co
{

    /**
     * Manages a node.
     *
     * A node represents a separate entity in a peer-to-peer network, typically
     * a process on a cluster node or on a shared-memory system. It should have
     * at least one Connection through which is reachable. A Node provides the
     * basic communication facilities through message passing.
     */
    class Node : public Dispatcher, public co::base::Referenced
    {
    public:
        /** Construct a new Node. */
        CO_API Node();

        /** @name Data Access. */
        //@{
        bool operator == ( const Node* n ) const;

        bool isConnected() const 
            { return (_state == STATE_CONNECTED || _state == STATE_LISTENING);}
        bool isClosed() const { return _state == STATE_CLOSED; }
        bool isListening() const { return _state == STATE_LISTENING; }

        //@}

        /**
         * @name Connectivity information. 
         */
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
        ConnectionPtr getConnection() const { return _outgoing; }

        /** @return the multicast connection to this node, or 0. */
        ConnectionPtr getMulticast();
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
                if( !connection )
                    return false;
                return connection->send( packet );
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
                if( !connection )
                    return false;
                return connection->send( packet, string );
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
                if( !connection )
                    return false;
                return connection->send( packet, data );
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
                if( !connection )
                    return false;
                return connection->send( packet, data, size );
            }

        /** 
         * Multicasts a packet to the multicast group of this node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool multicast( const Packet& packet )
            {
                ConnectionPtr connection = getMulticast();
                if( !connection )
                    return false;
                return connection->send( packet );
            }
        //@}

        const NodeID& getNodeID() const { return _id; }

        /** Serialize the node's information. */
        CO_API std::string serialize() const;
        /** Deserialize the node information, consumes given data. */
        CO_API bool deserialize( std::string& data );

    protected:
        /** Destructs this node. */
        CO_API virtual ~Node();

        /** @return the type of the node, used during connect(). */
        virtual uint32_t getType() const { return NODETYPE_CO_NODE; }

        /** 
         * Factory method to create a new node.
         * 
         * @param type the type the node type
         * @return the node.
         * @sa getType()
         */
        CO_API virtual NodePtr createNode( const uint32_t type );

    private:
        /** The state of the node. */
        enum State
        {
            STATE_CLOSED,    //!< initial state
            STATE_CONNECTED, //!< proxy for a remote node, connected  
            STATE_LISTENING  //!< local node, listening
        };

        friend CO_API std::ostream& operator << ( std::ostream& os, 
                                                     const Node& node );
        friend CO_API std::ostream& operator << ( std::ostream&,
                                                     const State );
        friend class LocalNode;

        /** Globally unique node identifier. */
        NodeID _id;

        /** The current state of this node. */
        State _state;

        /** The connection to this node. */
        ConnectionPtr _outgoing;

        /** The multicast connection to this node, can be 0. */
        co::base::Lockable< ConnectionPtr > _outMulticast;

        struct MCData
        {
            ConnectionPtr connection;
            NodePtr       node;
        };
        typedef std::vector< MCData > MCDatas;

        /** 
         * Unused multicast connections for this node.
         *
         * On the first multicast send usage, the connection is 'primed' by
         * sending our node identifier to the MC group, removed from this vector
         * and set as _outMulticast.
         */
        MCDatas _multicasts;

        /** The list of descriptions on how this node is reachable. */
        co::base::Lockable< ConnectionDescriptions, co::base::SpinLock >
            _connectionDescriptions;

        /** Ensures the connectivity of this node. */
        ConnectionPtr _getConnection()
            {
                ConnectionPtr connection = _outgoing;
                if( _state == STATE_CONNECTED || _state == STATE_LISTENING )
                    return connection;
                return 0;
            }
    };

    CO_API std::ostream& operator << ( std::ostream& os, const Node& node );
    CO_API std::ostream& operator << ( std::ostream&, const Node::State );
}

#endif // CO_NODE_H
