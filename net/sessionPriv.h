
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_PRIV_H
#define EQNET_SESSION_PRIV_H

#include "session.h"
#include "basePriv.h"

#include "commands.h"
#include "idHash.h"

#include <iostream>

namespace eqNet
{
    class Node;

    namespace priv
    {
        class Node;
        class NodeList;
        class Network;
        class Server;

        struct Packet;
        struct SessionPacket;

        inline std::ostream& operator << ( std::ostream& os, Network* network );
        inline std::ostream& operator << ( std::ostream& os, const Node* node );

        class Session : public Base, public eqNet::Session
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
             * Sets the node identifier of the local node.
             * 
             * @param nodeID the local node identifier.
             */
            void setLocalNode( const uint nodeID );

            /** 
             * Handles a command packet.
             * 
             * @param packet the packet.
             */
            void handlePacket( const SessionPacket* packet );

            void pack( const NodeList& nodes, const bool initial );
            
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

            /** The local node. */
            Node* _localNode;

            /** The command handler function table. */
            void (eqNet::priv::Session::*_cmdHandler[CMD_SESSION_ALL])( const Packet* packet );

            // the command handler functions and helper functions
            void _cmdNodeNew( const Packet* packet );
            void _cmdNetworkNew( const Packet* packet );


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
                os << "    " << node << std::endl;
            }

            for( IDHash<Network*>::iterator iter = session->_networks.begin();
                 iter != session->_networks.end(); iter++ )
            {
                Network* network = (*iter).second;
                os << "    " << network << std::endl;
            }

            return os;
        }
    }
}
#endif // EQNET_SESSION_PRIV_H

