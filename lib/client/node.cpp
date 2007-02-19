
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "client.h"
#include "commands.h"
#include "frameData.h"
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
        : _config(NULL)
{
    registerCommand( CMD_NODE_CREATE_PIPE, 
                     eqNet::CommandFunc<Node>( this, &Node::_cmdCreatePipe ));
    registerCommand( CMD_NODE_DESTROY_PIPE,
                    eqNet::CommandFunc<Node>( this, &Node::_cmdDestroyPipe ));
    registerCommand( CMD_NODE_INIT, 
                     eqNet::CommandFunc<Node>( this, &Node::_cmdInit ));
    registerCommand( REQ_NODE_INIT,
                     eqNet::CommandFunc<Node>( this, &Node::_reqInit ));
    registerCommand( CMD_NODE_EXIT,
                     eqNet::CommandFunc<Node>( this, &Node::_pushCommand ));
    registerCommand( REQ_NODE_EXIT,
                     eqNet::CommandFunc<Node>( this, &Node::_reqExit ));

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
    EQASSERT( iter != _pipes.end( ))
    
    _pipes.erase( iter );
    pipe->_node = NULL;
}

Pipe* Node::_findPipe( const uint32_t id )
{
    for( vector<Pipe*>::const_iterator i = _pipes.begin(); i != _pipes.end(); 
         ++i )
    {
        Pipe* pipe = *i;
        if( pipe->getID() == id )
            return pipe;
    }
    return 0;
}

eqNet::Barrier* Node::getBarrier( const uint32_t id, const uint32_t version )
{
    _barriersMutex.set();
    eqNet::Barrier* barrier = _barriers[ id ];

    if( !barrier )
    {
        eqNet::Session* session = getSession();

        barrier = new eqNet::Barrier;
        barrier->makeThreadSafe();
        const bool mapped = session->mapObject( barrier, id );
        EQASSERT( mapped );

        _barriers[ id ] = barrier;
    }
    _barriersMutex.unset();

    barrier->sync( version );
    return barrier;
}

FrameData* Node::getFrameData( const uint32_t id, const uint32_t version )
{
    _frameDatasMutex.set();
    FrameData* frameData = _frameDatas[ id ];

    if( !frameData )
    {
        eqNet::Session* session = getSession();
        
        frameData = new FrameData;
        frameData->makeThreadSafe();
        const bool mapped = session->mapObject( frameData, id );
        EQASSERT( mapped );

        _frameDatas[ id ] = frameData;
    }
    _frameDatasMutex.unset();

    frameData->sync( version );
    return frameData;
}

void Node::_flushObjects()
{
    eqNet::Session* session = getSession();

    _barriersMutex.set();
    for( eqNet::IDHash< eqNet::Barrier* >::const_iterator i =_barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        eqNet::Barrier* barrier = i->second;
        session->unmapObject( barrier );
        delete barrier;
    }
    _barriers.clear();
    _barriersMutex.unset();

    _frameDatasMutex.set();
    for( eqNet::IDHash< FrameData* >::const_iterator i = _frameDatas.begin(); 
         i != _frameDatas.end(); ++ i )
    {
        FrameData* frameData = i->second;
        session->unmapObject( frameData );
        delete frameData;
    }
    _frameDatas.clear();
    _frameDatasMutex.unset();
}


void* Node::_runThread()
{
    EQINFO << "Entered node thread" << endl;

    Config* config = getConfig();
    EQASSERT( config );

    eqNet::Node::setLocalNode( config->getLocalNode( ));

#if 0
    while( node->isRunning( ))
    {
        node->startFrame();
        node->endFrame();
    }
#else
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
#endif
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
    
    _config->attachObject( pipe, packet->pipeID );
    _addPipe( pipe );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdDestroyPipe( eqNet::Command& command )
{
    const NodeDestroyPipePacket* packet = 
        command.getPacket<NodeDestroyPipePacket>();
    EQINFO << "Handle destroy pipe " << packet << endl;

    Pipe* pipe = _findPipe( packet->pipeID );
    pipe->_thread->join(); // wait for pipe thread termination. move to pipe

    _removePipe( pipe );
    _config->detachObject( pipe );
    delete pipe;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdInit( eqNet::Command& command )
{
    EQINFO << "handle node init (recv) " << command.getPacket<NodeInitPacket>()
           << endl;

    _thread->start();
    _pushCommand( command );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqInit( eqNet::Command& command )
{
    const NodeInitPacket* packet = command.getPacket<NodeInitPacket>();
    EQINFO << "handle node init (node) " << packet << endl;

    _error.clear();
    NodeInitReplyPacket reply( packet );
    reply.result = init( packet->initID );

    send( command.getNode(), reply, _error );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_reqExit( eqNet::Command& command )
{
    const NodeExitPacket* packet = command.getPacket<NodeExitPacket>();
    EQINFO << "handle node exit " << packet << endl;

    exit();
    _flushObjects();

    NodeExitReplyPacket reply( packet );
    send( command.getNode(), reply );

    _thread->exit();
    EQUNREACHABLE;
    return eqNet::COMMAND_HANDLED;
}

