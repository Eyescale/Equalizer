
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "config.h"
#include "pipe.h"

#include <eq/packets.h>

using namespace eqs;
using namespace std;

Node::Node()
        : eqNet::Node( eq::CMD_NODE_ALL ),
          _used(0),
          _config(NULL),
          _pendingRequestID(INVALID_ID)
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
    ASSERT( config );

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

    eq::NodeCreatePipePacket createPipePacket( _sessionID, _id );

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
    ASSERT( _pendingRequestID == INVALID_ID );

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

    ASSERT( _pendingRequestID != INVALID_ID );

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
    ASSERT( _pendingRequestID == INVALID_ID );

    eq::NodeExitPacket packet( _config->getID(), _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Node::syncExit()
{
    ASSERT( _pendingRequestID != INVALID_ID );

    bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;
    
    eq::NodeDestroyPipePacket destroyPipePacket( _sessionID, _id );
    
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

//===========================================================================
// command handling
//===========================================================================
void Node::_cmdInitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::NodeInitReplyPacket* packet = (eq::NodeInitReplyPacket*)pkg;
    INFO << "handle node init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
}

void Node::_cmdExitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::NodeExitReplyPacket* packet = (eq::NodeExitReplyPacket*)pkg;
    INFO << "handle node exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
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
