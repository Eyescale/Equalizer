
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
typedef fabric::Node< Config, Node, Pipe > Super;

#define MAKE_ATTRIB_STRING( attr ) ( std::string("EQ_NODE_") + #attr )

std::string Node::_sAttributeStrings[SATTR_ALL] = {
    MAKE_ATTRIB_STRING( SATTR_LAUNCH_COMMAND ),
    MAKE_ATTRIB_STRING( SATTR_FILL1 ),
    MAKE_ATTRIB_STRING( SATTR_FILL2 )
};
std::string Node::_cAttributeStrings[CATTR_ALL] = {
    MAKE_ATTRIB_STRING( CATTR_LAUNCH_COMMAND_QUOTE ),
    MAKE_ATTRIB_STRING( CATTR_FILL1 ),
    MAKE_ATTRIB_STRING( CATTR_FILL2 )
};

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
        _sattributes[i] = global->getNodeSAttribute((Node::SAttribute)i);
    for( int i=0; i < Node::CATTR_ALL; ++i )
        _cattributes[i] = global->getNodeCAttribute((Node::CAttribute)i);
    for( int i=0; i < eq::Node::IATTR_ALL; ++i )
        _iAttributes[i] = global->getNodeIAttribute((Node::IAttribute)i);
}

Node::~Node()
{
    EQINFO << "Delete node @" << (void*)this << std::endl;

    if( _config )
        _config->_removeNode( this );
    
    while( !_pipes.empty() )
    {
        Pipe* pipe = _pipes.back();
        EQASSERT( pipe->getNode() == this );
        _removePipe( pipe );
        delete pipe;
    }
    _pipes.clear();
}

void Node::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( CMD_NODE_CONFIG_INIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigInitReply ), queue );
    registerCommand( CMD_NODE_CONFIG_EXIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigExitReply ), queue );
    registerCommand( CMD_NODE_FRAME_FINISH_REPLY,
                     NodeFunc( this, &Node::_cmdFrameFinishReply ), queue );
}

Channel* Node::getChannel( const ChannelPath& path )
{
    EQASSERT( _pipes.size() > path.pipeIndex );

    if( _pipes.size() <= path.pipeIndex )
        return 0;

    return _pipes[ path.pipeIndex ]->getChannel( path );
}

namespace
{
template< class C >
VisitorResult _accept( C* node, NodeVisitor& visitor )
{ 
    VisitorResult result = visitor.visitPre( node );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const PipeVector& pipes = node->getPipes();
    for( PipeVector::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( node ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

VisitorResult Node::accept( NodeVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Node::accept( NodeVisitor& visitor ) const
{
    return _accept( this, visitor );
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

void Node::addTasks( const uint32_t tasks )
{
    setTasks( getTasks() | tasks );
}

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
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
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
        return true;

    // Sync state updates
    bool success = true;
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( !pipe->syncRunning( ))
        {
            setErrorMessage( getErrorMessage() +  "pipe " + pipe->getName() +
                             ": '" + pipe->getErrorMessage() + '\'' );
            success = false;
        }
    }

    flushSendBuffer();

    if( isActive() && _state != STATE_RUNNING && !_syncConfigInit( ))
        // becoming active
        success = false;

    if( !isActive() && !_syncConfigExit( ))
        // becoming inactive
        success = false;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_STOPPED );
    return success;
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

    EQASSERT( isMaster( ));
    commit();

    EQLOG( LOG_INIT ) << "Create node" << std::endl;
    ConfigCreateNodePacket createNodePacket;
    createNodePacket.nodeID = getID();
    createNodePacket.sessionID = _config->getID();
    _node->send( createNodePacket );

    EQLOG( LOG_INIT ) << "Init node" << std::endl;
    NodeConfigInitPacket packet;
    packet.initID      = initID;
    packet.tasks       = getTasks();
    packet.frameNumber = frameNumber;

    memcpy( packet.iAttributes, _iAttributes, 
            eq::Node::IATTR_ALL * sizeof( int32_t ));

    _send( packet, getName() );
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
    destroyNodePacket.sessionID = _config->getID();
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
    _send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK node start frame " << &startPacket << std::endl;

    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
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
        case DRAW_SYNC:
            if( getTasks() & fabric::TASK_DRAW )
            {
                // More than one frame latency doesn't make sense, since the
                // draw sync for frame+1 does not allow for more
                const Config* config = getConfig();
                const uint32_t latency = config->getLatency();

                return EQ_MIN( latency, 1 );
            }
            break;

        case LOCAL_SYNC:
            if( getTasks() != fabric::TASK_NONE )
                // local sync enforces no latency
                return 0;
            break;

        case ASYNC:
            break;
        default:
            EQUNIMPLEMENTED;
    }

    const Config* config = getConfig();
    return config->getLatency();
}

void Node::_finish( const uint32_t currentFrame )
{
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
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
    EQVERB << "Flush frames including " << frameNumber << std::endl;

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
        _config->registerObject( barrier );
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
        _config->deregisterObject( barrier );
        delete barrier;
    }
    _barriers.clear();
}

bool Node::removeConnectionDescription( ConnectionDescriptionPtr cd )
{
    ConnectionDescriptionVector::iterator i = 
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

    setErrorMessage( packet->error );
    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    memcpy( _iAttributes, packet->iAttributes, 
            eq::Node::IATTR_ALL * sizeof( int32_t ));

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
    _config->notifyNodeFrameFinished( packet->frameNumber );

    return net::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Node* node )
{
    if( !node )
        return os;
    
    os << base::disableFlush << base::disableHeader;
    const Config* config = node->getConfig();
    if( config && config->isApplicationNode( node ))
        os << "appNode" << std::endl;
    else
        os << "node" << std::endl;

    os << "{" << std::endl << base::indent;

    const std::string& name = node->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const ConnectionDescriptionVector& descriptions = 
        node->getConnectionDescriptions();
    for( ConnectionDescriptionVector::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )

        os << (*i).get();

    bool attrPrinted   = false;
    
    for( Node::SAttribute i = static_cast<Node::SAttribute>( 0 );
         i<Node::SATTR_ALL; 
         i = static_cast<Node::SAttribute>( static_cast<uint32_t>( i )+1))
    {
        const std::string& value = node->getSAttribute( i );
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
        const char value = node->getCAttribute( i );
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
        const int value = node->getIAttribute( i );
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
           << static_cast<IAttrValue>( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << base::exdent << "}" << std::endl << std::endl;

    const PipeVector& pipes = node->getPipes();
    for( PipeVector::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
        os << *i;

    os << base::exdent << "}" << std::endl
       << base::enableHeader << base::enableFlush;
    return os;
}

}
}

#include "../lib/fabric/node.cpp"
template class eq::fabric::Node< eq::server::Config, eq::server::Node,
                                 eq::server::Pipe >;
