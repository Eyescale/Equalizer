
#include "connectionNetwork.h"
#include "connection.h"

using namespace eqNet::priv;

bool ConnectionNetwork::init()
{
    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint nodeID = (*iter).first;
        if( _nodeStates[nodeID] == NODE_STOPPED )
            _nodeStates[nodeID] = NODE_INITIALIZED;
    }

    return true; // do nothing, connections can and will be created dynamically
}

void ConnectionNetwork::exit()
{
    for( IDHash<Connection*>::iterator iter = _connections.begin();
         iter != _connections.end(); iter++ )
    {
        const uint  nodeID     = (*iter).first;
        Connection* connection = (*iter).second;
        connection->close();
        delete connection;
    }
    _connections.clear();

    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint nodeID = (*iter).first;
        _nodeStates[nodeID] = NODE_STOPPED;
    }
}

void ConnectionNetwork::setStarted( const uint nodeID, Connection* connection )
{
    ASSERT( _descriptions.count(nodeID)!=0 );
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );

    _connections[nodeID] = connection;
    _nodeStates[nodeID]  = NODE_RUNNING;
}
