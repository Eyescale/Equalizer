
#include "pipeNetwork.h"
#include "connection.h"
#include "connectionDescription.h"
#include "pipeConnection.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

bool PipeNetwork::start()
{
    if( _descriptions.size() != 2 )
    {
        WARN << "A pipe network has to have two nodes, but this one has " 
             << _descriptions.size() << " nodes" << endl;
        return false;
    }

    // create and connect pipe connections, will fork().
    INFO << "PipeNetwork starting " << _descriptions.size() << " nodes" << endl;

    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint                   nodeID      = (*iter).first;
        const ConnectionDescription* description = (*iter).second;
        const char*         entryFunc = description->parameters.PIPE.entryFunc;

        if( entryFunc == NULL ) // local node
            continue;

        INFO << "Starting node " << nodeID << endl;
        Connection *connection = Connection::create( eqNet::PROTO_PIPE );
    
        if( !connection->connect( *description ))
        {
            delete connection;
            return false;
        }

        setStarted( nodeID, connection );
    }
 
    return true;
}

bool PipeNetwork::startNode(const uint nodeID)
{
    WARN << "PipeNetwork does not support dynamic node starting" << endl;
    return false;
}

void PipeNetwork::stop()
{
}
