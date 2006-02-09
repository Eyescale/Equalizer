
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "config.h"
#include "pipe.h"

#include <eq/packets.h>
#include <eq/net/barrier.h>

using namespace eqs;
using namespace std;

Node::Node()
        : eqNet::Node( eq::CMD_NODE_ALL ),
          _used(0),
          _config(NULL),
          _pendingRequestID( INVALID_ID )
{
    _autoLaunch = true;

    registerCommand( eq::CMD_NODE_INIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Node::_cmdInitReply ));
    registerCommand( eq::CMD_NODE_EXIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Node::_cmdExitReply ));
}

void Node::addPipe( Pipe* pipe )
{
    _pipes.push_back( pipe ); 
    pipe->_node = this; 
    _config->registerObject( pipe );
}

const string& Node::getProgramName()
{
    const Config* config = getConfig();
    EQASSERT( config );

    return config->getRenderClient();
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Node::startInit()
{
    _sendInit();

    eq::NodeCreatePipePacket createPipePacket( _session->getID(), _id );

    const uint32_t nPipes = _pipes.size();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = _pipes[i];
        if( pipe->isUsed( ))
        {
            createPipePacket.pipeID = pipe->getID();
            send( createPipePacket );
            pipe->startInit();
        }
    }
}

void Node::_sendInit()
{
    EQASSERT( _pendingRequestID == INVALID_ID );

    eq::NodeInitPacket packet( _config->getID(), _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Node::syncInit()
{
    bool success = true;
    const int nPipes = _pipes.size();
    for( int i=0; i<nPipes; ++i )
    {
        Pipe* pipe = _pipes[i];
        if( pipe->isUsed( ))
            if( !pipe->syncInit( ))
                success = false;
    }

    EQASSERT( _pendingRequestID != INVALID_ID );

    if( !(bool)_requestHandler.waitRequest( _pendingRequestID ))
        success = false;
    _pendingRequestID = INVALID_ID;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Node::startExit()
{
    const uint32_t nPipes = _pipes.size();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = _pipes[i];
        if( pipe->isUsed( ))
            pipe->startExit();
    }

    _sendExit();
}

void Node::_sendExit()
{
    EQASSERT( _pendingRequestID == INVALID_ID );

    eq::NodeExitPacket packet( _config->getID(), _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Node::syncExit()
{
    EQASSERT( _pendingRequestID != INVALID_ID );

    bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;
    
    eq::NodeDestroyPipePacket destroyPipePacket( _session->getID(), _id );
    
    const uint32_t nPipes = _pipes.size();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = _pipes[i];
        if( pipe->isUsed( ))
        {
            if( !pipe->syncExit( ))
                success = false;

            destroyPipePacket.pipeID = pipe->getID();
            send( destroyPipePacket );
        }
    }

    return success;
}

void Node::stop()
{
    eq::NodeStopPacket packet( _config->getID(), _id );
    send( packet );
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Node::update()
{
    const uint32_t nPipes = this->nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = getPipe( i );
        if( pipe->isUsed( ))
            pipe->update();
    }
}

//---------------------------------------------------------------------------
// Barrier cache
//---------------------------------------------------------------------------
eqNet::Barrier* Node::getBarrier( const uint32_t height )
{
    vector<eqNet::Barrier*>& barriers  = _barrierCache[height];

    if( barriers.size() > 0 )
    {
        eqNet::Barrier* barrier = barriers.back();
        barriers.pop_back();
        return barrier;
    }

    eqNet::Barrier* barrier = new eqNet::Barrier( height );
    _config->registerMobject( barrier, this );
    
    eq::NodeCreateBarrierPacket packet( _session->getID(), _id );
    packet.barrierID = barrier->getID();
    packet.height    = height;
    send( packet );

    return barrier;
}

void Node::releaseBarrier( eqNet::Barrier* barrier )
{
    _barrierCache[barrier->getHeight()].push_back( barrier );
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Node::_cmdInitReply( eqNet::Node* node,
                                          const eqNet::Packet* pkg )
{
    eq::NodeInitReplyPacket* packet = (eq::NodeInitReplyPacket*)pkg;
    EQINFO << "handle node init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdExitReply( eqNet::Node* node,
                                          const eqNet::Packet* pkg )
{
    eq::NodeExitReplyPacket* packet = (eq::NodeExitReplyPacket*)pkg;
    EQINFO << "handle node exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}


ostream& eqs::operator << ( ostream& os, const Node* node )
{
    if( !node )
    {
        os << "NULL node";
        return os;
    }
    
    const uint32_t nPipes = node->nPipes();
    os << "node " << node->getID() << "(" << (void*)node << ")"
       << ( node->isUsed() ? " used " : " unused " ) << nPipes << " pipes";

    for( uint32_t i=0; i<nPipes; i++ )
        os << endl << "    " << node->getPipe(i);

    return os;
}
