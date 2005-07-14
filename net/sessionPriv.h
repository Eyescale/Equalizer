
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_PRIV_H
#define EQNET_SESSION_PRIV_H

#include "base.h"
#include "idHash.h"
#include "nodePriv.h"
#include "session.h"

#include <iostream>

namespace eqNet
{
    class Node;

    namespace priv
    {
        class Server;
        struct Packet;

        inline std::ostream& operator << (std::ostream& os, Network* network);

        class Session : public eqNet::Session
        {
        public:
            /** 
             * Returns the session instance.
             * 
             * @param sessionID the identifier of the session.
             * @return the session.
             */
            static Session* get(const uint sessionID );

            /** 
             * Creates a new session on the specified server.
             * 
             * @param server the server address.
             */
            static Session* create( const char *server );

            /** @name Managing nodes */
            //*{
            /**
             * Adds a new node to this session.
             * 
             * @return the node.
             */
            Node* newNode();

            /**
             * Returns the local node.
             *
             * @return the local node.
             */
            Node* getLocalNode(){ return _localNode; }

            /** 
             * Gets a node using its identifier.
             * 
             * @param nodeID the node identifier.
             * @return the node.
             */
            Node* getNodeByID( const uint nodeID ){ return _nodes[nodeID]; }
            //*}

            /**
             * @name Managing Networks
             * 
             * Networks are used to create connectivity between nodes.
             * @sa Network, Node
             */
            //*{
            /**
             * Adds a new network to this session.
             *
             * @param protocol the network protocol.
             */
            Network* newNetwork( const NetworkProtocol protocol );

            /**
             * Deletes a network of this session.
             *
             * @param network the network to remove
             * @return <code>true</code> if the network was removed,
             *         <code>false</code> if not.
             */
            bool deleteNetwork( Network* network );
            //*}

            /** 
             * Creates a new session.
             * 
             * @param id the session id.
             * @param server the server for this session.
             */
            Session( const uint id, Server* server );

            /** 
             * Initialises a remote node.
             * 
             * This is used to copy the session information to a newly connected
             * node.
             *
             * @param nodeID the node identifier.
             * @return the success value.
             */
            bool initNode( const uint nodeID );

            /** 
             * Sets the node identifier of the local node.
             * 
             * @param nodeID the local node identifier.
             */
            void setLocalNode( const uint nodeID );

            /** 
             * Sends a packet to a node using the best network.
             * 
             * @param toNode the receiver node.
             * @param packet the packet.
             */
            void send( Node* toNode, const Packet& packet )
                { _localNode->send( toNode, packet ); }

        private:
            /** The list of nodes in this session. */
            IDHash<Node*> _nodes;

            /** The list of networks in this session. */
            IDHash<Network*> _networks;

            /** The next unique network identifier. */
            uint _networkID;

            /** The next unique node identifier. */
            uint _nodeID;

            /** The server node. */
            Server* _server;
            uint    _serverID;

            /** The local node. */
            Node* _localNode;
            uint  _localNodeID;

            /*virtual void pack( const Connection* connection, 
                               const bool fullUpdate );
            */

            friend inline std::ostream& operator << 
                (std::ostream& os, Session* session);
        };

        inline std::ostream& operator << ( std::ostream& os, Session* session )
        {
            os << "    Session " << session->getID() << "(" << (void*)session
               << "): " << session->_nodes.size() << " node[s], " 
               << session->_networks.size() << " network[s]" << std::endl;
            
            for( IDHash<Node*>::iterator iter = session->_nodes.begin();
                 iter != session->_nodes.end(); iter++ )
            {
                Node* node = (*iter).second;
                os << node << std::endl;
            }

            for( IDHash<Network*>::iterator iter = session->_networks.begin();
                 iter != session->_networks.end(); iter++ )
            {
                Network* network = (*iter).second;
                os << network;
            }

            return os;
        }
    }
}
#endif // EQNET_SESSION_PRIV_H

