
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_NETWORK_H
#define EQNET_PIPE_NETWORK_H

#include "networkPriv.h"

#include <iostream> 

namespace eqNet
{
    namespace priv
    {
        class Connection;
        class PipeConnection;

        /** A 'network' of two nodes connected using a pipe. */
        class PipeNetwork : public Network
        {
        public:
            PipeNetwork(const uint id) : Network(id){}

            virtual bool init();
            virtual bool start() ;
            virtual void stop();
            virtual bool startNode(const uint nodeID);

            /** 
             * Forces a node into started mode, used during server launch
             * 
             * @param nodeID the node identifier.
             * @param connection the open connection to the node.
             */
            void setStarted( const uint nodeID, PipeConnection* connection );

        private:
            /** The list of active connections, indexed per node. */
            IDHash<Connection*> _connections;
            friend inline std::ostream& operator << 
                (std::ostream& os, PipeNetwork* network);
        };

        inline std::ostream& operator << (std::ostream& os,PipeNetwork* network)
        {
            os << "    PipeNetwork " << network->getID() << "(" 
               << (void*)network << "): " << network->_descriptions.size() 
               << " node[s] connected, " << network->_connections.size() 
               << " connection[s] active" << std::endl;
            
            for( IDHash<ConnectionDescription*>::iterator iter =
                     network->_descriptions.begin();
                 iter != network->_descriptions.end(); iter++ )
            {
                const uint             nodeID      = (*iter).first;
                ConnectionDescription* description = (*iter).second;
                os << "Node " << nodeID << ": " << description;
            }

            for( IDHash<Connection*>::iterator iter =
                     network->_connections.begin();
                 iter != network->_connections.end(); iter++ )
            {
                const uint  nodeID     = (*iter).first;
                Connection* connection = (*iter).second;
                os << "Node " << nodeID << ": " << connection;
            }

            return os;
        }
    }
}

#endif //EQNET_PIPE_NETWORK_H
