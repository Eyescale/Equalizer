
#include "dynamicNetwork.h"
#include "connection.h"

using namespace eqNet::priv;
using namespace eqBase;

bool DynamicNetwork::init()
{
    for( PtrHash<Node*, ConnectionDescription*>::iterator iter =
             _descriptions.begin(); iter != _descriptions.end(); iter++ )
    {
        Node* node = (*iter).first;
        if( _nodeStates[node] == NODE_STOPPED )
            _nodeStates[node] = NODE_INITIALIZED;
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

    for( PtrHash<Node*, ConnectionDescription*>::iterator iter =
             _descriptions.begin(); iter != _descriptions.end(); iter++ )
    {
        Node* node = (*iter).first;
        _nodeStates[node] = NODE_STOPPED;
    }
}
