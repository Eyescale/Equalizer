
#include "socketNetwork.h"
#include "connection.h"
#include "connectionDescription.h"
#include "launcher.h"
#include "sessionPriv.h"

#include <eq/base/log.h>
#include <eq/base/thread.h>

using namespace eqNet::priv;
using namespace std;

SocketNetwork::SocketNetwork( const uint id, Session* session )
        : ConnectionNetwork( id, session ),
          _listener(NULL),
          _receiver(NULL)
{
}

SocketNetwork::~SocketNetwork()
{
    stop();
    exit();

    if( _receiver )
    {
        _receiver->join();
        delete _receiver;
    }
}

bool SocketNetwork::start()
{
    INFO << "SocketNetwork starting " << _descriptions.size() << " nodes" 
         << endl;

    if( !_startReceiver())
    {
        WARN << "Failed to start receiver" << endl;
        return false;
    }
    
    if( !_launchNodes( ))
    {
        stop();
        return false;
    }
    
}

//----------------------------------------------------------------------
// receiver thread initialisation
//----------------------------------------------------------------------

class ReceiverThread : public eqBase::Thread
{
public:
    ReceiverThread( SocketNetwork* parent )
            : eqBase::Thread( Thread::PTHREAD ),
              _parent( parent )
        {}

    virtual ssize_t run() { return _parent->runReceiver(); }

private:
    SocketNetwork* _parent;
};

bool SocketNetwork::_startReceiver()
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
    {
        //TODO: _connections.erase( _listener );
        delete _listener;
    }

    _listener = listener;
    _connections.push_back( listener );

    _receiver = new ReceiverThread( this );
    _state    = STATE_STARTING;

    if( !_receiver->start( ))
    {
        WARN << "Could not start receiver thread" << endl;
        return false;
    }

    _nodeStates[localNodeID] = NODE_RUNNING;
    return true;
}

//----------------------------------------------------------------------
// receiver thread
//----------------------------------------------------------------------
ssize_t SocketNetwork::runReceiver()
{
    if( _state != STATE_STARTING )
        return EXIT_FAILURE;

    if( !_connectNodes( ))
        return EXIT_FAILURE;

    _state = STATE_RUNNING;
    // TODO: unlock parent

    while( true )
    {
        
         short event;
         Connection *connection = Connection::select( _connections, -1, event );

         switch( event )
         {
             case 0:
                 WARN << "Got timeout during connection selection?" << endl;
                 break;
                 
             case POLLERR:
                 WARN << "Got error during connection selection" << endl;
                 break;
                 
             case POLLIN:
             case POLLPRI: // data is ready for reading
                 WARN << "Unhandled: incoming data" << endl;
                 //_handleRequest( connection );
                 break;
                 
             case POLLHUP: // disconnect happened
                 WARN << "Unhandled: Connection disconnect" << endl;
                 break;
                 
             case POLLNVAL: // disconnected connection
                 WARN << "Unhandled: Disconnected connection" << endl;
                 break;
         }
         break;
    }
    return EXIT_SUCCESS;
}


bool SocketNetwork::_connectNodes()
{
    IDHash<ConnectionDescription*> launchedNodes;

    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint nodeID = (*iter).first;
        
        if( _nodeStates[nodeID] == NODE_LAUNCHED )
        {
            ConnectionDescription* description = (*iter).second;
            launchedNodes[nodeID] = description;
        }
    }

    while( launchedNodes.size() > 0 )
    {
        Connection* connection = _listener->accept( STARTUP_TIMEOUT );
        if( connection == NULL )
        {
            WARN << "Could not connect all nodes" << endl;
            return false;
        }

        
        // TODO: find node in launchedNodes, remove and start it.
    }

    return true;
}

//----------------------------------------------------------------------
// node startup
//----------------------------------------------------------------------

bool SocketNetwork::_launchNodes()
{
    for( IDHash<ConnectionDescription*>::iterator iter = _descriptions.begin();
         iter != _descriptions.end(); iter++ )
    {
        const uint nodeID = (*iter).first;
        
        if( _nodeStates[nodeID] == NODE_INITIALIZED )
        {
            const ConnectionDescription* description = (*iter).second;
            INFO << "Launching node " << nodeID << endl;
    
            if( !_launchNode( nodeID, description ))
            {
                WARN << "Could not launch node " << nodeID << endl;
                return false;
            }
        }
    }
    return true;
}

bool SocketNetwork::_launchNode( const uint nodeID,
    const ConnectionDescription* description )
{
    const char*                address = description->parameters.TCPIP.address;
    const uint             localNodeID = _session->getLocalNodeID();
    const ConnectionDescription* localDesc = _descriptions[localNodeID];
    const char*           localAddress = localDesc->parameters.TCPIP.address;

    char*       options       = (char*)alloca( strlen(localAddress) + 128 );
    sprintf( options, "--eq-server %s --eq-nodeID %d", localAddress, nodeID );

    const char* launchCommand = _createLaunchCommand( nodeID, options );
    
    if( launchCommand )
        Launcher::run( launchCommand );

    _nodeStates[nodeID] = NODE_LAUNCHED;
    return true;
}

bool SocketNetwork::startNode(const uint nodeID)
{
    // TODO
    return false;
}
void SocketNetwork::stop()
{
}

