
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "client.h"
#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "object.h"
#include "packets.h"
#include "pipe.h"
#include "server.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Node::Node()
        : eqNet::Object( eq::Object::TYPE_NODE ),
          _config(NULL)
{
    registerCommand( CMD_NODE_CREATE_PIPE, 
                     eqNet::PacketFunc<Node>( this, &Node::_cmdCreatePipe ));
    registerCommand( CMD_NODE_DESTROY_PIPE,
                    eqNet::PacketFunc<Node>( this, &Node::_cmdDestroyPipe ));
    registerCommand( CMD_NODE_INIT, 
                     eqNet::PacketFunc<Node>( this, &Node::_cmdInit ));
    registerCommand( REQ_NODE_INIT,
                     eqNet::PacketFunc<Node>( this, &Node::_reqInit ));
    registerCommand( CMD_NODE_EXIT,
                     eqNet::PacketFunc<Node>( this, &Node::_pushCommand ));
    registerCommand( REQ_NODE_EXIT,
                     eqNet::PacketFunc<Node>( this, &Node::_reqExit ));

    _thread = new NodeThread( this );
    EQINFO << " New eq::Node @" << (void*)this << endl;
}

Node::~Node()
{
    EQINFO << " Delete eq::Node @" << (void*)this << endl;
    delete _thread;
    _thread = NULL;
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

void* Node::_runThread()
{
    EQINFO << "Entered node thread" << endl;

    Config* config = getConfig();
    EQASSERT( config );

    eqNet::Node::setLocalNode( config->getLocalNode( ));

    while( _thread->isRunning( ))
    {
        eqNet::Command* command = _commandQueue.pop();
        switch( config->dispatchCommand( *command ))
        {
            case eqNet::COMMAND_HANDLED:
            case eqNet::COMMAND_DISCARD:
                break;

            case eqNet::COMMAND_ERROR:
                EQERROR << "Error handling command packet" << endl;
                abort();

            case eqNet::COMMAND_PUSH:
                EQUNIMPLEMENTED;
            case eqNet::COMMAND_PUSH_FRONT:
                EQUNIMPLEMENTED;
            case eqNet::COMMAND_REDISPATCH:
                EQUNIMPLEMENTED;
        }
    }

    return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Node::_cmdCreatePipe( eqNet::Command& command )
{
    const NodeCreatePipePacket* packet = 
        command.getPacket<NodeCreatePipePacket>();
    EQINFO << "Handle create pipe " << packet << endl;
    EQASSERT( packet->pipeID != EQ_ID_INVALID );

    Pipe* pipe = Global::getNodeFactory()->createPipe();
    
    _config->addRegisteredObject( packet->pipeID, pipe, 
                                   eqNet::Object::SHARE_NODE );
    _addPipe( pipe );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdDestroyPipe( eqNet::Command& command )
{
    const NodeDestroyPipePacket* packet = 
        command.getPacket<NodeDestroyPipePacket>();
    EQINFO << "Handle destroy pipe " << packet << endl;

    Pipe* pipe = (Pipe*)_config->pollObject( packet->pipeID );
    if( !pipe )
        return eqNet::COMMAND_HANDLED;

    pipe->_thread->join(); // wait for pipe thread termination. move to pipe

    _removePipe( pipe );
    EQASSERT( pipe->getRefCount() == 1 );
    _config->removeRegisteredObject( pipe, eqNet::Object::SHARE_NODE );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdInit( eqNet::Command& command )
{
    EQINFO << "handle node init (recv) " << command.getPacket<NodeInitPacket>()
           << endl;

    ((eq::Client*)_config->getLocalNode().get())->refUsed();
    _thread->start();
    _pushCommand( command );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqInit( eqNet::Command& command )
{
    const NodeInitPacket* packet = command.getPacket<NodeInitPacket>();
    EQINFO << "handle node init (node) " << packet << endl;

    NodeInitReplyPacket reply( packet );
    reply.result = init( packet->initID );

    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqExit( eqNet::Command& command )
{
    const NodeExitPacket* packet = command.getPacket<NodeExitPacket>();
    EQINFO << "handle node exit " << packet << endl;

    exit();

    NodeExitReplyPacket reply( packet );
    send( command.getNode(), reply );

    ((eq::Client*)_config->getLocalNode().get())->unrefUsed();
    _thread->exit();
    return eqNet::COMMAND_HANDLED;
}

