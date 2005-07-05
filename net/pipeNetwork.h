
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_NETWORK_H
#define EQNET_PIPE_NETWORK_H

#include "connectionNetwork.h"

#include <iostream> 

namespace eqNet
{
    namespace priv
    {
        class Connection;

        /** A 'network' of two nodes connected using a pipe. */
        class PipeNetwork : public ConnectionNetwork
        {
        public:
            PipeNetwork( const uint id, Session* session ) :
                    ConnectionNetwork( id, session ) {}

            virtual bool start();
            virtual void stop();
            virtual bool startNode(const uint nodeID);
            virtual bool connect( const uint nodeID )
                { return ( _connections.find(nodeID) != _connections.end( )); }

        private:

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
