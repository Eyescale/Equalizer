
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "object.h"
#include "packets.h"
#include "pipe.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

Node::Node()
        : eqNet::Object( eq::Object::TYPE_NODE, CMD_NODE_ALL ),
          _config(NULL)
{
    registerCommand( CMD_NODE_CREATE_PIPE, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_cmdCreatePipe ));
    registerCommand( CMD_NODE_DESTROY_PIPE, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_cmdDestroyPipe ));
    registerCommand( CMD_NODE_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_pushRequest ));
    registerCommand( REQ_NODE_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Node::_reqInit ));
    registerCommand( CMD_NODE_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Node::_pushRequest ));
    registerCommand( REQ_NODE_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Node::_reqExit ));
}

Node::~Node()
{
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
eqNet::CommandResult Node::_cmdCreatePipe( eqNet::Node* node, 
                                           const eqNet::Packet* pkg )
{
    NodeCreatePipePacket* packet = (NodeCreatePipePacket*)pkg;
    EQINFO << "Handle create pipe " << packet << endl;
    EQASSERT( packet->pipeID != EQ_INVALID_ID );

    Pipe* pipe = Global::getNodeFactory()->createPipe();
    
    _config->_addRegisteredObject( packet->pipeID, pipe );
    _addPipe( pipe );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdDestroyPipe( eqNet::Node* node, 
                                            const eqNet::Packet* pkg )
{
    NodeDestroyPipePacket* packet = (NodeDestroyPipePacket*)pkg;
    EQINFO << "Handle destroy pipe " << packet << endl;

    Pipe* pipe = (Pipe*)_config->pollObject( packet->pipeID );
    if( !pipe )
        return eqNet::COMMAND_HANDLED;

    _removePipe( pipe );
    _config->deregisterObject( pipe );
    delete pipe;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqInit(eqNet::Node* node, const eqNet::Packet* pkg)
{
    NodeInitPacket* packet = (NodeInitPacket*)pkg;
    EQINFO << "handle node init (node) " << packet << endl;

    NodeInitReplyPacket reply( packet );
    reply.result = init( packet->initID );
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqExit(eqNet::Node* node, const eqNet::Packet* pkg)
{
    NodeExitPacket* packet = (NodeExitPacket*)pkg;
    EQINFO << "handle node exit " << packet << endl;

    exit();

    NodeExitReplyPacket reply( packet );
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

