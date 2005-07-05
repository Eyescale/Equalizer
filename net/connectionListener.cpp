
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionListener.h"
#include "connection.h"
#include "connectionNetwork.h"

using namespace eqNet::priv;

void ConnectionListener::notifyData(Connection* connection)
{
    if( connection->getState() == Connection::STATE_LISTENING )
    {
        Connection* newConnection = connection->accept();
        uint nodeID;
        const size_t received = newConnection->recv( &nodeID, sizeof( nodeID ));
        ASSERT( received == sizeof( nodeID ));
        ASSERT( nodeID );

        _network->setStarted( nodeID, newConnection );
    }
    else
        ; // TODO: read message
}

