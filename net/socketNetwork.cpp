
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "socketNetwork.h"
#include "connection.h"
#include "connectionDescription.h"
#include "launcher.h"
#include "sessionPriv.h"
#include "socketConnection.h"

#include <eq/base/base.h>
#include <eq/base/log.h>
#include <eq/base/thread.h>
#include <alloca.h>

using namespace eqNet::priv;
using namespace std;

SocketNetwork::SocketNetwork( const uint id, Session* session )
        : DynamicNetwork( id, session ),
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

    if( !_startListener( ))
        return false;

    if( !_startNodes( ))
    {
        _stopListener();
        return false;
    }

    if( !_startReceiver())
    {
        _stopNodes();
        _stopListener();
        return false;
    }
    return true;
}

bool SocketNetwork::_startListener()
{
    Node*                  localNode   = _session->getLocalNode();
    const uint             localNodeID = localNode->getID();
    ConnectionDescription* localDesc   = _getConnectionDescription( localNodeID );

    if( !localDesc )
    {
        WARN << "Local node is not part of this network" << endl;
        return false;
    }

    _listener = Connection::create( PROTO_TCPIP );

    if( !_listener->listen( *localDesc ))
    {
        WARN << "Could not open listener on " \
             << localDesc->parameters.TCPIP.hostname << ":" 
             << localDesc->parameters.TCPIP.port << endl;
        delete _listener;
        _listener = NULL;
        return false;
    }

    _connectionSet.addConnection( _listener, this, localNode );
    _nodeStates[localNodeID] = NODE_RUNNING;

    // TODO: Use 'address' if specified in localDesc?
    gethostname( localDesc->parameters.TCPIP.hostname, MAXHOSTNAMELEN+1 );
    localDesc->parameters.TCPIP.port =
        static_cast<SocketConnection*>(_listener)->getPort();

    INFO << "Listening on: " << localDesc->parameters.TCPIP.hostname << ":"
         << localDesc->parameters.TCPIP.port << endl;
    return true;
}

void SocketNetwork::_stopListener()
{
    if( !_listener )
        return;

    _connectionSet.removeConnection( _listener );
    _listener->close();
    delete _listener;
    _listener = NULL;

    const uint localNodeID   = _session->getLocalNodeID();
    _nodeStates[localNodeID] = NODE_STOPPED;
}

//----------------------------------------------------------------------
// receiver thread initialisation
//----------------------------------------------------------------------

class ReceiverThread : public eqBase::Thread
{
public:
    ReceiverThread( SocketNetwork* parent )
            : eqBase::Thread( eqBase::Thread::PTHREAD ),
              _parent( parent )
        {}

    virtual ssize_t run() { return _parent->runReceiver(); }

private:
    SocketNetwork* _parent;
};

bool SocketNetwork::_startReceiver()
{
    _receiver = new ReceiverThread( this );
    _state    = STATE_STARTING;

    if( !_receiver->start( ))
    {
        WARN << "Could not start receiver thread" << endl;
        return false;
    }
    
    const uint   localNodeID = _session->getLocalNodeID();
    _nodeStates[localNodeID] = NODE_RUNNING;
    return true;
}

void  SocketNetwork::_stopReceiver()
{
    // TODO
}

//----------------------------------------------------------------------
// receiver thread
//----------------------------------------------------------------------
ssize_t SocketNetwork::runReceiver()
{
    INFO << "Receiver running" << endl;
    if( _state != STATE_STARTING )
        return EXIT_FAILURE;

    _state = STATE_RUNNING;
    // TODO: unlock parent ?

    while( _state == STATE_RUNNING )
    {
         short event = _connectionSet.select( -1 );

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


//----------------------------------------------------------------------
// node startup
//----------------------------------------------------------------------

bool SocketNetwork::_startNodes()
{
    if( !_launchNodes( ))
        return false;
    if( !_connectNodes( ))
        return false;

    INFO << "All nodes started" << endl;
    return true;
}
    
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
    const uint                 localNodeID = _session->getLocalNodeID();
    const ConnectionDescription* localDesc = _descriptions[localNodeID];
    const char*              hostname = localDesc->parameters.TCPIP.hostname;
    const ushort             port     = localDesc->parameters.TCPIP.port;

    char* options = (char*)alloca( strlen(hostname) + 128 );
    sprintf( options, "--eq-server %s --eq-port %d --eq-nodeID %d", 
             hostname, port, nodeID );

    const char* launchCommand = _createLaunchCommand( nodeID, options );
    
    if( launchCommand )
        Launcher::run( launchCommand );

    _nodeStates[nodeID] = NODE_LAUNCHED;

    INFO << "All nodes launched" << endl;
    return true;
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

        uint nodeID;
        const size_t received = connection->recv( &nodeID, sizeof( nodeID ));
        ASSERT( received == sizeof( nodeID ));
        ASSERT( nodeID );
        ASSERT( launchedNodes.find( nodeID ) != launchedNodes.end() );

        setStarted( nodeID, connection );
        launchedNodes.erase( nodeID );
    }

    INFO << "All nodes connected" << endl;
    return true;
}

void SocketNetwork::_stopNodes()
{
    // TODO
}




bool SocketNetwork::startNode(const uint nodeID)
{
    // TODO
    return false;
}

bool SocketNetwork::connect( const uint nodeID )
{
    if( _nodeStates[nodeID] != NODE_RUNNING )
        return false;

    ConnectionDescription* description = _getConnectionDescription( nodeID );
    if( !description )
        return false;

    Connection* connection = Connection::create( PROTO_TCPIP );
    if( !connection->connect( *description ))
        return false;

}

void SocketNetwork::stop()
{
    INFO << "SocketNetwork stopping" << endl;

    _stopReceiver();
    _stopNodes();
    _stopListener();
}

