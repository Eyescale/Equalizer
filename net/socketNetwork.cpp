
#include "socketNetwork.h"
#include "connection.h"
#include "connectionDescription.h"
#include "launcher.h"
#include "sessionPriv.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

SocketNetwork::SocketNetwork( const uint id, Session* session )
        : ConnectionNetwork( id, session ),
          _listener(NULL)
{
}

bool SocketNetwork::start()
{
    INFO << "SocketNetwork starting " << _descriptions.size() << " nodes" 
         << endl;

    if( !_startListening())
    {
        WARN << "Failed to open listener" << endl;
        return false;
    }
    
    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint nodeID = (*iter).first;

        if( !startNode( nodeID ))
        {
            WARN << "Could not start node " << nodeID << endl;
            stop();
            return false;
        }
    }
    return true;
}

bool SocketNetwork::_startListening()
{
    const uint localNodeID = _session->getLocalNodeID();

    IDHash<ConnectionDescription*>::iterator iter = 
        _descriptions.find( localNodeID );

    if( iter == _descriptions.end() )
    {
        WARN << "Could not find local node " << localNodeID
             << " in this network" << endl;
        return false;
    }

    ConnectionDescription* localDesc = (*iter).second;
    const char*            address   = localDesc->parameters.TCPIP.address;

    INFO << "Found local node (" << localNodeID << ") in network, address "
         << (address ? address : "'null'") << endl;

    Connection* listener = Connection::create( eqNet::PROTO_TCPIP );
    
    if( !listener->listen( *localDesc ))
    {
        WARN << "Could not open listener on " << (address ? address : "'null'")
             << endl;
        delete listener;
        return false;
    }

    if( _listener )
        delete _listener;

    _listener = listener;
    return true;
}

bool SocketNetwork::startNode(const uint nodeID)
{
    INFO << "Starting node " << nodeID << endl;

    IDHash<ConnectionDescription*>::iterator iter = _descriptions.find( nodeID);

    if( iter == _descriptions.end() )
    {
        WARN << "Could not find node " << nodeID << " in this network" << endl;
        return false;
    }

    ConnectionDescription* description = (*iter).second;
    const char*                address = description->parameters.TCPIP.address;
    const char*          launchCommand = _createLaunchCommand( nodeID );
    
    if( launchCommand )
        Launcher::run( launchCommand );

    return false;
}

void SocketNetwork::stop()
{
}
