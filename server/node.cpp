
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "channel.h"
#include "config.h"
#include "global.h"
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

    registerCommand( eq::CMD_NODE_INIT_REPLY, 
                     eqNet::CommandFunc<Node>( this, &Node::_cmdInitReply ));
    registerCommand( eq::CMD_NODE_EXIT_REPLY, 
                     eqNet::CommandFunc<Node>( this, &Node::_cmdExitReply ));

    const Global* global = Global::instance();

    for( int i=0; i<SATTR_ALL; ++i )
        _sAttributes[i] = global->getNodeSAttribute( (SAttribute)i );
    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = global->getNodeIAttribute( (IAttribute)i );
}

Node::Node()
        : eqNet::Object( eq::Object::TYPE_NODE )
{
    _construct();
}

Node::Node( const Node& from )
        : eqNet::Object( eq::Object::TYPE_NODE )
{
    _construct();
    _node = from._node;

    for( int i=0; i<SATTR_ALL; ++i )
        _sAttributes[i] = from._sAttributes[i];
    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

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

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Node::startInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::NodeInitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
    Object::send( _node, packet );
}

bool Node::syncInit()
{
    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_ID_INVALID;

    if( !success )
        EQWARN << "Node initialisation failed" << endl;
    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Node::startExit()
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::NodeExitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    Object::send( _node, packet );
}

bool Node::syncExit()
{
    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_ID_INVALID;

    return success;
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
eqNet::Barrier* Node::getBarrier()
{
    if( _barrierCache.empty() )
    {
        eqNet::Barrier* barrier = new eqNet::Barrier( _node );
        barrier->setAutoObsolete( getConfig()->getLatency()+1, 
                                  Object::AUTO_OBSOLETE_COUNT_VERSIONS );
        _config->registerObject( barrier, 
                        RefPtr_static_cast<Server, eqNet::Node>(getServer( )) );
        
        return barrier;
    }

    eqNet::Barrier* barrier = _barrierCache.back();
    _barrierCache.pop_back();
    barrier->setHeight(0);
    return barrier;
}

void Node::releaseBarrier( eqNet::Barrier* barrier )
{
    _barrierCache.push_back( barrier );
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Node::_cmdInitReply( eqNet::Command& command )
{
    const eq::NodeInitReplyPacket* packet = 
        command.getPacket<eq::NodeInitReplyPacket>();
    EQINFO << "handle init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdExitReply( eqNet::Command& command )
{
    const eq::NodeExitReplyPacket* packet =
        command.getPacket<eq::NodeExitReplyPacket>();
    EQINFO << "handle exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
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
