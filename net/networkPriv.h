
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NETWORK_PRIV_H
#define EQNET_NETWORK_PRIV_H

#include "network.h"
#include "base.h"
#include "connectionSet.h"

#include <eq/base/hash.h>

namespace eqNet
{
    std::ostream& operator << ( std::ostream& os,
        ConnectionDescription* description);

    namespace priv
    {
        class  Connection;
        class  Node;
        struct Packet;
        class  Session;

        class Network : public eqNet::Network
        {
        public:
            /** The network state. */
            enum State
            {
                STATE_STOPPED,
                STATE_STARTING,
                STATE_RUNNING
            };

            /** 
             * Constructs a new Network.
             * 
             * @param id the identifier of the network.
             * @param protocol the network protocol.
             * @return the network.
             */
            static Network* create( const uint id, Session* session,
                const eqNet::NetworkProtocol protocol );

            /**
             * Adds a node to this network.
             *
             * @param node the node.
             * @param connection the connection parameters.
             * @sa Node::enableForwarding(), Node::disableForwarding()
             */
            virtual void addNode( Node* nodeID,
                const eqNet::ConnectionDescription& connection );

            /**
             * @name State Management
             */
            //@{
            /**
             * Initialise this network.
             *
             * Initialising this network prepares the network to be
             * started. Some concrete implementations may contact the nodes to
             * start a process.
             *
             * @return <code>true</code> if the network was successfully
             *         initialized, <code>false</code> if not.
             */
            virtual bool init() = 0;

            /** Exits this network. */
            virtual void exit() = 0;

            /**
             * Start all nodes in this initialized network.
             *
             * @return <code>true</code> if all nodes in this
             *         network were successfully started, <code>false</code>
             *         if not.
             * @sa startNode(), init()
             */
            virtual bool start() = 0;

            /**
             * Stops all running nodes in this initialized network.
             *
             * @sa stopNode(), exit()
             */
            virtual void stop() = 0;
 
            /**
             * Starts a node in this initialized network.
             *
             * @param node the node.
             * @return <code>true</code> if the node was successfully
             *         started, <code>false</code> if not.
             * @sa start(), init()
             */
            virtual bool startNode( Node* node ) = 0;
            //@}

            /** 
             * Puts a node into started mode.
             *
             * Used for nodes already running, i.e., the server.
             * 
             * @param nodeID the node.
             */
            virtual void setStarted( Node* node );

            /** 
             * Puts a node into started mode and specifies an existing
             * connection to the node.
             * 
             * @param node the node.
             * @param connection the open connection to the node.
             */
            virtual void setStarted( Node* node, Connection* connection );

            /** 
             * Sends a packet to a node using this network.
             * 
             * @param toNode the receiver node.
             * @param packet the packet.
             */
            void send( Node* toNode, const Packet& packet );

            virtual ~Network();

        protected:
            Network( const uint id, Session* session );

            /** The state of the individual nodes. */
            enum NodeState
            {
                NODE_STOPPED,
                NODE_INITIALIZED,
                NODE_LAUNCHED,
                NODE_RUNNING
            };

            /** The session for this network. */
            Session* _session;

            /** The current state of the network. */
            State    _state;

            /** The protocol of the network. */
            eqNet::NetworkProtocol _protocol;

            /** The list of connection descriptions, indexed per node. */
            eqBase::PtrHash<Node*, ConnectionDescription*> _descriptions;

            /** The list of node states. */
            eqBase::PtrHash<Node*, NodeState> _nodeStates;

            /** The set of active connections. */
            ConnectionSet _connectionSet;

            /** 
             * Creates the launch command for a node.
             * 
             * The returned string has to be freed by the caller.
             *
             * @param node the node.
             * @param args command line arguments for the launch command, may
             *             not be <code>NULL</code>
             * @return the launch command.
             */
            const char* _createLaunchCommand( Node* node, const char* args );

            friend inline std::ostream& operator << 
                (std::ostream& os, Network* network);
        };

        inline std::ostream& operator << ( std::ostream& os, Network* network )
        {
            os << "    Network " << network->getID() << "(" 
               << (void*)network <<  "): proto " << 
                ( network->_protocol == PROTO_TCPIP ? "TCP/IP" : 
                  network->_protocol == PROTO_PIPE  ? "pipe()" :
                  network->_protocol == PROTO_MPI   ? "MPI " : "unknown" )
               << network->_descriptions.size() << " node[s]" << std::endl;
            
            for( eqBase::PtrHash<Node*, ConnectionDescription*>::iterator iter= 
                     network->_descriptions.begin();
                 iter != network->_descriptions.end(); iter++ )
            {
                Node*                         node = (*iter).first;
                ConnectionDescription* description = (*iter).second;
                const Network::NodeState     state = network->_nodeStates[node];

                os << "    Node " << (void*)node << ": " << description << ", " 
                   << (state==Network::NODE_STOPPED     ? "stopped" : 
                       state==Network::NODE_INITIALIZED ? "initialized" :
                       state==Network::NODE_RUNNING     ? "running" : 
                       "unknown state") << std::endl;
            }

            return os;
        }
    }
}

#endif // EQNET_NETWORK_PRIV_H
