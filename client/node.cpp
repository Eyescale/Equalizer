
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "pipe.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

static bool _firstNode = true;

Node::Node()
        : eqNet::Node( CMD_NODE_ALL ),
          _config(NULL),
          _clientLoopRunning(false)
{
    registerCommand( CMD_NODE_CREATE_CONFIG, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_cmdCreateConfig ));
    registerCommand( CMD_NODE_CREATE_PIPE, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_cmdCreatePipe ));
    registerCommand( CMD_NODE_DESTROY_PIPE, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_cmdDestroyPipe ));
    registerCommand( CMD_NODE_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_cmdInit ));
    registerCommand( REQ_NODE_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_reqInit ));
    registerCommand( CMD_NODE_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Node::_pushRequest ));
    registerCommand( REQ_NODE_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Node::_reqExit ));
    registerCommand( CMD_NODE_STOP, this, reinterpret_cast<CommandFcn>( 
                         &eq::Node::_pushRequest ));
    registerCommand( REQ_NODE_STOP, this, reinterpret_cast<CommandFcn>( 
                         &eq::Node::_reqStop ));
}

Node::~Node()
{
}

eqBase::RefPtr<eqNet::Node> Node::createNode()
{ 
    // The first node in a render node's life cycle is the server. There must be
    // a cleaner way to instantiate it...
    if( _firstNode ) 
    {
        _firstNode = false;
        _server    = new Server;
        return _server.get();
    }

    return Global::getNodeFactory()->createNode();
}

eqNet::Session* Node::createSession()
{
    return Global::getNodeFactory()->createConfig(); 
}


void Node::clientLoop()
{
    _clientLoopRunning = true;

    eqNet::Node*   node;
    eqNet::Packet* packet;

    while( _clientLoopRunning )
    {
        _requestQueue.pop( &node, &packet );

        switch( dispatchPacket( node, packet ))
        {
            case eqNet::COMMAND_HANDLED:
                break;

            case eqNet::COMMAND_ERROR:
                ERROR << "Error handling command packet" << endl;
                abort();

            case eqNet::COMMAND_RESCHEDULE:
                UNIMPLEMENTED;
        }
    }
}

eqNet::CommandResult Node::handlePacket( eqNet::Node* node,
                                         const eqNet::Packet* packet )
{
    VERB << "handlePacket " << packet << endl;
    const uint32_t datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQ_SERVER:
            ASSERT( node == _server.get( ));
            return _server->handleCommand( node, packet );
            break;

        default:
            return eqNet::COMMAND_ERROR;
    }
}

void Node::_addPipe( Pipe* pipe )
{
    _pipes.push_back( pipe );
    pipe->_node = this;
}

void Node::_removePipe( Pipe* pipe )
{
    vector<Pipe*>::iterator iter = find( _pipes.begin(), _pipes.end(), pipe );
    if( iter == _pipes.end( ))
        return;
    
    _pipes.erase( iter );
    pipe->_node = NULL;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Node::_cmdCreateConfig( eqNet::Node* node, 
                                             const eqNet::Packet* pkg )
{
    NodeCreateConfigPacket* packet = (NodeCreateConfigPacket*)pkg;
    INFO << "Handle create config " << packet << ", name " << packet->name 
         << endl;

    _config = Global::getNodeFactory()->createConfig();
    
    addSession( _config, node, packet->configID, packet->name );
    _server->addConfig( _config );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdCreatePipe( eqNet::Node* node, 
                                           const eqNet::Packet* pkg )
{
    NodeCreatePipePacket* packet = (NodeCreatePipePacket*)pkg;
    INFO << "Handle create pipe " << packet << endl;

    Pipe* pipe = Global::getNodeFactory()->createPipe();
    
    _config->addRegisteredObject( packet->pipeID, pipe );
    _addPipe( pipe );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdDestroyPipe( eqNet::Node* node, 
                                            const eqNet::Packet* pkg )
{
    NodeDestroyPipePacket* packet = (NodeDestroyPipePacket*)pkg;
    INFO << "Handle destroy pipe " << packet << endl;

    Pipe* pipe = (Pipe*)_config->getObject( packet->pipeID );
    if( !pipe )
        return eqNet::COMMAND_HANDLED;

    _removePipe( pipe );
    _config->deregisterObject( pipe );
    delete pipe;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdInit(eqNet::Node* node, const eqNet::Packet* pkg)
{
    NodeInitPacket* packet = (NodeInitPacket*)pkg;
    INFO << "handle node init (recv) " << packet << endl;

    _config->addRegisteredObject( packet->nodeID, this );
    _pushRequest( node, pkg );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqInit(eqNet::Node* node, const eqNet::Packet* pkg)
{
    NodeInitPacket* packet = (NodeInitPacket*)pkg;
    INFO << "handle node init (node) " << packet << endl;

    NodeInitReplyPacket reply( packet );
    reply.result = init();
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqExit(eqNet::Node* node, const eqNet::Packet* pkg)
{
    NodeExitPacket* packet = (NodeExitPacket*)pkg;
    INFO << "handle node exit " << packet << endl;

    exit();

    NodeExitReplyPacket reply( packet );
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqStop(eqNet::Node* node, const eqNet::Packet* pkg)
{
    NodeStopPacket* packet = (NodeStopPacket*)pkg;
    INFO << "handle node stop " << packet << endl;

    _clientLoopRunning = false;

    // stop ourselves
    eqNet::NodeStopPacket stopPacket;
    send( stopPacket );
    return eqNet::COMMAND_HANDLED;
}
