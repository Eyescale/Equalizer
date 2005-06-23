
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_PRIV_H
#define EQNET_SESSION_PRIV_H

#include <eq/base/base.h>

#include "base.h"
#include "idHash.h"
#include "network.h"
#include "session.h"

#include <iostream>

namespace eqNet
{
    class Node;

    namespace priv
    {
        class Network;
        class Node;
        class Server;

        inline std::ostream& operator << (std::ostream& os, Network* network);

        class Session : public Base, public eqNet::Session
        {
        public:
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
            Node* addNode();

            /**
             * Adds an existing node to this session, used during startup.
             * 
             * @param node the node.
             */
            void addNode(Node* node);
            //*}

            /**
             * Adds a new network to this session.
             *
             * @param protocol the network protocol.
             */
            Network* addNetwork( const NetworkProtocol protocol );

            Session(const uint id);

        private:
            /** The list of nodes in this session. */
            IDHash<Node*> _nodes;

            /** The list of networks in this session. */
            IDHash<Network*> _networks;

            /** The next unique network identifier. */
            uint _networkID;

            /** The next unique node identifier. */
            uint _nodeID;

            bool _create( const char* serverAddress );

            /** 
             * Opens and returns a session to the specified server.
             * 
             * @param server the server address.
             * @return the Server object, or <code>NULL</code> if no
             *         server could be contacted.
             */
            Server* _openServer( const char* server );

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
                os << node;
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

