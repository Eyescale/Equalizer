
#include "connectionNetwork.h"
#include "connection.h"

using namespace eqNet::priv;

bool ConnectionNetwork::init()
{
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
}


void ConnectionNetwork::setStarted( const uint nodeID, Connection* connection )
{
    ASSERT( _descriptions.count(nodeID)!=0 );
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );

    _connections[nodeID] = connection;
}
