
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "config.h"
#include "pipe.h"

using namespace eqs;
using namespace std;

Node::Node()
        : eqNet::Node(),
          _used(0),
          _config(NULL),
          _pendingRequestID(INVALID_ID)
{
    _autoLaunch = true;

    for( int i=0; i<eq::CMD_NODE_ALL; i++ )
        _cmdHandler[i] = &eqs::Node::_cmdUnknown;

    _cmdHandler[eq::CMD_NODE_INIT_REPLY] = &eqs::Node::_cmdInitReply;
    _cmdHandler[eq::CMD_NODE_EXIT_REPLY] = &eqs::Node::_cmdExitReply;
}

void Node::addPipe( Pipe* pipe )
{
    _pipes.push_back( pipe ); 
    pipe->_node = this; 
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

void Node::sendInit()
{
    ASSERT( _pendingRequestID == INVALID_ID );

    eq::NodeInitPacket packet( _config->getID(), _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Node::syncInit()
{
    ASSERT( _pendingRequestID != INVALID_ID );

    const bool result = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;
    return result;
}

void Node::sendExit()
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

    const bool result = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;
    return result;
}

void Node::stop()
{
    eq::NodeStopPacket packet( _config->getID(), _id );
    send( packet );
}

//===========================================================================
// command handling
//===========================================================================
void Node::handlePacket( eqNet::Node* node, const eq::NodePacket* packet )
{
    switch( packet->datatype )
    {
        case eq::DATATYPE_EQ_NODE:
            ASSERT( packet->command < eq::CMD_NODE_ALL );

            (this->*_cmdHandler[packet->command])(node, packet);
            break;

        default:
            ERROR << "unimplemented" << endl;
            break;
    }
}

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
    
    const uint nPipes = node->nPipes();
    os << "node " << node->getID() << "(" << (void*)node << ")"
       << ( node->isUsed() ? " used " : " unused " ) << nPipes << " pipes";

    for( uint i=0; i<nPipes; i++ )
        os << endl << "    " << node->getPipe(i);

    return os;
}
