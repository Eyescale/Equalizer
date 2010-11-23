
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder<cedric.stalder@gmail.com>
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

#include "node.h"

#include "client.h"
#include "channel.h"
#include "config.h"
#include "configPackets.h"
#include "error.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "nodePackets.h"
#include "pipe.h"
#include "pipePackets.h"
#include "server.h"

#ifdef AGL
#  include "aglEventHandler.h"
#elif defined WGL
#  include "wglEventHandler.h"
#endif

#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/task.h>
#include <eq/net/barrier.h>
#include <eq/net/command.h>
#include <eq/net/connection.h>
#include <eq/base/scopedMutex.h>

namespace eq
{
/** @cond IGNORE */
typedef net::CommandFunc<Node> NodeFunc;
typedef fabric::Node< Config, Node, Pipe, NodeVisitor > Super;
/** @endcond */

Node::Node( Config* parent )
        : Super( parent )
#pragma warning( push )
#pragma warning( disable : 4355 )
        , transmitter( this )
#pragma warning( push )
        , _state( STATE_STOPPED )
        , _finishedFrame( 0 )
        , _unlockedFrame( 0 )
{
}

Node::~Node()
{
    EQASSERT( getPipes().empty( ));
}

void Node::attachToSession( const uint32_t id, 
                            const uint32_t instanceID, 
                            net::Session* session )
{
    Super::attachToSession( id, instanceID, session );

    Config* config = getConfig();
    EQASSERT( config );
    net::CommandQueue* queue = config->getMainThreadQueue();

    registerCommand( fabric::CMD_NODE_CREATE_PIPE, 
                     NodeFunc( this, &Node::_cmdCreatePipe ), queue );
    registerCommand( fabric::CMD_NODE_DESTROY_PIPE,
                     NodeFunc( this, &Node::_cmdDestroyPipe ), queue );
    registerCommand( fabric::CMD_NODE_CONFIG_INIT, 
                     NodeFunc( this, &Node::_cmdConfigInit ), queue );
    registerCommand( fabric::CMD_NODE_CONFIG_EXIT,
                     NodeFunc( this, &Node::_cmdConfigExit ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_START,
                     NodeFunc( this, &Node::_cmdFrameStart ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_FINISH,
                     NodeFunc( this, &Node::_cmdFrameFinish ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_DRAW_FINISH, 
                     NodeFunc( this, &Node::_cmdFrameDrawFinish ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_TASKS_FINISH, 
                     NodeFunc( this, &Node::_cmdFrameTasksFinish ), queue );
}

ClientPtr Node::getClient()
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getClient() : 0 );
}

ServerPtr Node::getServer()
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

net::CommandQueue* Node::getMainThreadQueue()
{
    return getClient()->getMainThreadQueue();
}

net::Barrier* Node::getBarrier( const net::ObjectVersion barrier )
{
    base::ScopedMutex<> mutex( _barriers );
    net::Barrier* netBarrier = _barriers.data[ barrier.identifier ];

    if( netBarrier )
        netBarrier->sync( barrier.version );
    else
    {
        net::Session* session = getSession();

        netBarrier = new net::Barrier;
        EQCHECK( session->mapObject( netBarrier, barrier ));

        _barriers.data[ barrier.identifier ] = netBarrier;
    }

    return netBarrier;
}

FrameData* Node::getFrameData( const net::ObjectVersion& dataVersion )
{
    base::ScopedMutex<> mutex( _frameDatas );
    FrameData* frameData = _frameDatas.data[ dataVersion.identifier ];

    if( !frameData )
    {
        net::Session* session = getSession();
        
        frameData = new FrameData;
        EQCHECK( session->mapObject( frameData, dataVersion ));
        frameData->update( dataVersion.version );

        _frameDatas.data[ dataVersion.identifier ] = frameData;
        return frameData;
    }

    if( frameData->getVersion() < dataVersion.version )
    {
        frameData->sync( dataVersion.version );
        frameData->update( dataVersion.version );
    }
    EQASSERT( frameData->getVersion() == dataVersion.version );

    return frameData;
}

void Node::waitInitialized() const
{
    _state.waitGE( STATE_INIT_FAILED );
}

bool Node::isRunning() const
{
    return (_state == STATE_RUNNING);
}

bool Node::isStopped() const
{
    return (_state == STATE_STOPPED);
}

bool Node::configInit( const uint32_t )
{
#ifdef EQ_USE_MAGELLAN
#  ifdef AGL
    AGLEventHandler::initMagellan( this );
#  elif defined WGL
    WGLEventHandler::initMagellan( this );
#  else
    EQUNIMPLEMENTED;
#  endif
#endif
    return true;
}

bool Node::configExit()
{
#ifdef EQ_USE_MAGELLAN
#  ifdef AGL
    AGLEventHandler::exitMagellan( this );
#  elif defined WGL
    WGLEventHandler::exitMagellan( this );
#  else
    EQUNIMPLEMENTED;
#  endif
#endif
    return true;
}

void Node::waitFrameStarted( const uint32_t frameNumber ) const
{
    _currentFrame.waitGE( frameNumber );
}

void Node::startFrame( const uint32_t frameNumber ) 
{
    _currentFrame = frameNumber;
}

void Node::frameFinish( const uint128_t&, const uint32_t frameNumber ) 
{
    releaseFrame( frameNumber );
}

void Node::_finishFrame( const uint32_t frameNumber ) const
{
    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        const Pipe* pipe = *i;
        EQASSERT( pipe->isThreaded() || 
                  pipe->getFinishedFrame() >= frameNumber );

        pipe->waitFrameLocal( frameNumber );
        pipe->waitFrameFinished( frameNumber );
    }
}

void Node::_frameFinish( const uint128_t& frameID, 
                         const uint32_t frameNumber )
{
    frameFinish( frameID, frameNumber );
    EQLOG( LOG_TASKS ) << "---- Finished Frame --- " << frameNumber
                       << std::endl;

    if( _unlockedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not locally unlocked, enforcing unlock" 
               << std::endl;
        releaseFrameLocal( frameNumber );
    }

    if( _finishedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not released, enforcing unlock"
               << std::endl;
        releaseFrame( frameNumber );
    }
}

void Node::releaseFrame( const uint32_t frameNumber )
{
    EQASSERTINFO( _currentFrame >= frameNumber, 
                  "current " << _currentFrame << " release " << frameNumber );

    if( _finishedFrame >= frameNumber )
        return;
    _finishedFrame = frameNumber;

    NodeFrameFinishReplyPacket packet;
    packet.frameNumber = frameNumber;

    Config* config = getConfig();
    ServerPtr server = config->getServer();
    net::NodePtr node = server.get();
    send( node, packet );
}

void Node::releaseFrameLocal( const uint32_t frameNumber )
{
    EQASSERT( _unlockedFrame <= frameNumber );
    _unlockedFrame = frameNumber;
    
    Config* config = getConfig();
    EQASSERT( config->getNodes().size() == 1 );
    EQASSERT( config->getNodes()[0] == this );
    config->releaseFrameLocal( frameNumber );

    EQLOG( LOG_TASKS ) << "---- Unlocked Frame --- " << _unlockedFrame
                       << std::endl;
}

void Node::frameStart( const uint128_t&, const uint32_t frameNumber )
{
    startFrame( frameNumber ); // unlock pipe threads
    
    switch( getIAttribute( IATTR_THREAD_MODEL ))
    {
        case ASYNC:
            // Don't wait for pipes to release frame locally, sync not needed
            releaseFrameLocal( frameNumber );
            break;

        case DRAW_SYNC:  // Sync and release in frameDrawFinish
        case LOCAL_SYNC: // Sync and release in frameTasksFinish
            break;

        default:
            EQUNIMPLEMENTED;
    }
}

void Node::frameDrawFinish( const uint128_t&, const uint32_t frameNumber )
{
    switch( getIAttribute( IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // No sync, release in frameStart
        case LOCAL_SYNC: // Sync and release in frameTasksFinish
            break;

        case DRAW_SYNC:
        {
            const Pipes& pipes = getPipes();
            for( Pipes::const_iterator i = pipes.begin();
                 i != pipes.end(); ++i )
            {
                const Pipe* pipe = *i;
                if( pipe->getTasks() & fabric::TASK_DRAW )
                    pipe->waitFrameLocal( frameNumber );
            }
            
            releaseFrameLocal( frameNumber );
            break;
        }
        default:
            EQUNIMPLEMENTED;
    }
}

void Node::frameTasksFinish( const uint128_t&, const uint32_t frameNumber )
{
    switch( getIAttribute( IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // No sync, release in frameStart
        case DRAW_SYNC:  // Sync and release in frameDrawFinish
            break;

        case LOCAL_SYNC:
        {
            const Pipes& pipes = getPipes();
            for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i)
            {
                const Pipe* pipe = *i;
                if( pipe->getTasks() != fabric::TASK_NONE )
                    pipe->waitFrameLocal( frameNumber );
            }
            
            releaseFrameLocal( frameNumber );
            break;
        }
        default:
            EQUNIMPLEMENTED;
    }
}

void Node::_flushObjects()
{
    net::Session* session = getSession();
    {
        base::ScopedMutex<> mutex( _barriers );
        for( BarrierHash::const_iterator i =_barriers->begin();
             i != _barriers->end(); ++ i )
        {
            net::Barrier* barrier = i->second;
            session->unmapObject( barrier );
            delete barrier;
        }
        _barriers->clear();
    }

    base::ScopedMutex<> mutex( _frameDatas );
    for( FrameDataHash::const_iterator i = _frameDatas->begin(); 
         i != _frameDatas->end(); ++ i )
    {
        FrameData* frameData = i->second;
        session->unmapObject( frameData );
        delete frameData;
    }
    _frameDatas->clear();
}

Node::TransmitThread::Task::Task( FrameData* d, net::NodePtr n,
                                  const uint32_t f, const uint32_t i,
                                  const uint32_t t, Channel* c )
    : data( d )
    , node( n )
    , frameNumber( f )
    , index( i )
    , renderTaskID( t )
    , channel( c ){}

void Node::TransmitThread::send( FrameData* data, net::NodePtr node,
                                 const uint32_t frameNumber, 
                                 const uint32_t index,
                                 const uint32_t renderTaskID,
                                 Channel* channel )
{
    _tasks.push( Task( data, node, frameNumber, index, renderTaskID, channel ));
}

void Node::TransmitThread::run()
{
    base::Thread::setDebugName( std::string( "Trm " ) + typeid( *_node).name());
    while( true )
    {
        const Task task = _tasks.pop();
        if( _tasks.isEmpty() && !task.node )
            return; // exit thread
        
        EQLOG( LOG_ASSEMBLY ) << "node transmit " << task.data->getID()
                              << " to " << task.node->getNodeID() << std::endl;
        task.channel->transmit( task.node, task.frameNumber, 
                                task.data, task.index, task.renderTaskID );
    }
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Node::_cmdCreatePipe( net::Command& command )
{
    const NodeCreatePipePacket* packet = 
        command.getPacket<NodeCreatePipePacket>();
    EQLOG( LOG_INIT ) << "Create pipe " << packet << std::endl;

    EQ_TS_THREAD( _nodeThread );
    EQASSERT( packet->pipeID <= EQ_ID_MAX );

    Pipe* pipe = Global::getNodeFactory()->createPipe( this );

    if( packet->threaded )
        pipe->startThread();

    Config* config = getConfig();
    EQCHECK( config->mapObject( pipe, packet->pipeID ));
    pipe->notifyMapped();

    return true;
}

bool Node::_cmdDestroyPipe( net::Command& command )
{
    EQ_TS_THREAD( _nodeThread );

    const NodeDestroyPipePacket* packet = 
        command.getPacket< NodeDestroyPipePacket >();
    EQLOG( LOG_INIT ) << "Destroy pipe " << packet << std::endl;

    Pipe* pipe = findPipe( packet->pipeID );
    EQASSERT( pipe );
    pipe->joinThread();

    PipeConfigExitReplyPacket reply( packet->pipeID, pipe->isStopped( ));

    Config* config = getConfig();
    config->unmapObject( pipe );
    Global::getNodeFactory()->releasePipe( pipe );

    config->send( getServer(), reply ); // do not use Object::send()
    return true;
}

bool Node::_cmdConfigInit( net::Command& command )
{
    EQ_TS_THREAD( _nodeThread );

    const NodeConfigInitPacket* packet = 
        command.getPacket<NodeConfigInitPacket>();
    EQLOG( LOG_INIT ) << "Init node " << packet << std::endl;

    _state = STATE_INITIALIZING;

    _currentFrame  = packet->frameNumber;
    _unlockedFrame = packet->frameNumber;
    _finishedFrame = packet->frameNumber;

    transmitter.start();
    setError( ERROR_NONE );
    NodeConfigInitReplyPacket reply;
    reply.result = configInit( packet->initID );

    if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
        setIAttribute( IATTR_THREAD_MODEL, eq::DRAW_SYNC );

    _state = reply.result ? STATE_RUNNING : STATE_INIT_FAILED;

    commit();
    send( command.getNode(), reply );
    return true;
}

bool Node::_cmdConfigExit( net::Command& command )
{
    const NodeConfigExitPacket* packet = 
        command.getPacket<NodeConfigExitPacket>();
    EQLOG( LOG_INIT ) << "Node exit " << packet << std::endl;

    EQ_TS_THREAD( _nodeThread );
    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        pipe->waitExited();
    }
    
    _state = configExit() ? STATE_STOPPED : STATE_FAILED;
    transmitter.send( 0, 0, 0, 0, 0, 0 );
    transmitter.join();
    _flushObjects();

    ConfigDestroyNodePacket destroyPacket( getID( ));
    getConfig()->send( getLocalNode(), destroyPacket );
    return true;
}

bool Node::_cmdFrameStart( net::Command& command )
{
    EQ_TS_THREAD( _nodeThread );
    const NodeFrameStartPacket* packet = 
        command.getPacket<NodeFrameStartPacket>();
    EQVERB << "handle node frame start " << packet << std::endl;

    const uint32_t frameNumber = packet->frameNumber;
    EQASSERT( _currentFrame == frameNumber-1 );

    EQLOG( LOG_TASKS ) << "----- Begin Frame ----- " << frameNumber
                       << std::endl;

    Config* config = getConfig();
    
    if( packet->configVersion != net::VERSION_INVALID )
        config->sync( packet->configVersion );
    sync( packet->version );

    config->_frameStart();
    frameStart( packet->frameID, frameNumber );

    EQASSERTINFO( _currentFrame >= frameNumber, 
                  "Node::frameStart() did not start frame " << frameNumber );
    return true;
}

bool Node::_cmdFrameFinish( net::Command& command )
{
    EQ_TS_THREAD( _nodeThread );
    const NodeFrameFinishPacket* packet = 
        command.getPacket<NodeFrameFinishPacket>();
    EQLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << packet
                       << std::endl;

    const uint32_t frameNumber = packet->frameNumber;

    _finishFrame( frameNumber );
    _frameFinish( packet->frameID, frameNumber );
    commit();
    return true;
}

bool Node::_cmdFrameDrawFinish( net::Command& command )
{
    NodeFrameDrawFinishPacket* packet = 
        command.getPacket< NodeFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return true;
}

bool Node::_cmdFrameTasksFinish( net::Command& command )
{
    NodeFrameTasksFinishPacket* packet = 
        command.getPacket< NodeFrameTasksFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK tasks finish " << getName() <<  " " << packet
                       << std::endl;

    frameTasksFinish( packet->frameID, packet->frameNumber );
    return true;
}
}


#include "../fabric/node.ipp"
template class eq::fabric::Node< eq::Config, eq::Node, eq::Pipe,
                                 eq::NodeVisitor >;
/** @cond IGNORE */
template EQFABRIC_EXPORT std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
