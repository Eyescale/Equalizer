
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "networkPriv.h"

#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "network.h"
#include "nodePriv.h"
#include "packet.h"
#include "pipeNetwork.h"
#include "sessionPriv.h"
#include "socketNetwork.h"

#include <eq/base/log.h>
#include <alloca.h>

using namespace eqNet::priv;
using namespace eqBase;
using namespace std;

Network::Network( const uint id, Session* session )
        : eqNet::Network(id),
          _session(session),
          _state(STATE_STOPPED)
{
    for( int i=0; i<CMD_NETWORK_ALL; i++ )
        _cmdHandler[i] = &eqNet::priv::Network::_handleUnknown;

    _cmdHandler[CMD_NETWORK_ADD_NODE] = &eqNet::priv::Network::_handleNetworkAddNode;

    INFO << "New network" << this;
}

Network::~Network()
{
    // TODO: ConnectionDescription cleanup
}

Network* Network::create( const uint id, Session* session, 
    const eqNet::NetworkProtocol protocol )
{
    Network* network;
    switch( protocol )
    {
        case eqNet::PROTO_TCPIP:
            network = new SocketNetwork( id, session );
            break;

        case eqNet::PROTO_PIPE:
            network = new PipeNetwork( id, session );
            break;

        default:
            WARN << "Protocol " << protocol << " not implemented" << endl;
            return NULL;
    }
    network->_protocol = protocol;
    return network;
}

void Network::addNode( Node* node,
                       const eqNet::ConnectionDescription& description )
{
    ConnectionDescription *desc = new ConnectionDescription();
    *desc = description;

    if( typeid(this) == typeid( PipeNetwork* ))
    {
        if( description.parameters.PIPE.entryFunc != NULL)
            desc->parameters.PIPE.entryFunc =
                strdup( description.parameters.PIPE.entryFunc );
    }
   
    _descriptions[node] = desc;
    _nodeStates[node]   = NODE_STOPPED;
    node->addNetwork( this );
}

void Network::setStarted( Node* node )
{
    ASSERT( _descriptions.count(node)==1 );

    _nodeStates[node]  = NODE_RUNNING;
}

void Network::setStarted( Node* node, Connection* connection )
{
    ASSERT( _descriptions.count(node)==1 );
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );

    _connectionSet.addConnection( connection, this, node );
    _nodeStates[node]  = NODE_RUNNING;
}

const char* Network::_createLaunchCommand( Node* node, const char* args )
{
    PtrHash<Node*, ConnectionDescription*>::iterator iter = 
        _descriptions.find(node);

    if( iter == _descriptions.end() )
        return NULL;

    ConnectionDescription* description = (*iter).second;
    const char*          launchCommand = description->launchCommand;

    if( !launchCommand )
        return NULL;

    const size_t      launchCommandLen = strlen( launchCommand );
    size_t                  resultSize = 256;
    char*                       result = (char*)alloca(resultSize);
    size_t                 resultIndex = 0;
    bool                  commandFound = false;

    for( size_t i=0; i<launchCommandLen-1; i++ )
    {
        if( launchCommand[i] == '%' )
        {
             char* replacement = NULL;
            switch( launchCommand[i+1] )
            {
                case 'c':
                {
                    const char* programName = Global::getProgramName();
                    replacement  = (char*)alloca( strlen(programName) +
                                                  strlen( args ) + 2 );
                    sprintf( replacement, "%s %s", programName, args );
                    commandFound = true;
                } break;
            }

            if( replacement )
            {
                // check string length
                const size_t replacementLen = strlen( replacement );
                size_t       newSize        = resultSize;
                
                while( newSize <= resultIndex + replacementLen )
                    newSize = newSize << 1;
                if( newSize > resultSize )
                {
                    char* newResult = (char*)alloca(newSize);
                    memcpy( newResult, result, resultSize );
                    result     = newResult;
                    resultSize = newSize;
                }

                // replace
                memcpy( &result[resultIndex], replacement, replacementLen );
                resultIndex += replacementLen;
                i++;
            }
        }
        else
        {
            result[resultIndex++] = launchCommand[i];
            if( resultIndex == resultSize ) // check string length
            {
                resultSize      = resultSize << 1;
                char* newResult = (char*)alloca(resultSize);
                memcpy( newResult, result, resultSize >> 1 );
                result = newResult;
            }
        }
    }

    if( !commandFound )
    { 
        // check string length
        const char* programName = Global::getProgramName();
        char*       command     = (char*)alloca( strlen(programName) +
                                                 strlen( args ) + 2 );
        sprintf( command, "%s %s", programName, args );
        const size_t commandLen = strlen( command );
        size_t       newSize    = resultSize;
                
        while( newSize <= resultIndex + commandLen )
            newSize = newSize << 1;

        if( newSize > resultSize )
        {
            char* newResult = (char*)alloca(newSize);
            memcpy( newResult, result, resultSize );
            result     = newResult;
            resultSize = newSize;
        }

        // append command
        memcpy( &result[resultIndex], command, commandLen );
        resultIndex += commandLen;
   }

    result[resultIndex] = '\0';
    INFO << "Launch command: " << result << endl;
    return strdup(result);
}

void Network::handlePacket( Connection* connection, 
                            const NetworkPacket* packet )
{
    INFO << "handle " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_NETWORK:
            ASSERT( packet->command < CMD_NETWORK_ALL );
            (this->*_cmdHandler[packet->command])( connection, packet );
            break;

        default:
            WARN << "Unhandled packet " << packet << endl;
    }
}

void Network::_handleNetworkAddNode( Connection* connection, const Packet* pkg )
{
    INFO << "Handle network add node" << endl;
    const NetworkAddNodePacket* packet  = (NetworkAddNodePacket*)pkg;
    
    Node* node = _session->getNodeByID( packet->nodeID );
    ASSERT( node );

    addNode( node, packet->connectionDescription );
    _nodeStates[node] = packet->nodeState;
}

void Network::send( Node* toNode, const Packet& packet )
{
    ASSERT( _nodeStates[toNode] == NODE_RUNNING );
    
    Connection* connection = _connectionSet.getConnection( toNode );
    
    if( !connection )
    {
        connection = Connection::create( _protocol );
        ConnectionDescription* desc = _descriptions[toNode];
        ASSERT( desc );

        const bool connected = connection->connect( *desc );
        if( !connected )
        {
            ERROR << "Can't connect to node " << toNode << endl;
            delete connection;
            return;
        }
        _connectionSet.addConnection( connection, this, toNode );
    }

    INFO << "Sending packet " << &packet << "using connection " << connection
         << endl;
    connection->send( &packet, packet.size );
}
// {
//         NetworkNewPacket networkNewPacket;
//         networkNewPacket.id = networkID;
//         networkNewPacket.sessionID = sessionID;
//         networkNewPacket.state = network->getState();
//         networkNewPacket.protocol = network->getProtocol();
//         send( node, networkNewPacket );

//         NetworkAddNodePacket networkAddNodePacket;
//         networkAddNodePacket.id = networkID;
//         networkAddNodePacket.nodeID = ;
//         networkAddNodePacket.connectionDescription;
//         networkAddNodePacket.nodeState;
//     }
