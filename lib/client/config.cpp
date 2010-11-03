
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric Stalder@gmail.com> 
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

#include "config.h"

#include "canvas.h"
#include "channel.h"
#include "client.h"
#include "configEvent.h"
#include "configStatistics.h"
#include "configPackets.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "layout.h"
#include "log.h"
#include "messagePump.h"
#include "node.h"
#include "nodeFactory.h"
#include "nodePackets.h"
#include "observer.h"
#include "pipe.h"
#include "server.h"
#include "view.h"

#include <eq/fabric/configVisitor.h>
#include <eq/fabric/task.h>
#include <eq/net/object.h>
#include <eq/net/command.h>
#include <eq/net/global.h>

namespace eq
{
/** @cond IGNORE */
typedef net::CommandFunc<Config> ConfigFunc;
/** @endcond */

Config::Config( ServerPtr server )
        : Super( server )
        , _lastEvent( 0 )
        , _currentFrame( 0 )
        , _unlockedFrame( 0 )
        , _finishedFrame( 0 )
        , _running( false )
{
    base::Log::setClock( &_clock );
}

Config::~Config()
{
    EQASSERT( getObservers().empty( ));
    EQASSERT( getLayouts().empty( ));
    EQASSERT( getCanvases().empty( ));
    EQASSERT( getNodes().empty( ));
    EQASSERT( _latencyObjects->empty() );

    while( tryNextEvent( )) /* flush all pending events */ ;
    if( _lastEvent )
        _lastEvent->release();
    _eventQueue.flush();
    _lastEvent = 0;
    _appNode   = 0;
    base::Log::setClock( 0 );
}

void Config::notifyMapped( net::NodePtr node )
{
    Super::notifyMapped( node );

    ServerPtr          server = getServer();
    net::CommandQueue* queue  = server->getMainThreadQueue();

    registerCommand( fabric::CMD_CONFIG_CREATE_NODE,
                     ConfigFunc( this, &Config::_cmdCreateNode ), queue );
    registerCommand( fabric::CMD_CONFIG_DESTROY_NODE,
                     ConfigFunc( this, &Config::_cmdDestroyNode ), queue );
    registerCommand( fabric::CMD_CONFIG_INIT_REPLY, 
                     ConfigFunc( this, &Config::_cmdInitReply ), queue );
    registerCommand( fabric::CMD_CONFIG_EXIT_REPLY, 
                     ConfigFunc( this, &Config::_cmdExitReply ), queue );
    registerCommand( fabric::CMD_CONFIG_UPDATE_VERSION,
                     ConfigFunc( this, &Config::_cmdUpdateVersion ), 0 );
    registerCommand( fabric::CMD_CONFIG_UPDATE_REPLY,
                     ConfigFunc( this, &Config::_cmdUpdateReply ), queue );
    registerCommand( fabric::CMD_CONFIG_RELEASE_FRAME_LOCAL, 
                     ConfigFunc( this, &Config::_cmdReleaseFrameLocal ), queue);
    registerCommand( fabric::CMD_CONFIG_FRAME_FINISH, 
                     ConfigFunc( this, &Config::_cmdFrameFinish ), 0 );
    registerCommand( fabric::CMD_CONFIG_EVENT, 
                     ConfigFunc( this, &Config::_cmdUnknown ), &_eventQueue );
    registerCommand( fabric::CMD_CONFIG_SYNC_CLOCK, 
                     ConfigFunc( this, &Config::_cmdSyncClock ), 0 );
    registerCommand( fabric::CMD_CONFIG_SWAP_OBJECT,
                     ConfigFunc( this, &Config::_cmdSwapObject ), 0 );
}

net::CommandQueue* Config::getMainThreadQueue()
{
    return getClient()->getMainThreadQueue();
}

ClientPtr Config::getClient()
{ 
    return getServer()->getClient(); 
}

ConstClientPtr Config::getClient() const
{ 
    return getServer()->getClient(); 
}

void Config::unmap()
{
     {
        base::ScopedMutex< base::SpinLock > mutex( _latencyObjects );
        while( !_latencyObjects->empty() )
        {
            LatencyObject* latencyObject = _latencyObjects->back();
            _latencyObjects->pop_back();
            Session::deregisterObject( latencyObject );
            delete latencyObject;
            latencyObject = 0;
        }
    }
    _exitMessagePump();
    Super::unmap();
}

bool Config::init( const uint32_t initID )
{
    EQASSERT( !_running );
    _currentFrame = 0;
    _unlockedFrame = 0;
    _finishedFrame = 0;
    _frameTimes.clear();

    net::NodePtr localNode = getLocalNode();
    ConfigInitPacket packet;
    packet.requestID  = localNode->registerRequest();
    packet.initID     = initID;

    send( packet );
    
    ClientPtr client = getClient();
    while( !localNode->isRequestServed( packet.requestID ))
        client->processCommand();
    localNode->waitRequest( packet.requestID, _running );

    if( _running )
        handleEvents();
    else
        EQWARN << "Config initialization failed: " << getError() << std::endl;
    return _running;
}

bool Config::exit()
{
    finishAllFrames();

    net::NodePtr localNode = getLocalNode();
    ConfigExitPacket packet;
    packet.requestID = localNode->registerRequest();
    send( packet );

    ClientPtr client = getClient();
    while( !localNode->isRequestServed( packet.requestID ))
        client->processCommand();

    bool ret = false;
    localNode->waitRequest( packet.requestID, ret );

    while( tryNextEvent( )) /* flush all pending events */ ;
    if( _lastEvent )
        _lastEvent->release();
    _eventQueue.flush();
    _lastEvent = 0;
    _running = false;
    _appNode = 0;
    return ret;
}

bool Config::update()
{
    commit();

    // send update req to server
    ClientPtr client = getClient();

    ConfigUpdatePacket packet;    
    packet.versionID = client->registerRequest();
    packet.finishID = client->registerRequest();
    packet.requestID = client->registerRequest();
    send( packet );

    // wait for new version
    uint32_t version = net::VERSION_INVALID;
    client->waitRequest( packet.versionID, version );
    uint32_t finishID = 0;
    client->waitRequest( packet.finishID, finishID );

    if( finishID == EQ_ID_INVALID )
    {
        sync( version );
        client->unregisterRequest( packet.requestID );
        return true;
    }

    while( _finishedFrame < _currentFrame )
        client->processCommand();

    sync( version );
    ackRequest( getServer(), finishID );

    while( !client->isRequestServed( packet.requestID ))
        client->processCommand();

    bool result = false;
    client->waitRequest( packet.requestID, result );
    return result;
}

uint32_t Config::startFrame( const uint32_t frameID )
{
    ConfigStatistics stat( Statistic::CONFIG_START_FRAME, this );
    update();

    // Request new frame
    ClientPtr client = getClient();
    ConfigStartFramePacket packet( frameID );
    send( packet );

    ++_currentFrame;
    EQLOG( base::LOG_ANY ) << "---- Started Frame ---- " << _currentFrame
                           << std::endl;
    stat.event.data.statistic.frameNumber = _currentFrame;
    return _currentFrame;
}

void Config::_frameStart()
{
    _frameTimes.push_back( _clock.getTime64( ));
    while( _frameTimes.size() > getLatency() )
    {
        const int64_t age = _frameTimes.back() - _frameTimes.front();
        expireInstanceData( age );
        _frameTimes.pop_front();
    }
}

uint32_t Config::finishFrame()
{
    ClientPtr client = getClient();
    const uint32_t latency = getLatency();
    const uint32_t frameToFinish = (_currentFrame >= latency) ? 
                                      _currentFrame - latency : 0;

    ConfigStatistics stat( Statistic::CONFIG_FINISH_FRAME, this );
    stat.event.data.statistic.frameNumber = frameToFinish;
    {
        ConfigStatistics waitStat( Statistic::CONFIG_WAIT_FINISH_FRAME, this );
        waitStat.event.data.statistic.frameNumber = frameToFinish;
        
        // local draw sync
        if( _needsLocalSync( ))
            while( _unlockedFrame < _currentFrame )
                client->processCommand();

        // local node finish (frame-latency) sync
        const Nodes& nodes = getNodes();
        if( !nodes.empty( ))
        {
            EQASSERT( nodes.size() == 1 );
            const Node* node = nodes.front();

            while( node->getFinishedFrame() < frameToFinish )
                client->processCommand();
        }

        // global sync
        _finishedFrame.waitGE( frameToFinish );
    }

    handleEvents();
    _updateStatistics( frameToFinish );
    _releaseObjects();

    EQLOG( base::LOG_ANY ) << "---- Finished Frame --- " << frameToFinish
                           << " (" << _currentFrame << ')' << std::endl;
    return frameToFinish;
}

uint32_t Config::finishAllFrames()
{
    if( _finishedFrame == _currentFrame )
        return _currentFrame;

    EQLOG( base::LOG_ANY ) << "-- Finish All Frames --" << std::endl;
    ConfigFinishAllFramesPacket packet;
    send( packet );

    ClientPtr client = getClient();
    while( _finishedFrame < _currentFrame )
        client->processCommand();

    handleEvents();
    _updateStatistics( _currentFrame );
    _releaseObjects();
    EQLOG( base::LOG_ANY ) << "-- Finished All Frames --" << std::endl;
    return _currentFrame;
}

void Config::stopFrames()
{
    ConfigStopFramesPacket packet;
    send( packet );
}

namespace
{
class ChangeLatencyVisitor : public ConfigVisitor
{
public:
    ChangeLatencyVisitor( const uint32_t latency ) : _latency( latency ){}
    virtual ~ChangeLatencyVisitor() {}

    VisitorResult visit( eq::View* view )
    {
        net::Object* userData = view->getUserData();
        if( userData && userData->isMaster( ))
            userData->setAutoObsolete( _latency + 1 );
        return TRAVERSE_CONTINUE;    
    }

    VisitorResult visit( eq::Channel* channel )
    {
        channel->changeLatency( _latency );
        return TRAVERSE_CONTINUE;
    }

private:
    const uint32_t _latency; 
};
}

void Config::setLatency( const uint32_t latency )
{
    if( getLatency() == latency )
        return;

    Super::setLatency( latency );
    changeLatency( latency );
}

void Config::changeLatency( const uint32_t latency )
{
    finishAllFrames();
    commit();

    // update views
    ChangeLatencyVisitor changeLatencyVisitor( latency );
    accept( changeLatencyVisitor );
}

void Config::sendEvent( ConfigEvent& event )
{
    EQASSERT( event.data.type != Event::STATISTIC ||
              event.data.statistic.type != Statistic::NONE );
    EQASSERT( getAppNodeID() != net::NodeID::ZERO );

    if( !_appNode )
    {
        net::NodePtr localNode = getLocalNode();
        _appNode = localNode->connect( getAppNodeID( ));
    }
    EQASSERT( _appNode );

    event.sessionID = getID();
    _appNode->send( event );
}

const ConfigEvent* Config::nextEvent()
{
    if( _lastEvent )
        _lastEvent->release();
    _lastEvent = _eventQueue.pop();
    return _lastEvent->getPacket<ConfigEvent>();
}

const ConfigEvent* Config::tryNextEvent()
{
    net::Command* command = _eventQueue.tryPop();
    if( !command )
        return 0;

    if( _lastEvent )
        _lastEvent->release();
    _lastEvent = command;
    return command->getPacket<ConfigEvent>();
}

void Config::handleEvents()
{
    for( const ConfigEvent* event = tryNextEvent(); event; 
         event = tryNextEvent( ))
    {
        if( !handleEvent( event ))
            EQVERB << "Unhandled " << event << std::endl;
    }
}

bool Config::handleEvent( const ConfigEvent* event )
{
    switch( event->data.type )
    {
        case Event::EXIT:
        case Event::WINDOW_CLOSE:
            _running = false;
            return true;

        case Event::KEY_PRESS:
            if( event->data.keyPress.key == KC_ESCAPE )
            {
                _running = false;
                return true;
            }    
            break;

        case Event::POINTER_BUTTON_PRESS:
            if( event->data.pointerButtonPress.buttons == 
                ( PTR_BUTTON1 | PTR_BUTTON2 | PTR_BUTTON3 ))
            {
                _running = false;
                return true;
            }
            break;

        case Event::STATISTIC:
        {
            EQLOG( LOG_STATS ) << event->data << std::endl;

            const uint32_t originator = event->data.originator;
            EQASSERT( originator != EQ_ID_INVALID );
            if( originator == EQ_ID_INVALID )
                return false;

            const Statistic& statistic = event->data.statistic;
            const uint32_t   frame     = statistic.frameNumber;
            EQASSERT( statistic.type != Statistic::NONE )

            if( frame == 0 ||      // Not a frame-related stat event or
                statistic.type == Statistic::NONE ) // No event-type set
            {
                return false;
            }

            base::ScopedMutex<> mutex( _statisticsMutex );

            for( std::deque< FrameStatistics >::iterator i =_statistics.begin();
                 i != _statistics.end(); ++i )
            {
                FrameStatistics& frameStats = *i;
                if( frameStats.first == frame )
                {
                    SortedStatistics& sortedStats = frameStats.second;
                    Statistics&       statistics  = sortedStats[ originator ];
                    statistics.push_back( statistic );
                    return false;
                }
            }
            
            _statistics.push_back( FrameStatistics( ));
            FrameStatistics& frameStats = _statistics.back();
            frameStats.first = frame;

            SortedStatistics& sortedStats = frameStats.second;
            Statistics&       statistics  = sortedStats[ originator ];
            statistics.push_back( statistic );
            
            return false;
        }

        case Event::VIEW_RESIZE:
        {
            View* view = find< View >( event->data.originator );
            if( view )
                return view->handleEvent( event->data );
            break;
        }
    }

    return false;
}

bool Config::_needsLocalSync() const
{
    const Nodes& nodes = getNodes();
    if( nodes.empty( ))
        return true; // server sends unlock command - process it

    EQASSERT( nodes.size() == 1 );
    const Node* node = nodes.front();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:
            return false;
            break;

        case DRAW_SYNC:
            if( !(node->getTasks() & fabric::TASK_DRAW) )
                return false;
            break;

        case LOCAL_SYNC:
            if( node->getTasks() == fabric::TASK_NONE )
                return false;
            break;
                
        default:
            EQUNIMPLEMENTED;
    }

    return true;
}

void Config::_updateStatistics( const uint32_t finishedFrame )
{
    // keep statistics for three frames
    _statisticsMutex.set();
    while( !_statistics.empty() &&
           finishedFrame - _statistics.front().first > 2 )
    {
        _statistics.pop_front();
    }
    _statisticsMutex.unset();
}

void Config::getStatistics( std::vector< FrameStatistics >& statistics )
{
    _statisticsMutex.set();

    for( std::deque< FrameStatistics >::const_iterator i = _statistics.begin();
         i != _statistics.end(); ++i )
    {
        if( (*i).first <= _finishedFrame.get( ))
            statistics.push_back( *i );
    }

    _statisticsMutex.unset();
}

bool Config::mapViewObjects() const
{
    // only on application node...
    return ( getClient()->getNodeID() == getAppNodeID( ));
}

void Config::setupMessagePump( Pipe* pipe )
{
    const bool isThreaded = pipe->isThreaded();
    const WindowSystem windowSystem = pipe->getWindowSystem();

    if( isThreaded && windowSystem != WINDOW_SYSTEM_AGL )
        return;

    // called from pipe threads - but only during init
    static base::Lock _lock;
    base::ScopedMutex<> mutex( _lock );

    if( _eventQueue.getMessagePump( )) // Already done
        return;

    MessagePump* pump = pipe->createMessagePump();
    _eventQueue.setMessagePump( pump );

    ClientPtr client = getClient();
    CommandQueue* queue = EQSAFECAST( CommandQueue*, 
                                      client->getMainThreadQueue( ));
    EQASSERT( queue );
    EQASSERT( !queue->getMessagePump( ));

    queue->setMessagePump( pump );
}

void Config::_exitMessagePump()
{
    MessagePump* pump = _eventQueue.getMessagePump();

    _eventQueue.setMessagePump( 0 );

    ClientPtr client = getClient();
    CommandQueue* queue = EQSAFECAST( CommandQueue*, 
                                      client->getMainThreadQueue( ));
    EQASSERT( queue );
    EQASSERT( queue->getMessagePump() == pump );

    queue->setMessagePump( 0 );
    delete pump;
}

void Config::freezeLoadBalancing( const bool onOff )
{
    ConfigFreezeLoadBalancingPacket packet;
    packet.freeze = onOff;
    send( packet );
}

void Config::deregisterObject( net::Object* object )
{
    EQASSERT( object )
    EQASSERT( object->isMaster( ));

    const uint32_t latency = getLatency();

    if( latency == 0 || !_running || 
        object->getChangeType() == net::Object::STATIC || 
        object->getChangeType() == net::Object::UNBUFFERED ) // OPT
    {
        Session::deregisterObject( object );
        return;
    }

    const uint32_t id = object->getID();
    if( id >= EQ_ID_MAX ) // not registered
        return;

    // Keep a distributed object latency frames.
    // Replaces the object with a dummy proxy object using the
    // existing master change manager.
    ConfigSwapObjectPacket packet;
    packet.sessionID         = getID();
    packet.requestID         = getLocalNode()->registerRequest();
    packet.object            = object;

    getLocalNode()->send( packet );
    getLocalNode()->waitRequest( packet.requestID );
}

void Config::_releaseObjects()
{
    base::ScopedMutex< base::SpinLock > mutex( _latencyObjects );
    while( !_latencyObjects->empty() )
    {
        LatencyObject* latencyObject = _latencyObjects->front();

        if ( latencyObject->frameNumber > _currentFrame )
            break;

        Session::deregisterObject( latencyObject );
        _latencyObjects->erase( _latencyObjects->begin() );

        delete latencyObject;
    }
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Config::_cmdCreateNode( net::Command& command )
{
    const ConfigCreateNodePacket* packet = 
        command.getPacket<ConfigCreateNodePacket>();
    EQVERB << "Handle create node " << packet << std::endl;
    EQASSERT( packet->nodeID <= EQ_ID_MAX );

    Node* node = Global::getNodeFactory()->createNode( this );
    EQCHECK( mapObject( node, packet->nodeID ));

    return true;
}

bool Config::_cmdDestroyNode( net::Command& command ) 
{
    const ConfigDestroyNodePacket* packet =
        command.getPacket<ConfigDestroyNodePacket>();
    EQVERB << "Handle destroy node " << packet << std::endl;

    Node* node = _findNode( packet->nodeID );
    EQASSERT( node );
    if( !node )
        return true;

    NodeConfigExitReplyPacket reply( packet->nodeID, node->isStopped( ));

    EQASSERT( node->getPipes().empty( ));
    unmapObject( node );
    Global::getNodeFactory()->releaseNode( node );

    send( getServer(), reply );
    return true;
}

bool Config::_cmdInitReply( net::Command& command )
{
    const ConfigInitReplyPacket* packet = 
        command.getPacket<ConfigInitReplyPacket>();
    EQVERB << "handle init reply " << packet << std::endl;

    sync( packet->version );
    getLocalNode()->serveRequest( packet->requestID, (void*)(packet->result) );
    return true;
}

bool Config::_cmdExitReply( net::Command& command )
{
    const ConfigExitReplyPacket* packet = 
        command.getPacket<ConfigExitReplyPacket>();
    EQVERB << "handle exit reply " << packet << std::endl;

    _exitMessagePump();
    getLocalNode()->serveRequest( packet->requestID, (void*)(packet->result) );    
    return true;
}

bool Config::_cmdUpdateVersion( net::Command& command )
{
    const ConfigUpdateVersionPacket* packet = 
        command.getPacket<ConfigUpdateVersionPacket>();

    getClient()->serveRequest( packet->versionID, packet->version );
    getClient()->serveRequest( packet->finishID, packet->requestID );
    return true;
}

bool Config::_cmdUpdateReply( net::Command& command )
{
    const ConfigUpdateReplyPacket* packet = 
        command.getPacket<ConfigUpdateReplyPacket>();

    sync( packet->version );
    getClient()->serveRequest( packet->requestID, packet->result );
    return true;
}

bool Config::_cmdReleaseFrameLocal( net::Command& command )
{
    const ConfigReleaseFrameLocalPacket* packet =
        command.getPacket< ConfigReleaseFrameLocalPacket >();

    _frameStart(); // never happened from node
    releaseFrameLocal( packet->frameNumber );
    return true;
}

bool Config::_cmdFrameFinish( net::Command& command )
{
    const ConfigFrameFinishPacket* packet = 
        command.getPacket<ConfigFrameFinishPacket>();
    EQLOG( LOG_TASKS ) << "frame finish " << packet << std::endl;

    _finishedFrame = packet->frameNumber;

    if( _unlockedFrame < _finishedFrame.get( ))
    {
        EQWARN << "Finished frame " << _unlockedFrame 
               << " was not locally unlocked, enforcing unlock" << std::endl;
        _unlockedFrame = _finishedFrame.get();
    }

    getMainThreadQueue()->wakeup();
    return true;
}

bool Config::_cmdSyncClock( net::Command& command )
{
    const ConfigSyncClockPacket* packet = 
        command.getPacket< ConfigSyncClockPacket >();

    EQVERB << "sync global clock to " << packet->time << ", drift " 
           << packet->time - _clock.getTime64() << std::endl;

    _clock.set( packet->time );
    return true;
}

bool Config::_cmdSwapObject( net::Command& command )
{
    const ConfigSwapObjectPacket* packet = 
        command.getPacket<ConfigSwapObjectPacket>();
    EQVERB << "Cmd swap object " << packet << std::endl;

    net::Object* object = packet->object;

    LatencyObject* latencyObject = new LatencyObject( object->getChangeType( ));
    latencyObject->frameNumber   = _currentFrame + getLatency() + 1;

    swapObject( object, latencyObject  );
    
    base::ScopedMutex< base::SpinLock > mutex( _latencyObjects );
    _latencyObjects->push_back( latencyObject );

    EQASSERT( packet->requestID != EQ_ID_INVALID );
    getLocalNode()->serveRequest( packet->requestID );
    return true;
}
}

#include "../fabric/config.ipp"
template class eq::fabric::Config< eq::Server, eq::Config, eq::Observer,
                                   eq::Layout, eq::Canvas, eq::Node,
                                   eq::ConfigVisitor >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Config::Super& );
/** @endcond */

#define FIND_ID_TEMPLATE1( type )                                       \
    template EQ_EXPORT void eq::Config::Super::find< type >( const uint32_t, \
                                                             type** );

FIND_ID_TEMPLATE1( eq::Window );
FIND_ID_TEMPLATE1( eq::Channel );
FIND_ID_TEMPLATE1( eq::Layout );
FIND_ID_TEMPLATE1( eq::Observer );
FIND_ID_TEMPLATE1( eq::Canvas );

#define FIND_ID_TEMPLATE2( type )                                       \
    template EQ_EXPORT type* eq::Config::Super::find< type >( const uint32_t );

FIND_ID_TEMPLATE2( eq::Window );
FIND_ID_TEMPLATE2( eq::Observer );
FIND_ID_TEMPLATE2( eq::Layout );
FIND_ID_TEMPLATE2( eq::View );
FIND_ID_TEMPLATE2( eq::Canvas );

#define CONST_FIND_ID_TEMPLATE2( type )                                       \
    template EQ_EXPORT const type* eq::Config::Super::find< type >( const uint32_t ) const;

CONST_FIND_ID_TEMPLATE2( eq::Window );
CONST_FIND_ID_TEMPLATE2( eq::Observer );
CONST_FIND_ID_TEMPLATE2( eq::Layout );
CONST_FIND_ID_TEMPLATE2( eq::View );
CONST_FIND_ID_TEMPLATE2( eq::Canvas );

#define FIND_NAME_TEMPLATE1( type )                                     \
    template EQ_EXPORT void \
    eq::Config::Super::find< type >(const std::string&, const type** ) const;
FIND_NAME_TEMPLATE1( eq::Window );
FIND_NAME_TEMPLATE1( eq::Layout );
FIND_NAME_TEMPLATE1( eq::Observer );
FIND_NAME_TEMPLATE1( eq::Canvas );


#define CONST_FIND_NAME_TEMPLATE2( type )                               \
    template EQ_EXPORT const type*                                      \
    eq::Config::Super::find< type >( const std::string& ) const;

CONST_FIND_NAME_TEMPLATE2( eq::Window );
CONST_FIND_NAME_TEMPLATE2( eq::Canvas );
CONST_FIND_NAME_TEMPLATE2( eq::Channel );
CONST_FIND_NAME_TEMPLATE2( eq::Layout );
CONST_FIND_NAME_TEMPLATE2( eq::Observer );
CONST_FIND_NAME_TEMPLATE2( eq::View );

#define FIND_NAME_TEMPLATE2( type )                               \
    template EQ_EXPORT type*                                      \
    eq::Config::Super::find< type >( const std::string& );

FIND_NAME_TEMPLATE2( eq::Window );
FIND_NAME_TEMPLATE2( eq::Canvas );
FIND_NAME_TEMPLATE2( eq::Channel );
FIND_NAME_TEMPLATE2( eq::Layout );
FIND_NAME_TEMPLATE2( eq::Observer );
FIND_NAME_TEMPLATE2( eq::View );

