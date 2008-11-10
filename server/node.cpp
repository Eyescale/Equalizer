
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

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{
typedef net::CommandFunc<Node> NodeFunc;

void Node::_construct()
{
    _used           = 0;
    _config         = 0;
    _lastDrawPipe   = 0;
    _flushedFrame   = 0;
    _finishedFrame  = 0;
    EQINFO << "New node @" << (void*)this << endl;
}

Node::Node()
{
    _construct();

    const Global* global = Global::instance();    
    for( int i=0; i < eq::Node::IATTR_ALL; ++i )
        _iAttributes[i] =global->getNodeIAttribute((eq::Node::IAttribute)i);
}

Node::Node( const Node& from, const CompoundVector& compounds )
        : net::Object()
{
    _construct();

    _name = from._name;
    _node = from._node;

    for( int i=0; i<eq::Node::IATTR_ALL; ++i )
        _iAttributes[i] = from.getIAttribute( (eq::Node::IAttribute)i );

    const ConnectionDescriptionVector& descriptions =
        from.getConnectionDescriptions();
    for( ConnectionDescriptionVector::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        const ConnectionDescriptionPtr desc = *i;
        addConnectionDescription( 
            new ConnectionDescription( desc.getReference( )));
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
                               net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( eq::CMD_NODE_CONFIG_INIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigInitReply ), queue );
    registerCommand( eq::CMD_NODE_CONFIG_EXIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigExitReply ), queue );
    registerCommand( eq::CMD_NODE_FRAME_FINISH_REPLY,
                     NodeFunc( this, &Node::_cmdFrameFinishReply ), queue );
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

NodeVisitor::Result Node::accept( NodeVisitor* visitor )
{ 
    PipeVisitor::Result result = visitor->visitPre( this );
    if( result != PipeVisitor::TRAVERSE_CONTINUE )
        return result;

    for( PipeVector::const_iterator i = _pipes.begin(); 
         i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        switch( pipe->accept( visitor ))
        {
            case PipeVisitor::TRAVERSE_TERMINATE:
                return PipeVisitor::TRAVERSE_TERMINATE;

            case PipeVisitor::TRAVERSE_PRUNE:
                result = PipeVisitor::TRAVERSE_PRUNE;
                break;
                
            case PipeVisitor::TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor->visitPost( this ))
    {
        case NodeVisitor::TRAVERSE_TERMINATE:
	  return NodeVisitor::TRAVERSE_TERMINATE;

        case NodeVisitor::TRAVERSE_PRUNE:
	  return NodeVisitor::TRAVERSE_PRUNE;
	  break;
                
        case NodeVisitor::TRAVERSE_CONTINUE:
        default:
	  break;
    }

    return result;
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
    memcpy( packet.iAttributes, _iAttributes, 
            eq::Node::IATTR_ALL * sizeof( int32_t ));

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
    
    if( !_lastDrawPipe ) // happens when used channels skip a frame (DPlex, LB)
        _lastDrawPipe = _pipes[0];

    eq::NodeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    _send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK node start frame " << &startPacket << endl;

    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( pipe->isUsed( ))
            pipe->update( frameID, frameNumber );
    }

    const Config*  config  = getConfig();
    const uint32_t latency = config->getLatency();

    eq::NodeFrameTasksFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    _send( finishPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK node tasks finish " << &finishPacket <<endl;

    if( frameNumber > latency )
        flushFrames( frameNumber - latency );

    _bufferedTasks.sendBuffer( _node->getConnection( ));
    _lastDrawPipe = 0;
}

void Node::updateFrameFinishNT( const uint32_t currentFrame )
{
    const Config*  config  = getConfig();
    const uint32_t latency = config->getLatency();
    if( latency == 0 || currentFrame <= latency )
        return;

    const uint32_t            frameNumber = currentFrame - latency;
    if( _frameIDs.find( frameNumber ) == _frameIDs.end( ))
        return; // finish already send by previous updateFrameFinishNT

    eq::NodeFrameFinishPacket packet;
    packet.frameID          = _frameIDs[ frameNumber ];
    packet.frameNumber      = frameNumber;
    packet.syncGlobalFinish = isApplicationNode();

    _send( packet );
    _frameIDs.erase( frameNumber );

    EQLOG( eq::LOG_TASKS ) << "TASK node finish frame non-threaded " << &packet
                           << endl;
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
}

void Node::_sendFrameFinish( const uint32_t frameNumber )
{
    if( _frameIDs.find( frameNumber ) == _frameIDs.end( ))
        return; // finish already send by updateFrameFinishNT

    eq::NodeFrameFinishPacket packet;
    packet.frameID     = _frameIDs[ frameNumber ];
    packet.frameNumber = frameNumber;

    _send( packet );
    _frameIDs.erase( frameNumber );
    EQLOG( eq::LOG_TASKS ) << "TASK node finish frame  " << &packet << endl;
}

//---------------------------------------------------------------------------
// Barrier cache
//---------------------------------------------------------------------------
net::Barrier* Node::getBarrier()
{
    if( _barriers.empty() )
    {
        net::Barrier* barrier = new net::Barrier( _node );
        _config->registerObject( barrier );
        barrier->setAutoObsolete( getConfig()->getLatency()+1, 
                                  Object::AUTO_OBSOLETE_COUNT_VERSIONS );
        
        return barrier;
    }

    net::Barrier* barrier = _barriers.back();
    _barriers.pop_back();
    barrier->setHeight(0);
    return barrier;
}

void Node::releaseBarrier( net::Barrier* barrier )
{
    _barriers.push_back( barrier );
}

void Node::_flushBarriers()
{
    for( vector< net::Barrier* >::const_iterator i =_barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        net::Barrier* barrier = *i;
        _config->deregisterObject( barrier );
        delete barrier;
    }
    _barriers.clear();
}

//===========================================================================
// command handling
//===========================================================================
net::CommandResult Node::_cmdConfigInitReply( net::Command& command )
{
    const eq::NodeConfigInitReplyPacket* packet = 
        command.getPacket<eq::NodeConfigInitReplyPacket>();
    EQINFO << "handle configInit reply " << packet << endl;

    _error = packet->error;
    if( packet->result )
        _state = STATE_RUNNING;
    else
        _state = STATE_INIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Node::_cmdConfigExitReply( net::Command& command )
{
    const eq::NodeConfigExitReplyPacket* packet =
        command.getPacket<eq::NodeConfigExitReplyPacket>();
    EQINFO << "handle configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_STOPPED;
    else
        _state = STATE_STOP_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Node::_cmdFrameFinishReply( net::Command& command )
{
    const eq::NodeFrameFinishReplyPacket* packet = 
        command.getPacket<eq::NodeFrameFinishReplyPacket>();
    EQVERB << "handle frame finish reply " << packet << endl;
    
    _finishedFrame = packet->frameNumber;
    _config->notifyNodeFrameFinished( packet->frameNumber );

    return net::COMMAND_HANDLED;
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

    const int32_t value = node->getIAttribute( eq::Node::IATTR_THREAD_MODEL );
    if( value != 
        Global::instance()->getNodeIAttribute( eq::Node::IATTR_THREAD_MODEL ))
    {
        os << "attributes" << endl << "{" << endl << indent
           << "thread_model " << static_cast< eq::IAttrValue >( value ) << endl
           << exdent << "}" << endl;
    }

    const PipeVector& pipes = node->getPipes();
    for( PipeVector::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
        os << *i;

    os << exdent << "}" << enableFlush << endl;
    return os;
}

}
}
