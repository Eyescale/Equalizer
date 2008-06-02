
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
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

using namespace eqBase;
using namespace std;
using eqNet::ConnectionDescriptionVector;
using eqNet::CommandFunc;

namespace eqs
{

void Node::_construct()
{
    _used             = 0;
    _config           = NULL;
    _lastDrawCompound = 0;
    _flushedFrame     = 0;
    _finishedFrame    = 0;

    EQINFO << "New node @" << (void*)this << endl;
}

Node::Node()
{
    _construct();
}

Node::Node( const Node& from, const CompoundVector& compounds )
        : eqNet::Object()
{
    _construct();

    _name = from._name;
    _node = from._node;

    const ConnectionDescriptionVector& descriptions = 
        from.getConnectionDescriptions();
    for( ConnectionDescriptionVector::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        const eqNet::ConnectionDescription* desc = (*i).get();
        addConnectionDescription( new eqNet::ConnectionDescription( *desc ));
    }

    const PipeVector& pipes = from.getPipes();
    for( PipeVector::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        const Pipe* pipe      = *i;
        Pipe*       pipeClone = new Pipe( *pipe, compounds );

        addPipe( pipeClone );
    }
}

Node::~Node()
{
    EQINFO << "Delete node @" << (void*)this << endl;

    if( _config )
        _config->removeNode( this );
    
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;

        pipe->_node = 0;
        delete pipe;
    }
    _pipes.clear();
}

void Node::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               eqNet::Session* session )
{
    eqNet::Object::attachToSession( id, instanceID, session );
    
    eqNet::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( eq::CMD_NODE_CONFIG_INIT_REPLY, 
                     CommandFunc<Node>( this, &Node::_cmdConfigInitReply ),
                     queue );
    registerCommand( eq::CMD_NODE_CONFIG_EXIT_REPLY, 
                     CommandFunc<Node>( this, &Node::_cmdConfigExitReply ),
                     queue );
    registerCommand( eq::CMD_NODE_FRAME_FINISH_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdFrameFinishReply ),
                     queue );
}

void Node::addPipe( Pipe* pipe )
{
    _pipes.push_back( pipe ); 
    pipe->_node = this; 
}

bool Node::removePipe( Pipe* pipe )
{
    PipeVector::iterator i = find( _pipes.begin(), _pipes.end(), pipe );
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
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    _flushedFrame  = 0;
    _finishedFrame = 0;
    _frameIDs.clear();

    eq::NodeConfigInitPacket packet;
    packet.initID = initID;
    _send( packet, _name );
    EQLOG( eq::LOG_TASKS ) << "TASK node configInit  " << &packet << endl;

    eq::NodeCreatePipePacket createPipePacket;
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( !pipe->isUsed( ))
            continue;
        
        _config->registerObject( pipe );
        createPipePacket.pipeID   = pipe->getID();
        createPipePacket.threaded = 
            pipe->getIAttribute( Pipe::IATTR_HINT_THREAD );
        _send( createPipePacket );
        pipe->startConfigInit( initID );
    }

    _bufferedTasks.sendBuffer( _node->getConnection( ));
}

bool Node::syncConfigInit()
{
    bool success = true;
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( !pipe->isUsed( ))
            continue;

        if( !pipe->syncConfigInit( ))
        {
            _error += "pipe " + pipe->getName() + ": '" + 
                      pipe->getErrorMessage() + '\'';
            success = false;
        }
    }

    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_RUNNING ||
              _state == STATE_INIT_FAILED );
    _state.waitNE( STATE_INITIALIZING );
    if( _state == STATE_INIT_FAILED )
        success = false;

    if( !success )
        EQWARN << "Node initialization failed: " << _error << endl;
    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Node::startConfigExit()
{
    if( _state == STATE_STOPPED ) // never send the init tasks
        return;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_STOPPING;

    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;

        if( pipe->getState() != Pipe::STATE_STOPPED )
            pipe->startConfigExit();
    }

    eq::NodeConfigExitPacket packet;
    _send( packet );
    _bufferedTasks.sendBuffer( _node->getConnection( ));
}

bool Node::syncConfigExit()
{
    EQASSERT( _state == STATE_STOPPING || _state == STATE_STOPPED || 
              _state == STATE_STOP_FAILED );
    
    _state.waitNE( STATE_STOPPING );
    bool success = ( _state == STATE_STOPPED );
    EQASSERT( success || _state == STATE_STOP_FAILED );
    _state = STATE_STOPPED; // STOP_FAILED -> STOPPED transition

    eq::NodeDestroyPipePacket destroyPipePacket;
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( pipe->getID() == EQ_ID_INVALID )
            continue;

        if( !pipe->syncConfigExit( ))
        {
            EQWARN << "Could not exit cleanly: " << pipe << endl;
            success = false;
        }

        destroyPipePacket.pipeID = pipe->getID();
        _send( destroyPipePacket );
        _config->deregisterObject( pipe );
    }
    _bufferedTasks.sendBuffer( _node->getConnection( ));

    _frameIDs.clear();
    _flushBarriers();
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Node::update( const uint32_t frameID, const uint32_t frameNumber )
{
    EQVERB << "Start frame " << frameNumber << endl;
    _frameIDs[ frameNumber ] = frameID;
    
    if( !_lastDrawCompound )
    {
        Config* config = getConfig();
        _lastDrawCompound = config->getCompounds()[0];
    }

    eq::NodeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    _send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK node start frame " << &startPacket << endl;

    for( vector< Pipe* >::const_iterator i = _pipes.begin();
         i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( pipe->isUsed( ))
            pipe->update( frameID, frameNumber );
    }

    const Config*  config  = getConfig();
    const uint32_t latency = config->getLatency();

    // Note: we could always wait for the pipes to return the finish, and then
    // send the node finish. This is an optimisation to queue the command on the
    // node when the latency is exhausted, in order to avoid the round-trip at
    // the end of a frame.
    if( frameNumber > latency )
        flushFrames( frameNumber - latency );

    _bufferedTasks.sendBuffer( _node->getConnection( ));
    _lastDrawCompound = 0;
}

void Node::notifyPipeFrameFinished( const uint32_t frameNumber )
{
    if( _finishedFrame >= frameNumber ) // node finish already done
        return;

    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        const Pipe* pipe = *i;
        if( pipe->isUsed() && pipe->getFinishedFrame() < frameNumber )
            return;
    }

    // All pipes have finished the frame. Send a priority packet to the node, so
    // that the node can finish the frame early.
    eq::NodeFrameFinishEarlyPacket finishPacket;
    finishPacket.frameID     = _frameIDs[ frameNumber ];
    finishPacket.frameNumber = frameNumber;

    // do not used _send/_bufferedTasks, not thread-safe!
    finishPacket.sessionID   = _config->getID();
    finishPacket.objectID    = getID();
    _node->send( finishPacket );
}

void Node::flushFrames( const uint32_t frameNumber )
{
    EQVERB << "Flush frames including " << frameNumber << endl;

    while( _flushedFrame < frameNumber )
    {
        ++_flushedFrame;
        _sendFrameFinish( _flushedFrame );
    }

    _bufferedTasks.sendBuffer( _node->getConnection( ));
    _frameIDs.erase( frameNumber );
}

void Node::_sendFrameFinish( const uint32_t frameNumber)
{
    eq::NodeFrameFinishPacket finishPacket;
    finishPacket.frameID     = _frameIDs[ frameNumber ];
    finishPacket.frameNumber = frameNumber;

    _send( finishPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK node finish frame  " << &finishPacket
                           << endl;
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
    if( packet->result )
        _state = STATE_RUNNING;
    else
        _state = STATE_INIT_FAILED;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdConfigExitReply( eqNet::Command& command )
{
    const eq::NodeConfigExitReplyPacket* packet =
        command.getPacket<eq::NodeConfigExitReplyPacket>();
    EQINFO << "handle configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_STOPPED;
    else
        _state = STATE_STOP_FAILED;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdFrameFinishReply( eqNet::Command& command )
{
    const eq::NodeFrameFinishReplyPacket* packet = 
        command.getPacket<eq::NodeFrameFinishReplyPacket>();
    EQVERB << "handle frame finish reply " << packet << endl;
    
    _finishedFrame = packet->frameNumber;
    _config->notifyNodeFrameFinished( packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}

ostream& operator << ( ostream& os, const Node* node )
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

    const std::string& name = node->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << endl;

    const ConnectionDescriptionVector& descriptions = 
        node->getConnectionDescriptions();
    for( ConnectionDescriptionVector::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )

        os << (*i).get();

    const PipeVector& pipes = node->getPipes();
    for( PipeVector::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
        os << *i;

    os << exdent << "}" << enableFlush << endl;
    return os;
}

}
