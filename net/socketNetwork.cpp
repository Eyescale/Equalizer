
#include "socketNetwork.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

bool SocketNetwork::start()
{
    INFO << "SocketNetwork starting " << _descriptions.size() << " nodes" 
         << endl;

    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint                   nodeID      = (*iter).first;
        const ConnectionDescription* description = (*iter).second;
    }

    return false;
}

bool SocketNetwork::startNode(const uint nodeID)
{
    return false;
}

void SocketNetwork::stop()
{
}
