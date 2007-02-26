
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "pipe.h"
#include "server.h"
#include "window.h"

#include <eq/net/barrier.h>
#include <eq/net/command.h>
#include <eq/client/packets.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Node::_construct()
{
    _used             = 0;
    _config           = NULL;
    _pendingRequestID = EQ_ID_INVALID;

    registerCommand( eq::CMD_NODE_CONFIG_INIT_REPLY, 
                  eqNet::CommandFunc<Node>( this, &Node::_cmdConfigInitReply ));
    registerCommand( eq::CMD_NODE_CONFIG_EXIT_REPLY, 
                  eqNet::CommandFunc<Node>( this, &Node::_cmdConfigExitReply ));
    registerCommand( eq::CMD_NODE_FRAME_FINISH_REPLY,
                 eqNet::CommandFunc<Node>( this, &Node::_cmdFrameFinishReply ));

    EQINFO << "Add node @" << (void*)this << endl;
}

Node::Node()
{
    _construct();
}

Node::Node( const Node& from )
{
    _construct();
    _node = from._node;

    const uint32_t nConnectionDescriptions = from.nConnectionDescriptions();
    for( uint32_t i=0; i<nConnectionDescriptions; i++ )
    {
        eqNet::ConnectionDescription* desc = 
            from.getConnectionDescription(i).get();
        
        addConnectionDescription( new eqNet::ConnectionDescription( *desc ));
    }

    const uint32_t nPipes = from.nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe      = from.getPipe(i);
        Pipe* pipeClone = new Pipe( *pipe );
            
        addPipe( pipeClone );
    }
}

Node::~Node()
{
    EQINFO << "Delete node @" << (void*)this << endl;

    if( _config )
        _config->removeNode( this );
    
    for( vector<Pipe*>::const_iterator i = _pipes.begin(); i != _pipes.end();
         ++i )
    {
        Pipe* pipe = *i;

        pipe->_node = 0;
        delete pipe;
    }
    _pipes.clear();
}

void Node::addPipe( Pipe* pipe )
{
    _pipes.push_back( pipe ); 
    pipe->_node = this; 
}

bool Node::removePipe( Pipe* pipe )
{
    vector<Pipe*>::iterator i = find( _pipes.begin(), _pipes.end(), pipe );
    if( i == _pipes.end( ))
        return false;

    _pipes.erase( i );
    pipe->_node = 0; 

    return true;
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Node::startConfigInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::NodeConfigInitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
    send( packet );
}

bool Node::syncConfigInit()
{
    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    const bool success = static_cast<bool>(
        _requestHandler.waitRequest( _pendingRequestID ));
    _pendingRequestID = EQ_ID_INVALID;

    if( !success )
        EQWARN << "Node initialisation failed: " << _error << endl;
    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Node::startConfigExit()
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::NodeConfigExitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Node::syncConfigExit()
{
    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    const bool success = static_cast<bool>(
                             _requestHandler.waitRequest( _pendingRequestID ));
    _pendingRequestID = EQ_ID_INVALID;

    _flushBarriers();
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Node::startFrame( const uint32_t frameID, const uint32_t frameNumber )
{
    EQVERB << "Start frame" << endl;

    eq::NodeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    send( startPacket );

    const uint32_t nPipes = this->nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = getPipe( i );
        if( pipe->isUsed( ))
            pipe->startFrame( frameID, frameNumber );
    }


    eq::NodeFrameFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    send( finishPacket );
}

//---------------------------------------------------------------------------
// Barrier cache
//---------------------------------------------------------------------------
eqNet::Barrier* Node::getBarrier()
{
    if( _barriers.empty() )
    {
        eqNet::Barrier* barrier = new eqNet::Barrier( _node );
        _config->registerObject( barrier );
        barrier->setAutoObsolete( getConfig()->getLatency()+1, 
                                  Object::AUTO_OBSOLETE_COUNT_VERSIONS );
        
        return barrier;
    }

    eqNet::Barrier* barrier = _barriers.back();
    _barriers.pop_back();
    barrier->setHeight(0);
    return barrier;
}

void Node::releaseBarrier( eqNet::Barrier* barrier )
{
    _barriers.push_back( barrier );
}

void Node::_flushBarriers()
{
    for( vector< eqNet::Barrier* >::const_iterator i =_barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        eqNet::Barrier* barrier = *i;
        _config->deregisterObject( barrier );
        delete barrier;
    }
    _barriers.clear();
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Node::_cmdConfigInitReply( eqNet::Command& command )
{
    const eq::NodeConfigInitReplyPacket* packet = 
        command.getPacket<eq::NodeConfigInitReplyPacket>();
    EQINFO << "handle configInit reply " << packet << endl;

    _error = packet->error;
    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdConfigExitReply( eqNet::Command& command )
{
    const eq::NodeConfigExitReplyPacket* packet =
        command.getPacket<eq::NodeConfigExitReplyPacket>();
    EQINFO << "handle configExit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdFrameFinishReply( eqNet::Command& command )
{
    const eq::NodeFrameFinishReplyPacket* packet = 
        command.getPacket<eq::NodeFrameFinishReplyPacket>();
    EQVERB << "handle frame finish reply " << packet << endl;
    
    // Move me
    for( uint32_t i =0; i<packet->nStatEvents; ++i )
    {
        const eq::StatEvent& event = packet->statEvents[i];
        EQLOG( LOG_STATS ) << event << endl;
    }

    _finishedFrame = packet->frameNumber;
    return eqNet::COMMAND_HANDLED;
}

ostream& eqs::operator << ( ostream& os, const Node* node )
{
    if( !node )
        return os;
    
    os << disableFlush;
    const Config* config = node->getConfig();
    if( config && config->isApplicationNode( node ))
        os << "appNode" << endl;
    else
        os << "node" << endl;

    os << "{" << endl << indent;

    const uint32_t nConnectionDescriptions = node->nConnectionDescriptions();
    for( uint32_t i=0; i<nConnectionDescriptions; i++ )
        os << node->getConnectionDescription( i ).get();

    const uint32_t nPipes = node->nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
        os << node->getPipe(i);

    os << exdent << "}" << enableFlush << endl;
    return os;
}
