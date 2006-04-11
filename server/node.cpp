
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "pipe.h"
#include "window.h"

#include <eq/client/packets.h>
#include <eq/net/barrier.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Node::_construct()
{
    _used             = 0;
    _config           = NULL;
    _pendingRequestID = EQ_INVALID_ID;

    _autoLaunch       = true;

    registerCommand( eq::CMD_NODE_INIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Node::_cmdInitReply ));
    registerCommand( eq::CMD_NODE_EXIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Node::_cmdExitReply ));

    const Global* global = Global::instance();

    for( int i=0; i<SATTR_ALL; ++i )
        _sAttributes[i] = global->getNodeSAttribute( (SAttribute)i );
    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = global->getNodeIAttribute( (IAttribute)i );
}

Node::Node()
        : eqNet::Node( eq::CMD_NODE_ALL )
{
    _construct();
}

Node::Node( const Node& from )
        : eqNet::Node( eq::CMD_NODE_ALL )
{
    _construct();

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


void Node::addPipe( Pipe* pipe )
{
    _pipes.push_back( pipe ); 
    pipe->_node = this; 

    if( _config )
        pipe->adjustLatency( _config->getLatency( ));
}

bool Node::removePipe( Pipe* pipe )
{
    vector<Pipe*>::iterator iter = _pipes.begin();
    for( ; iter != _pipes.end(); ++iter )
        if( (*iter) == pipe )
            break;

    if( iter == _pipes.end( ))
        return false;

    _pipes.erase( iter );

    pipe->adjustLatency( -_config->getLatency( ));
    pipe->_node = NULL; 

    return true;
}

const string& Node::getProgramName()
{
    const Config* config = getConfig();
    EQASSERT( config );

    return config->getRenderClient();
}

const string& Node::getWorkDir()
{
    const Config* config = getConfig();
    EQASSERT( config );

    return config->getWorkDir();
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Node::startInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_INVALID_ID );

    eq::NodeInitPacket packet( _config->getID(), getID( ));
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
    send( packet );
}

bool Node::syncInit()
{
    EQASSERT( _pendingRequestID != EQ_INVALID_ID );

    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_INVALID_ID;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Node::startExit()
{
    EQASSERT( _pendingRequestID == EQ_INVALID_ID );

    eq::NodeExitPacket packet( _config->getID(), getID( ));
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Node::syncExit()
{
    EQASSERT( _pendingRequestID != EQ_INVALID_ID );

    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_INVALID_ID;

    return success;
}

void Node::stop()
{
    eq::NodeStopPacket packet( _config->getID(), getID( ));
    send( packet );
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Node::update( const uint32_t frameID )
{
    EQVERB << "Start frame" << endl;
    const uint32_t nPipes = this->nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = getPipe( i );
        if( pipe->isUsed( ))
            pipe->update( frameID );
    }
}

void Node::syncUpdate()
{
    const uint32_t nPipes = this->nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = getPipe( i );
        if( pipe->isUsed( ))
            pipe->syncUpdate();
    }
}

void Node::adjustLatency( const int delta)
{
    const uint32_t nPipes = this->nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
    {
        Pipe* pipe = getPipe( i );
        pipe->adjustLatency( delta );
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
    _config->registerObject( barrier, this );

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
    EQINFO << "handle init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdExitReply( eqNet::Node* node,
                                          const eqNet::Packet* pkg )
{
    eq::NodeExitReplyPacket* packet = (eq::NodeExitReplyPacket*)pkg;
    EQINFO << "handle exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}

ostream& eqs::operator << ( ostream& os, const Node* node )
{
    if( !node )
        return os;
    
    os << disableSync << "node" << endl;
    os << "{" << endl << indent;

    const uint32_t nConnectionDescriptions = node->nConnectionDescriptions();
    for( uint32_t i=0; i<nConnectionDescriptions; i++ )
        os << node->getConnectionDescription( i ).get();

    const uint32_t nPipes = node->nPipes();
    for( uint32_t i=0; i<nPipes; i++ )
        os << node->getPipe(i);

    os << exdent << "}" << enableSync << endl;
    return os;
}
