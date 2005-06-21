
#include "pipeNetwork.h"
#include "connection.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

bool PipeNetwork::init()
{
    return true; // do nothing, connections can and will be created dynamically
}

bool PipeNetwork::start()
{
    // create and connect pipe connections, will fork().
    INFO << "PipeNetwork starting " << _descriptions.size() << " nodes" << endl;

    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint nodeID = (*iter).first;
        if( !startNode( nodeID ))
        {
            stop();
            return false;
        }

        INFO << nodeID << endl;
    }
 
    return false;
}

bool PipeNetwork::startNode(const uint nodeID)
{
    INFO << "Starting node " << nodeID << endl;
    Connection *connection = Connection::create( eqNet::PROTO_PIPE );
    
    if( !connection->connect( *_descriptions[nodeID] ))
    {
        delete connection;
        return false;
    }
    
    return true;
}

void PipeNetwork::stop()
{
}
