
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>    
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <pthread.h>
#include "node.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "server.h"
#include "window.h"

#include <eq/client/packets.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>
#include <eq/net/barrier.h>
#include <eq/net/command.h>

namespace eq
{
namespace server
{
typedef fabric::Node< Config, Node, Pipe, NodeVisitor > Super;
typedef net::CommandFunc<Node> NodeFunc;

void Node::_construct()
{
    _active         = 0;
    _lastDrawPipe   = 0;
    _flushedFrame   = 0;
    _finishedFrame  = 0;
    EQINFO << "New node @" << (void*)this << std::endl;
}

Node::Node( Config* parent )
    : Super( parent ) 
{
    _construct();
    const Global* global = Global::instance();    
    for( int i=0; i < Node::SATTR_ALL; ++i )
    {
        const SAttribute attr = static_cast< SAttribute >( i );
        setSAttribute( attr, global->getNodeSAttribute( attr ));
    }
    for( int i=0; i < Node::CATTR_ALL; ++i )
    {
        const CAttribute attr = static_cast< CAttribute >( i );
        setCAttribute( attr, global->getNodeCAttribute( attr ));
    }
    for( int i = 0; i < IATTR_ALL; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getNodeIAttribute( attr ));
    }
}

Node::~Node()
{
    EQINFO << "Delete node @" << (void*)this << std::endl;
}

void Node::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    Super::attachToSession( id, instanceID, session );
    
    net::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( fabric::CMD_NODE_CONFIG_INIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigInitReply ), queue );
    registerCommand( fabric::CMD_NODE_CONFIG_EXIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigExitReply ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_FINISH_REPLY,
                     NodeFunc( this, &Node::_cmdFrameFinishReply ), queue );
}

ServerPtr Node::getServer()
{
    return getConfig() ? getConfig()->getServer() : 0;
}

ConstServerPtr Node::getServer() const
{
    return getConfig() ? getConfig()->getServer() : 0;
}

net::CommandQueue* Node::getMainThreadQueue()
{
    return getConfig()->getMainThreadQueue();
}

net::CommandQueue* Node::getCommandThreadQueue()
{
    return getConfig()->getCommandThreadQueue();
}

Channel* Node::getChannel( const ChannelPath& path )
{
    const Pipes& pipes = getPipes();
    EQASSERT( pipes.size() > path.pipeIndex );

    if( pipes.size() <= path.pipeIndex )
        return 0;

    return pipes[ path.pipeIndex ]->getChannel( path );
}

void Node::addTasks( const uint32_t tasks )
{
    setTasks( getTasks() | tasks );
}

void Node::activate()
{   
    ++_active;
    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Node::deactivate()
{ 
    EQASSERT( _active != 0 );
    --_active; 
    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// update running entities (init/exit)
//---------------------------------------------------------------------------

void Node::updateRunning( const uint32_t initID, const uint32_t frameNumber )
{
    if( !isActive() && _state == STATE_STOPPED ) // inactive
        return;

    if( isActive() && _state != STATE_RUNNING ) // becoming active
    {
        EQASSERT( _state == STATE_STOPPED );
        _configInit( initID, frameNumber );
    }

    // Let all running pipes update their running state (incl. children)
    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
        (*i)->updateRunning( initID, frameNumber );

    if( !isActive( )) // becoming inactive
    {
        EQASSERT( _state == STATE_RUNNING );
        _configExit();
    }

    flushSendBuffer();
}

bool Node::syncRunning()
{
    if( !isActive() && _state == STATE_STOPPED ) // inactive
        return 0;

    // Sync state updates
    bool result = true;

    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( !pipe->syncRunning( ))
        {
            setErrorMessage( getErrorMessage() + "pipe " + pipe->getName() +
                             ": '" + pipe->getErrorMessage() + '\'' );
            result = false;
        }
    }

    flushSendBuffer();

    if( isActive() && _state != STATE_RUNNING && !_syncConfigInit( ))
        // becoming active
        result = false;
    if( !isActive() && !_syncConfigExit( )) // becoming inactive
        result = false;

    EQASSERT( isMaster( ));
    commit();

    EQASSERT( _state == STATE_RUNNING || _state == STATE_STOPPED );
    return result;
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Node::_configInit( const uint32_t initID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    _flushedFrame  = 0;
    _finishedFrame = 0;
    _frameIDs.clear();

    EQLOG( LOG_INIT ) << "Create node" << std::endl;
    ConfigCreateNodePacket createNodePacket;
    createNodePacket.nodeID = getID();
    createNodePacket.sessionID = getConfig()->getID();
    _node->send( createNodePacket );

    EQLOG( LOG_INIT ) << "Init node" << std::endl;
    NodeConfigInitPacket packet;
    packet.initID      = initID;
    packet.frameNumber = frameNumber;

    _send( packet );
}

bool Node::_syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    const bool success = ( _state == STATE_INIT_SUCCESS );
    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Node initialization failed: " << getErrorMessage()
               << std::endl;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Node::_configExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    EQLOG( LOG_INIT ) << "Exit node" << std::endl;
    NodeConfigExitPacket packet;
    _send( packet );
    flushSendBuffer();

    EQLOG( LOG_INIT ) << "Destroy node" << std::endl;
    ConfigDestroyNodePacket destroyNodePacket;
    destroyNodePacket.nodeID = getID();
    destroyNodePacket.sessionID = getConfig()->getID();
    _node->send( destroyNodePacket );
}

bool Node::_syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

    _state = STATE_STOPPED; // EXIT_FAILED -> STOPPED transition
    setTasks( fabric::TASK_NONE );
    _frameIDs.clear();
    _flushBarriers();
    sync();
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Node::update( const uint32_t frameID, const uint32_t frameNumber )
{
    EQVERB << "Start frame " << frameNumber << std::endl;
    EQASSERT( _state == STATE_RUNNING );
    EQASSERT( _active > 0 );

    _frameIDs[ frameNumber ] = frameID;
    
    NodeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    startPacket.version     = getVersion();
    _send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK node start frame " << &startPacket << std::endl;

    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( pipe->isActive( ))
            pipe->update( frameID, frameNumber );
    }

    NodeFrameTasksFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    _send( finishPacket );
    EQLOG( LOG_TASKS ) << "TASK node tasks finish " << &finishPacket
                           << std::endl;

    _finish( frameNumber );

    flushSendBuffer();
    _lastDrawPipe = 0;
}

uint32_t Node::_getFinishLatency() const
{
    switch( getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case fabric::DRAW_SYNC:
            if( getTasks() & fabric::TASK_DRAW )
            {
                // More than one frame latency doesn't make sense, since the
                // draw sync for frame+1 does not allow for more
                const Config* config = getConfig();
                const uint32_t latency = config->getLatency();

                return EQ_MIN( latency, 1 );
            }
            break;

        case fabric::LOCAL_SYNC:
            if( getTasks() != fabric::TASK_NONE )
                // local sync enforces no latency
                return 0;
            break;

        case fabric::ASYNC:
            break;
        default:
            EQUNIMPLEMENTED;
    }

    const Config* config = getConfig();
    return config->getLatency();
}

void Node::_finish( const uint32_t currentFrame )
{
    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        const Pipe* pipe = *i;
        if( pipe->getIAttribute( Pipe::IATTR_HINT_THREAD ))
        {
            const uint32_t latency = _getFinishLatency();
            if( currentFrame > latency )
                flushFrames( currentFrame - latency );
            return;
        }
    }

    // else only non-threaded pipes, all local tasks are done, send finish now.
    flushFrames( currentFrame );
}

void Node::flushFrames( const uint32_t frameNumber )
{
    EQLOG( base::LOG_ANY ) << "Flush frames including " << frameNumber
                           << std::endl;

    while( _flushedFrame < frameNumber )
    {
        ++_flushedFrame;
        _sendFrameFinish( _flushedFrame );
    }

    flushSendBuffer();
}

void Node::_sendFrameFinish( const uint32_t frameNumber )
{
    if( _frameIDs.find( frameNumber ) == _frameIDs.end( ))
        return; // finish already send

    NodeFrameFinishPacket packet;
    packet.frameID     = _frameIDs[ frameNumber ];
    packet.frameNumber = frameNumber;

    _send( packet );
    _frameIDs.erase( frameNumber );
    EQLOG( LOG_TASKS ) << "TASK node finish frame  " << &packet << std::endl;
}

//---------------------------------------------------------------------------
// Barrier cache
//---------------------------------------------------------------------------
net::Barrier* Node::getBarrier()
{
    if( _barriers.empty() )
    {
        net::Barrier* barrier = new net::Barrier( _node );
        getConfig()->registerObject( barrier );
        barrier->setAutoObsolete( getConfig()->getLatency() + 1 );
        return barrier;
    }

    net::Barrier* barrier = _barriers.back();
    _barriers.pop_back();
    barrier->setHeight(0);
    return barrier;
}

void Node::changeLatency( const uint32_t latency )
{
    for( std::vector< net::Barrier* >::const_iterator i =_barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        net::Barrier* barrier = *i;
        barrier->setAutoObsolete( latency + 1 );
    }
    setAutoObsolete( latency + 1 );
}

void Node::releaseBarrier( net::Barrier* barrier )
{
    _barriers.push_back( barrier );
}

void Node::_flushBarriers()
{
    for( std::vector< net::Barrier* >::const_iterator i =_barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        net::Barrier* barrier = *i;
        getConfig()->deregisterObject( barrier );
        delete barrier;
    }
    _barriers.clear();
}

void Node::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Super::deserialize( is, dirtyBits );
    EQASSERT( isMaster( ));
    setDirty( dirtyBits ); // redistribute slave changes
}

bool Node::removeConnectionDescription( ConnectionDescriptionPtr cd )
{
    ConnectionDescriptions::iterator i = 
        std::find( _connectionDescriptions.begin(),
                   _connectionDescriptions.end(), cd );
    if( i == _connectionDescriptions.end( ))
        return false;

    _connectionDescriptions.erase( i );
    return true;
}

void Node::flushSendBuffer()
{
    _bufferedTasks.sendBuffer( _node->getConnection( ));
}

//===========================================================================
// command handling
//===========================================================================
net::CommandResult Node::_cmdConfigInitReply( net::Command& command )
{
    const NodeConfigInitReplyPacket* packet = 
        command.getPacket<NodeConfigInitReplyPacket>();
    EQVERB << "handle configInit reply " << packet << std::endl;
    EQASSERT( _state == STATE_INITIALIZING );
    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Node::_cmdConfigExitReply( net::Command& command )
{
    const NodeConfigExitReplyPacket* packet =
        command.getPacket<NodeConfigExitReplyPacket>();
    EQVERB << "handle configExit reply " << packet << std::endl;
    EQASSERT( _state == STATE_EXITING );

    _state = packet->result ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return net::COMMAND_HANDLED;
}

net::CommandResult Node::_cmdFrameFinishReply( net::Command& command )
{
    const NodeFrameFinishReplyPacket* packet = 
        command.getPacket<NodeFrameFinishReplyPacket>();
    EQVERB << "handle frame finish reply " << packet << std::endl;
    
    _finishedFrame = packet->frameNumber;
    getConfig()->notifyNodeFrameFinished( packet->frameNumber );

    return net::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Node& node )
{
    os << base::disableFlush << base::disableHeader;
    if( node.isApplicationNode( ))
        os << "appNode" << std::endl;
    else
        os << "node" << std::endl;

    os << "{" << std::endl << base::indent;

    const std::string& name = node.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const ConnectionDescriptions& descriptions = 
        node.getConnectionDescriptions();
    for( ConnectionDescriptions::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        ConnectionDescriptionPtr desc = *i;
        os << *desc;
    }

    bool attrPrinted   = false;
    
    for( Node::SAttribute i = static_cast<Node::SAttribute>( 0 );
         i<Node::SATTR_ALL; 
         i = static_cast<Node::SAttribute>( static_cast<uint32_t>( i )+1))
    {
        const std::string& value = node.getSAttribute( i );
        if( value == Global::instance()->getNodeSAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << base::indent;
            attrPrinted = true;
        }
        
        os << ( i==Node::SATTR_LAUNCH_COMMAND ? "launch_command       " :
                "ERROR" )
           << "\"" << value << "\"" << std::endl;
    }
    
    for( Node::CAttribute i = static_cast<Node::CAttribute>( 0 );
         i<Node::CATTR_ALL; 
         i = static_cast<Node::CAttribute>( static_cast<uint32_t>( i )+1))
    {
        const char value = node.getCAttribute( i );
        if( value == Global::instance()->getNodeCAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << base::indent;
            attrPrinted = true;
        }
        
        os << ( i==Node::CATTR_LAUNCH_COMMAND_QUOTE ? "launch_command_quote " :
                "ERROR" )
           << "'" << value << "'" << std::endl;
    }
    
    for( Node::IAttribute i = static_cast< Node::IAttribute>( 0 );
         i< Node::IATTR_ALL; 
         i = static_cast< Node::IAttribute >( static_cast<uint32_t>( i )+1))
    {
        const int value = node.getIAttribute( i );
        if( value == Global::instance()->getNodeIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << base::indent;
            attrPrinted = true;
        }
        
        os << ( i== Node::IATTR_LAUNCH_TIMEOUT ? "launch_timeout       " :
                i== Node::IATTR_THREAD_MODEL   ? "thread_model         " :
                "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << base::exdent << "}" << std::endl << std::endl;

    const Pipes& pipes = node.getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
        os << *i;

    os << base::exdent << "}" << std::endl
       << base::enableHeader << base::enableFlush;
    return os;
}

}
}

#include "../lib/fabric/node.ipp"
template class eq::fabric::Node< eq::server::Config, eq::server::Node,
                                 eq::server::Pipe, eq::server::NodeVisitor >;
