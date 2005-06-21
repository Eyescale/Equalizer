
#include "socketNetwork.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

bool SocketNetwork::init()
{
    return true; // do nothing, connections can and will be created dynamically
}

bool SocketNetwork::start()
{
    // create and connect socket connections, will fork().
    INFO << "SocketNetwork starting " << _descriptions.size() << " nodes" 
         << endl;

//    Sgi::hash_map<uint, ConnectionDescription*>::iterator iter =
//        _descriptions.begin();
    return false;
}

bool SocketNetwork::startNode(const uint nodeID)
{
}

void SocketNetwork::stop()
{
}
