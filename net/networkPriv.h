
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NETWORK_PRIV_H
#define EQNET_NETWORK_PRIV_H

#include <eq/net/network.h>

#include "base.h"
#include "idHash.h"

namespace eqNet
{
    std::ostream& operator << ( std::ostream& os,
        ConnectionDescription* description);

    namespace priv
    {
        class Node;

        class Network : public eqNet::Network, public Base
        {
        public:
            /** 
             * Constructs a new Network.
             * 
             * @param id the identifier of the network.
             * @param protocol the network protocol.
             * @return the network.
             */
            static Network* create( const uint id,
                const eqNet::NetworkProtocol protocol );

            /**
             * Adds a node to this network.
             *
             * @param nodeID the node identifier.
             * @param connection the connection parameters.
             * @sa Node::enableForwarding(), Node::disableForwarding()
             */
            virtual void addNode( const uint nodeID, 
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
             * @param nodeID the node identifier.
             * @return <code>true</code> if the node was successfully
             *         started, <code>false</code> if not.
             * @sa start(), init()
             */
            virtual bool startNode(const uint nodeID) = 0;
            //@}

            virtual ~Network();

        protected:
            Network(const uint id);

            /** The identifier of this Network. */
            uint _id;

            /** The list of connection descriptions, indexed per node. */
            IDHash<ConnectionDescription*> _descriptions;

            friend inline std::ostream& operator << 
                (std::ostream& os, Network* network);
        };

        inline std::ostream& operator << ( std::ostream& os, Network* network )
        {
            os << "    Network " << network->getID() << "(" 
               << (void*)network <<  "): " << network->_descriptions.size()
               << " node[s] connected" << std::endl;
            
            for( IDHash<ConnectionDescription*>::iterator iter = 
                     network->_descriptions.begin();
                 iter != network->_descriptions.end(); iter++ )
            {
                const uint             nodeID      = (*iter).first;
                ConnectionDescription* description = (*iter).second;
                os << "    Node " << nodeID << ": " << description;
            }

            return os;
        }
    }
}

#endif // EQNET_NETWORK_PRIV_H
