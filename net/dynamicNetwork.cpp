
#include "dynamicNetwork.h"
#include "connection.h"

using namespace eqNet::priv;

bool DynamicNetwork::init()
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

void DynamicNetwork::exit()
{
    for( eqBase::PtrHash<Node*, Connection*>::iterator iter = 
             _connectionSet.begin(); iter != _connectionSet.end(); iter++ )
    {
        Connection* connection = (*iter).second;
        connection->close();
        delete connection;
    }
    _connectionSet.clear();

    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint nodeID = (*iter).first;
        _nodeStates[nodeID] = NODE_STOPPED;
    }
}
