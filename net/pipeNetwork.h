
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_NETWORK_H
#define EQNET_PIPE_NETWORK_H

#include "connectionNetwork.h"
#include "connectionListener.h"

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
                { return ( _nodeStates.find(nodeID) != _nodeStates.end( )); }

        private:

            friend inline std::ostream& operator << 
                (std::ostream& os, PipeNetwork* network);
        };

        inline std::ostream& operator << (std::ostream& os,PipeNetwork* network)
        {
            os << "    PipeNetwork " << network->getID() << "(" 
               << (void*)network << "): " << network->_descriptions.size() 
               << " node[s] connected, " << network->_connectionSet.size() 
               << " connection[s] active" << std::endl;
            
            for( IDHash<ConnectionDescription*>::iterator iter =
                     network->_descriptions.begin();
                 iter != network->_descriptions.end(); iter++ )
            {
                const uint             nodeID      = (*iter).first;
                ConnectionDescription* description = (*iter).second;
                os << "Node " << nodeID << ": " << description;
            }

            for( eqBase::PtrHash<Connection*, ConnectionListener*>::iterator
                     iter = network->_connectionSet.begin(); 
                 iter != network->_connectionSet.end(); iter++ )
            {
                Connection*         connection         = (*iter).first;
                ConnectionListener* connectionListener = (*iter).second;

                os << "Node " << connectionListener->getNodeID() << ": " 
                   << connection;
            }

            return os;
        }
    }
}

#endif //EQNET_PIPE_NETWORK_H
