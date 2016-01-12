
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder<cedric.stalder@gmail.com>
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
#include "config.h"
#include "error.h"
#include "exception.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "nodeStatistics.h"
#include "pipe.h"
#include "server.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/frameData.h>
#include <eq/fabric/task.h>

#include <co/barrier.h>
#include <co/connection.h>
#include <co/global.h>
#include <co/objectICommand.h>
#include <lunchbox/scopedMutex.h>

namespace eq
{
namespace
{
typedef stde::hash_map< uint128_t, co::Barrier* > BarrierHash;
typedef stde::hash_map< uint128_t, FrameDataPtr > FrameDataHash;
typedef FrameDataHash::const_iterator FrameDataHashCIter;
typedef FrameDataHash::iterator FrameDataHashIter;

enum State
{
    STATE_STOPPED,
    STATE_INITIALIZING,
    STATE_INIT_FAILED,
    STATE_RUNNING,
    STATE_FAILED
};
}

namespace detail
{
class TransmitThread : public lunchbox::Thread
{
public:
    TransmitThread()
        : _queue( co::Global::getCommandQueueLimit( ))
    {}
    virtual ~TransmitThread() {}

    co::CommandQueue& getQueue() { return _queue; }

protected:
    bool init() override { setName( "Xmit" ); return true; }
    void run() override;

private:
    co::CommandQueue _queue;
};

class Node
{
public:
    Node()
        : state( STATE_STOPPED )
        , finishedFrame( 0 )
        , unlockedFrame( 0 )
    {}

    /** The configInit/configExit state. */
    lunchbox::Monitor< State > state;

    /** The number of the last started frame. */
    lunchbox::Monitor< uint32_t > currentFrame;

    /** The number of the last finished frame. */
    uint32_t finishedFrame;

    /** The number of the last locally released frame. */
    uint32_t unlockedFrame;

    /** All barriers mapped by the node. */
    lunchbox::Lockable< BarrierHash > barriers;

    /** All frame datas used by the node during rendering. */
    lunchbox::Lockable< FrameDataHash > frameDatas;

    TransmitThread transmitter;
};

}

/** @cond IGNORE */
typedef co::CommandFunc<Node> NodeFunc;
typedef fabric::Node< Config, Node, Pipe, NodeVisitor > Super;
/** @endcond */

Node::Node( Config* parent )
    : Super( parent )
    , _impl( new detail::Node )
{
}

Node::~Node()
{
    LBASSERT( getPipes().empty( ));
    delete _impl;
}

void Node::attach( const uint128_t& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* queue = getMainThreadQueue();
    co::CommandQueue* commandQ = getCommandThreadQueue();
    co::CommandQueue* transmitQ = getTransmitterQueue();

    registerCommand( fabric::CMD_NODE_CREATE_PIPE,
                     NodeFunc( this, &Node::_cmdCreatePipe ), queue );
    registerCommand( fabric::CMD_NODE_DESTROY_PIPE,
                     NodeFunc( this, &Node::_cmdDestroyPipe ), queue );
    registerCommand( fabric::CMD_NODE_CONFIG_INIT,
                     NodeFunc( this, &Node::_cmdConfigInit ), queue );
    registerCommand( fabric::CMD_NODE_SET_AFFINITY,
                     NodeFunc( this, &Node::_cmdSetAffinity), transmitQ );
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
    registerCommand( fabric::CMD_NODE_FRAMEDATA_TRANSMIT,
                     NodeFunc( this, &Node::_cmdFrameDataTransmit ), commandQ );
    registerCommand( fabric::CMD_NODE_FRAMEDATA_READY,
                     NodeFunc( this, &Node::_cmdFrameDataReady ), commandQ );
}

void Node::setDirty( const uint64_t bits )
{
    // jump over fabric setDirty to avoid dirty'ing config node list
    // nodes are individually synced in frame finish
    Object::setDirty( bits );
}

ClientPtr Node::getClient()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getClient() : 0 );
}

ServerPtr Node::getServer()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

co::CommandQueue* Node::getMainThreadQueue()
{
    return getConfig()->getMainThreadQueue();
}

co::CommandQueue* Node::getCommandThreadQueue()
{
    return getConfig()->getCommandThreadQueue();
}

co::CommandQueue* Node::getTransmitterQueue()
{
    return &_impl->transmitter.getQueue();
}

uint32_t Node::getCurrentFrame() const
{
    return _impl->currentFrame.get();
}

uint32_t Node::getFinishedFrame() const
{
    return _impl->finishedFrame;
}

co::Barrier* Node::getBarrier( const co::ObjectVersion& barrier )
{
    lunchbox::ScopedMutex<> mutex( _impl->barriers );
    co::Barrier* netBarrier = _impl->barriers.data[ barrier.identifier ];

    if( netBarrier )
        netBarrier->sync( barrier.version );
    else
    {
        ClientPtr client = getClient();

        netBarrier = new co::Barrier( client, barrier );
        if( !netBarrier->isGood( ))
        {
            LBCHECK( netBarrier->isGood( ));
            LBWARN << "Could not map swap barrier" << std::endl;
            delete netBarrier;
            return 0;
        }
        _impl->barriers.data[ barrier.identifier ] = netBarrier;
    }

    return netBarrier;
}

FrameDataPtr Node::getFrameData( const co::ObjectVersion& frameDataVersion )
{
    lunchbox::ScopedWrite mutex( _impl->frameDatas );
    FrameDataPtr data = _impl->frameDatas.data[ frameDataVersion.identifier ];

    if( !data )
    {
        data = new FrameData;
        data->setID( frameDataVersion.identifier );
        _impl->frameDatas.data[ frameDataVersion.identifier ] = data;
    }

    LBASSERT( frameDataVersion.version.high() == 0 );
    data->setVersion( frameDataVersion.version.low( ));
    return data;
}

void Node::releaseFrameData( FrameDataPtr data )
{
    lunchbox::ScopedWrite mutex( _impl->frameDatas );
    FrameDataHashIter i = _impl->frameDatas->find( data->getID( ));
    LBASSERT( i != _impl->frameDatas->end( ));
    if( i == _impl->frameDatas->end( ))
        return;

    _impl->frameDatas->erase( i );
}

void Node::waitInitialized() const
{
    _impl->state.waitGE( STATE_INIT_FAILED );
}

bool Node::isRunning() const
{
    return _impl->state == STATE_RUNNING;
}

bool Node::isStopped() const
{
    return _impl->state == STATE_STOPPED;
}

bool Node::configInit( const uint128_t& )
{
    WindowSystem::configInit( this );
    return true;
}

bool Node::configExit()
{
    WindowSystem::configExit( this );
    return true;
}

void Node::_setAffinity()
{
    const int32_t affinity = getIAttribute( IATTR_HINT_AFFINITY );
    switch( affinity )
    {
        case OFF:
            break;

        case AUTO:
            // TODO
            LBVERB << "No automatic thread placement for node threads "
                   << std::endl;
            break;

        default:
            co::LocalNodePtr node = getLocalNode();
            send( node, fabric::CMD_NODE_SET_AFFINITY ) << affinity;

            node->setAffinity( affinity );
            break;
    }
}

void Node::waitFrameStarted( const uint32_t frameNumber ) const
{
    _impl->currentFrame.waitGE( frameNumber );
}

void Node::startFrame( const uint32_t frameNumber )
{
    _impl->currentFrame = frameNumber;
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
        Pipe* pipe = *i;
        LBASSERT( pipe->isThreaded() || pipe->getFinishedFrame()>=frameNumber );

        pipe->waitFrameLocal( frameNumber );
        pipe->waitFrameFinished( frameNumber );
    }
}

void Node::_frameFinish( const uint128_t& frameID,
                         const uint32_t frameNumber )
{
    frameFinish( frameID, frameNumber );
    LBLOG( LOG_TASKS ) << "---- Finished Frame --- " << frameNumber
                       << std::endl;

    if( _impl->unlockedFrame < frameNumber )
    {
        LBWARN << "Finished frame was not locally unlocked, enforcing unlock"
               << std::endl;
        releaseFrameLocal( frameNumber );
    }

    if( _impl->finishedFrame < frameNumber )
    {
        LBWARN << "Finished frame was not released, enforcing unlock"
               << std::endl;
        releaseFrame( frameNumber );
    }
}

void Node::releaseFrame( const uint32_t frameNumber )
{
    LBASSERTINFO( _impl->currentFrame >= frameNumber,
                  "current " << _impl->currentFrame << " release " <<
                  frameNumber );

    if( _impl->finishedFrame >= frameNumber )
        return;
    _impl->finishedFrame = frameNumber;

    Config* config = getConfig();
    ServerPtr server = config->getServer();
    co::NodePtr node = server.get();
    send( node, fabric::CMD_NODE_FRAME_FINISH_REPLY ) << frameNumber;
}

void Node::releaseFrameLocal( const uint32_t frameNumber )
{
    LBASSERT( _impl->unlockedFrame <= frameNumber );
    _impl->unlockedFrame = frameNumber;

    Config* config = getConfig();
    LBASSERT( config->getNodes().size() == 1 );
    LBASSERT( config->getNodes()[0] == this );
    config->releaseFrameLocal( frameNumber );

    LBLOG( LOG_TASKS ) << "---- Unlocked Frame --- " << _impl->unlockedFrame
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
            LBUNIMPLEMENTED;
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
            LBUNIMPLEMENTED;
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
            LBUNIMPLEMENTED;
    }
}


EventOCommand Node::sendError( const uint32_t error )
{
    return getConfig()->sendError( Event::NODE_ERROR, Error( error, getID( )));
}

bool Node::processEvent( const Event& event )
{
    ConfigEvent configEvent( event );
    getConfig()->sendEvent( configEvent );
    return true;
}

void Node::_flushObjects()
{
    ClientPtr client = getClient();
    {
        lunchbox::ScopedMutex<> mutex( _impl->barriers );
        for( BarrierHash::const_iterator i = _impl->barriers->begin();
             i != _impl->barriers->end(); ++i )
        {
            delete i->second;
        }
        _impl->barriers->clear();
    }

    lunchbox::ScopedMutex<> mutex( _impl->frameDatas );
    for( FrameDataHashCIter i = _impl->frameDatas->begin();
         i != _impl->frameDatas->end(); ++i )
    {
        FrameDataPtr frameData = i->second;
        frameData->resetPlugins();
        client->unmapObject( frameData.get( ));
    }
    _impl->frameDatas->clear();
}

void detail::TransmitThread::run()
{
    while( true )
    {
        co::ICommand command = _queue.pop();
        if( !command.isValid( ))
            return; // exit thread

        LBCHECK( command( ));
    }
}

void Node::dirtyClientExit()
{
    const Pipes& pipes = getPipes();
    for( PipesCIter i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        pipe->cancelThread();
    }
    getTransmitterQueue()->push( co::ICommand( )); // wake up to exit
    _impl->transmitter.join();
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Node::_cmdCreatePipe( co::ICommand& cmd )
{
    LB_TS_THREAD( _nodeThread );
    LBASSERT( _impl->state >= STATE_INIT_FAILED );

    co::ObjectICommand command( cmd );
    const uint128_t& pipeID = command.read< uint128_t >();
    const bool threaded = command.read< bool >();

    LBLOG( LOG_INIT ) << "Create pipe " << command << " id " << pipeID
                      << std::endl;

    Pipe* pipe = Global::getNodeFactory()->createPipe( this );
    if( threaded )
        pipe->startThread();

    Config* config = getConfig();
    LBCHECK( config->mapObject( pipe, pipeID ));
    pipe->notifyMapped();

    return true;
}

bool Node::_cmdDestroyPipe( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LB_TS_THREAD( _nodeThread );
    LBLOG( LOG_INIT ) << "Destroy pipe " << command << std::endl;

    Pipe* pipe = findPipe( command.read< uint128_t >( ));
    LBASSERT( pipe );
    pipe->exitThread();

    const bool stopped = pipe->isStopped();

    Config* config = getConfig();
    config->unmapObject( pipe );
    pipe->send( getServer(), fabric::CMD_PIPE_CONFIG_EXIT_REPLY ) << stopped;
    Global::getNodeFactory()->releasePipe( pipe );

    return true;
}

bool Node::_cmdConfigInit( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LB_TS_THREAD( _nodeThread );
    LBLOG( LOG_INIT ) << "Init node " << command << std::endl;

    _impl->state = STATE_INITIALIZING;

    const uint128_t& initID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    _impl->currentFrame  = frameNumber;
    _impl->unlockedFrame = frameNumber;
    _impl->finishedFrame = frameNumber;
    _setAffinity();

    _impl->transmitter.start();
    const uint64_t result = configInit( initID );

    if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
        setIAttribute( IATTR_THREAD_MODEL, eq::DRAW_SYNC );

    _impl->state = result ? STATE_RUNNING : STATE_INIT_FAILED;

    commit();
    send( command.getRemoteNode(), fabric::CMD_NODE_CONFIG_INIT_REPLY )
            << result;
    return true;
}

bool Node::_cmdConfigExit( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LB_TS_THREAD( _nodeThread );
    LBLOG( LOG_INIT ) << "Node exit " << command << std::endl;

    const Pipes& pipes = getPipes();
    for( PipesCIter i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        pipe->waitExited();
    }

    _impl->state = configExit() ? STATE_STOPPED : STATE_FAILED;
    getTransmitterQueue()->push( co::ICommand( )); // wake up to exit
    _impl->transmitter.join();
    _flushObjects();

    getConfig()->send( getLocalNode(),
                       fabric::CMD_CONFIG_DESTROY_NODE ) << getID();
    return true;
}

bool Node::_cmdFrameStart( co::ICommand& cmd )
{
    LB_TS_THREAD( _nodeThread );

    co::ObjectICommand command( cmd );
    const uint128_t& version = command.read< uint128_t >();
    const uint128_t& configVersion = command.read< uint128_t >();
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBVERB << "handle node frame start " << command << " frame " << frameNumber
           << " id " << frameID << std::endl;

    LBASSERT( _impl->currentFrame == frameNumber-1 );

    LBLOG( LOG_TASKS ) << "----- Begin Frame ----- " << frameNumber
                       << std::endl;

    Config* config = getConfig();

    if( configVersion != co::VERSION_INVALID )
        config->sync( configVersion );
    sync( version );

    config->_frameStart();
    frameStart( frameID, frameNumber );

    LBASSERTINFO( _impl->currentFrame >= frameNumber,
                  "Node::frameStart() did not start frame " << frameNumber );
    return true;
}

bool Node::_cmdFrameFinish( co::ICommand& cmd )
{
    LB_TS_THREAD( _nodeThread );

    co::ObjectICommand command( cmd );
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << command
                       << " frame " << frameNumber << " id " << frameID
                       << std::endl;

    _finishFrame( frameNumber );
    _frameFinish( frameID, frameNumber );

    const uint128_t version = commit();
    if( version != co::VERSION_NONE )
        send( command.getNode(), fabric::CMD_OBJECT_SYNC );
    return true;
}

bool Node::_cmdFrameDrawFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << command
                       << " frame " << frameNumber << " id " << frameID
                       << std::endl;

    frameDrawFinish( frameID, frameNumber );
    return true;
}

bool Node::_cmdFrameTasksFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK tasks finish " << getName() <<  " " << command
                       << std::endl;

    frameTasksFinish( frameID, frameNumber );
    return true;
}

bool Node::_cmdFrameDataTransmit( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const co::ObjectVersion& frameDataVersion =
                                            command.read< co::ObjectVersion >();
    const PixelViewport& pvp = command.read< PixelViewport >();
    const Zoom& zoom = command.read< Zoom >();
    const uint32_t buffers = command.read< uint32_t >();
    const uint32_t frameNumber = command.read< uint32_t >();
    const bool useAlpha = command.read< bool >();
    const uint8_t* data = reinterpret_cast< const uint8_t* >(
                command.getRemainingBuffer( command.getRemainingBufferSize( )));

    LBLOG( LOG_ASSEMBLY )
        << "received image data for " << frameDataVersion << ", buffers "
        << buffers << " pvp " << pvp << std::endl;

    LBASSERT( pvp.isValid( ));

    FrameDataPtr frameData = getFrameData( frameDataVersion );
    LBASSERT( !frameData->isReady() );

    NodeStatistics event( Statistic::NODE_FRAME_DECOMPRESS, this,
                          frameNumber );

    // Note on the const_cast: since the PixelData structure stores non-const
    // pointers, we have to go non-const at some point, even though we do not
    // modify the data.
    LBCHECK( frameData->addImage( frameDataVersion, pvp, zoom, buffers,
                                  useAlpha, const_cast< uint8_t* >( data )));
    return true;
}

bool Node::_cmdFrameDataReady( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const co::ObjectVersion& frameDataVersion =
                                            command.read< co::ObjectVersion >();
    fabric::FrameData data;
    data.deserialize( command );

    LBLOG( LOG_ASSEMBLY ) << "received ready for " << frameDataVersion
                          << std::endl;
    FrameDataPtr frameData = getFrameData( frameDataVersion );
    LBASSERT( frameData );
    LBASSERT( !frameData->isReady() );
    frameData->setReady( frameDataVersion, data );
    LBASSERT( frameData->isReady() );
    return true;
}

bool Node::_cmdSetAffinity( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    lunchbox::Thread::setAffinity( command.read< int32_t >( ));
    return true;
}
}

#include <eq/fabric/node.ipp>
template class eq::fabric::Node< eq::Config, eq::Node, eq::Pipe,
                                 eq::NodeVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                             const eq::Super& );
/** @endcond */
