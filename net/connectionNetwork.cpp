
#include "connectionNetwork.h"
#include "connection.h"
#include "connectionListener.h"

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
    for( eqBase::PtrHash<Connection*, ConnectionListener*>::iterator iter = 
             _connectionSet.begin(); iter != _connectionSet.end(); iter++ )
    {
        Connection*         connection         = (*iter).first;
        ConnectionListener* connectionListener = (*iter).second;
        connection->close();
        delete connection;
        delete connectionListener;
    }
    _connectionSet.clear();

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

    ConnectionListener* listener = new ConnectionListener( this, nodeID );
    _connectionSet.addConnection( connection, listener );
    _nodeStates[nodeID]  = NODE_RUNNING;
}
