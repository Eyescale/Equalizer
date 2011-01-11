
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Cedric Stalder <cedric Stalder@gmail.com> 
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
#include <co/object.h>
#include <co/command.h>
#include <co/connectionDescription.h>
#include <co/global.h>

namespace eq
{
/** @cond IGNORE */
typedef co::CommandFunc<Config> ConfigFunc;
/** @endcond */

Config::Config( ServerPtr server )
        : Super( server )
        , _lastEvent( 0 )
        , _currentFrame( 0 )
        , _unlockedFrame( 0 )
        , _finishedFrame( 0 )
        , _running( false )
{
    co::base::Log::setClock( &_clock );
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
    co::base::Log::setClock( 0 );
}

void Config::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    ServerPtr          server = getServer();
    co::CommandQueue* queue  = getMainThreadQueue();

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
    registerCommand( fabric::CMD_CONFIG_EVENT, ConfigFunc( 0, 0 ),
                     &_eventQueue );
    registerCommand( fabric::CMD_CONFIG_SYNC_CLOCK, 
                     ConfigFunc( this, &Config::_cmdSyncClock ), 0 );
    registerCommand( fabric::CMD_CONFIG_SWAP_OBJECT,
                     ConfigFunc( this, &Config::_cmdSwapObject ), 0 );
}

void Config::notifyAttached()
{
    fabric::Object::notifyAttached();
    EQASSERT( !_appNode )
    EQASSERT( getAppNodeID().isGenerated() )
    co::LocalNodePtr localNode = getLocalNode();
    _appNode = localNode->connect( getAppNodeID( ));
    EQASSERTINFO( _appNode, "Connection to application node failed -- " <<
                            "misconfigured connections on appNode?" );
}

void Config::notifyDetach()
{
    {
        ClientPtr client = getClient();
        co::base::ScopedMutex< co::base::SpinLock > mutex( _latencyObjects );
        while( !_latencyObjects->empty() )
        {
            LatencyObject* latencyObject = _latencyObjects->back();
            _latencyObjects->pop_back();
            client->deregisterObject( latencyObject );
            delete latencyObject;
            latencyObject = 0;
        }
    }

    std::vector< uint32_t > requests;
    for( co::Connections::const_iterator i = _connections.begin();
         i != _connections.end(); ++i )
    {
        co::ConnectionPtr connection = *i;
        requests.push_back( getClient()->removeListenerNB( connection ));
    }

    co::LocalNodePtr localNode = getLocalNode();
    for( size_t i = 0; i < _connections.size(); ++i )
    {
        co::ConnectionPtr connection = _connections[i];
        localNode->waitRequest( requests[ i ] );
        connection->close();
        // connection and _connections hold reference
        EQASSERTINFO( connection->getRefCount()==2, connection->getRefCount( ));
    }

    _connections.clear();
    _exitMessagePump();
    Super::notifyDetach();
}

co::CommandQueue* Config::getMainThreadQueue()
{
    return getServer()->getMainThreadQueue();
}

co::CommandQueue* Config::getCommandThreadQueue()
{
    return getServer()->getCommandThreadQueue();
}

ClientPtr Config::getClient()
{ 
    return getServer()->getClient(); 
}

ConstClientPtr Config::getClient() const
{ 
    return getServer()->getClient(); 
}

bool Config::init( const uint128_t& initID )
{
    EQASSERT( !_running );
    _currentFrame = 0;
    _unlockedFrame = 0;
    _finishedFrame = 0;
    _frameTimes.clear();

    co::LocalNodePtr localNode = getLocalNode();
    ConfigInitPacket packet;
    packet.requestID  = localNode->registerRequest();
    packet.initID     = initID;

    send( getServer(), packet );
    
    ClientPtr client = getClient();
    while( !localNode->isRequestServed( packet.requestID ))
        client->processCommand();
    localNode->waitRequest( packet.requestID, _running );

    if( _running )
        handleEvents();
    else
        EQWARN << "Config initialization failed: " << getError() << std::endl
               << "    Consult client log for further information" << std::endl;
    return _running;
}

bool Config::exit()
{
    update();
    finishAllFrames();

    co::LocalNodePtr localNode = getLocalNode();
    ConfigExitPacket packet;
    packet.requestID = localNode->registerRequest();
    send( getServer(), packet );

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
    send( getServer(), packet );

    // wait for new version
    uint128_t version = co::VERSION_INVALID;
    client->waitRequest( packet.versionID, version );
    uint32_t finishID = 0;
    client->waitRequest( packet.finishID, finishID );

    if( finishID == EQ_UNDEFINED_UINT32 )
    {
        sync( version );
        client->unregisterRequest( packet.requestID );
        return true;
    }

    while( _finishedFrame < _currentFrame )
        client->processCommand();

    sync( version );
    client->ackRequest( getServer(), finishID );

    while( !client->isRequestServed( packet.requestID ))
        client->processCommand();

    bool result = false;
    client->waitRequest( packet.requestID, result );
    return result;
}

uint32_t Config::startFrame( const uint128_t& frameID )
{
    ConfigStatistics stat( Statistic::CONFIG_START_FRAME, this );
    update();

    // Request new frame
    ClientPtr client = getClient();
    ConfigStartFramePacket packet( frameID );
    send( getServer(), packet );

    ++_currentFrame;
    EQLOG( co::base::LOG_ANY ) << "---- Started Frame ---- " << _currentFrame
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
        getClient()->expireInstanceData( age );
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

    EQLOG( co::base::LOG_ANY ) << "---- Finished Frame --- " << frameToFinish
                           << " (" << _currentFrame << ')' << std::endl;
    return frameToFinish;
}

uint32_t Config::finishAllFrames()
{
    if( _finishedFrame == _currentFrame )
        return _currentFrame;

    EQLOG( co::base::LOG_ANY ) << "-- Finish All Frames --" << std::endl;
    ConfigFinishAllFramesPacket packet;
    send( getServer(), packet );

    ClientPtr client = getClient();
    while( _finishedFrame < _currentFrame )
        client->processCommand();

    handleEvents();
    _updateStatistics( _currentFrame );
    _releaseObjects();
    EQLOG( co::base::LOG_ANY ) << "-- Finished All Frames --" << std::endl;
    return _currentFrame;
}

void Config::stopFrames()
{
    ConfigStopFramesPacket packet;
    send( getServer(), packet );
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
        co::Object* userData = view->getUserData();
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
    EQASSERT( getAppNodeID() != co::NodeID::ZERO );
    EQASSERT( _appNode.isValid( ));

    send( _appNode, event );
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
    co::Command* command = _eventQueue.tryPop();
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

        case Event::WINDOW_POINTER_BUTTON_PRESS:
        case Event::CHANNEL_POINTER_BUTTON_PRESS:
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

            const uint128_t& originator = event->data.originator;
            EQASSERT( originator != co::base::UUID::ZERO );
            if( originator == co::base::UUID::ZERO )
                return false;

            const Statistic& statistic = event->data.statistic;
            const uint32_t   frame     = statistic.frameNumber;
            EQASSERT( statistic.type != Statistic::NONE )

            if( frame == 0 ||      // Not a frame-related stat event or
                statistic.type == Statistic::NONE ) // No event-type set
            {
                return false;
            }

            co::base::ScopedMutex<> mutex( _statisticsMutex );

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
            EQASSERT( event->data.originator != co::base::UUID::ZERO );
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
    static co::base::Lock _lock;
    co::base::ScopedMutex<> mutex( _lock );

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

MessagePump* Config::getMessagePump()
{
    ClientPtr client = getClient();
    CommandQueue* queue = EQSAFECAST( CommandQueue*, 
                                      client->getMainThreadQueue( ));
    if( queue )
        return queue->getMessagePump();
    return 0;
}

void Config::setupServerConnections( const char* connectionData )
{
    std::string data = connectionData;
    co::ConnectionDescriptions descriptions;
    EQCHECK( co::deserialize( data, descriptions ));
    EQASSERTINFO( data.empty(), data << " left from " << connectionData );

    for( co::ConnectionDescriptions::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        co::ConnectionPtr connection = co::Connection::create( *i );
        if( connection->listen( ))
        {
            _connections.push_back( connection );
            getClient()->addListener( connection );
        }
        else
        {
            // TODO: Multi-config handling when same connections are spec'd
            EQASSERT( connection->isListening( ));
        }
    }
}

void Config::freezeLoadBalancing( const bool onOff )
{
    ConfigFreezeLoadBalancingPacket packet;
    packet.freeze = onOff;
    send( getServer(), packet );
}

bool Config::registerObject( co::Object* object )
{
    return getClient()->registerObject( object );
}

void Config::deregisterObject( co::Object* object )
{
    EQASSERT( object )
    EQASSERT( object->isMaster( ));

    const uint32_t latency = getLatency();
    ClientPtr client = getClient();
    if( latency == 0 || !_running || 
        object->getChangeType() == co::Object::STATIC || 
        object->getChangeType() == co::Object::UNBUFFERED ) // OPT
    {
        client->deregisterObject( object );
        return;
    }

    if( !object->isAttached() ) // not registered
        return;

    // Keep a distributed object latency frames.
    // Replaces the object with a dummy proxy object using the
    // existing master change manager.
    ConfigSwapObjectPacket packet;
    packet.requestID         = getLocalNode()->registerRequest();
    packet.object            = object;

    send( client, packet );
    client->waitRequest( packet.requestID );
}

bool Config::mapObject( co::Object* object, const UUID& id,
                        const uint128_t& version )
{
    return mapObjectSync( mapObjectNB( object, id, version ));
}

uint32_t Config::mapObjectNB( co::Object* object, const co::base::UUID& id, 
                              const uint128_t& version )
{
    return getClient()->mapObjectNB( object, id, version );
}

bool Config::mapObjectSync( const uint32_t requestID )
{
    return getClient()->mapObjectSync( requestID );
}

void Config::unmapObject( co::Object* object )
{
    getClient()->unmapObject( object );
}

void Config::releaseObject( co::Object* object )
{
    if( !object->isAttached( ))
        return;
    if( object->isMaster( ))
        deregisterObject( object );
    else
        unmapObject( object );
}

void Config::_releaseObjects()
{
    ClientPtr client = getClient();

    co::base::ScopedMutex< co::base::SpinLock > mutex( _latencyObjects );
    while( !_latencyObjects->empty() )
    {
        LatencyObject* latencyObject = _latencyObjects->front();
        if ( latencyObject->frameNumber > _currentFrame )
            break;

        client->deregisterObject( latencyObject );
        _latencyObjects->erase( _latencyObjects->begin() );

        delete latencyObject;
    }
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Config::_cmdCreateNode( co::Command& command )
{
    const ConfigCreateNodePacket* packet = 
        command.getPacket<ConfigCreateNodePacket>();
    EQVERB << "Handle create node " << packet << std::endl;

    Node* node = Global::getNodeFactory()->createNode( this );
    EQCHECK( mapObject( node, packet->nodeID ));
    return true;
}

bool Config::_cmdDestroyNode( co::Command& command ) 
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

    getServer()->send( reply );
    return true;
}

bool Config::_cmdInitReply( co::Command& command )
{
    const ConfigInitReplyPacket* packet = 
        command.getPacket<ConfigInitReplyPacket>();
    EQVERB << "handle init reply " << packet << std::endl;

    sync( packet->version );
    getLocalNode()->serveRequest( packet->requestID, (void*)(packet->result) );
    return true;
}

bool Config::_cmdExitReply( co::Command& command )
{
    const ConfigExitReplyPacket* packet = 
        command.getPacket<ConfigExitReplyPacket>();
    EQVERB << "handle exit reply " << packet << std::endl;

    _exitMessagePump();
    getLocalNode()->serveRequest( packet->requestID, (void*)(packet->result) );
    return true;
}

bool Config::_cmdUpdateVersion( co::Command& command )
{
    const ConfigUpdateVersionPacket* packet = 
        command.getPacket<ConfigUpdateVersionPacket>();

    getClient()->serveRequest( packet->versionID, packet->version );
    getClient()->serveRequest( packet->finishID, packet->requestID );
    return true;
}

bool Config::_cmdUpdateReply( co::Command& command )
{
    const ConfigUpdateReplyPacket* packet = 
        command.getPacket<ConfigUpdateReplyPacket>();

    sync( packet->version );
    getClient()->serveRequest( packet->requestID, packet->result );
    return true;
}

bool Config::_cmdReleaseFrameLocal( co::Command& command )
{
    const ConfigReleaseFrameLocalPacket* packet =
        command.getPacket< ConfigReleaseFrameLocalPacket >();

    _frameStart(); // never happened from node
    releaseFrameLocal( packet->frameNumber );
    return true;
}

bool Config::_cmdFrameFinish( co::Command& command )
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

bool Config::_cmdSyncClock( co::Command& command )
{
    const ConfigSyncClockPacket* packet = 
        command.getPacket< ConfigSyncClockPacket >();

    EQVERB << "sync global clock to " << packet->time << ", drift " 
           << packet->time - _clock.getTime64() << std::endl;

    _clock.set( packet->time );
    return true;
}

bool Config::_cmdSwapObject( co::Command& command )
{
    const ConfigSwapObjectPacket* packet = 
        command.getPacket<ConfigSwapObjectPacket>();
    EQVERB << "Cmd swap object " << packet << std::endl;

    co::Object* object = packet->object;

    LatencyObject* latencyObject = new LatencyObject( object->getChangeType( ));
    latencyObject->frameNumber   = _currentFrame + getLatency() + 1;

    getLocalNode()->swapObject( object, latencyObject  );
    
    co::base::ScopedMutex< co::base::SpinLock > mutex( _latencyObjects );
    _latencyObjects->push_back( latencyObject );

    EQASSERT( packet->requestID != EQ_UNDEFINED_UINT32 );
    getLocalNode()->serveRequest( packet->requestID );
    return true;
}
}

#include "../fabric/config.ipp"
#include "../fabric/view.ipp"
#include "../fabric/observer.ipp"
template class eq::fabric::Config< eq::Server, eq::Config, eq::Observer,
                                   eq::Layout, eq::Canvas, eq::Node,
                                   eq::ConfigVisitor >;
/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Config::Super& );
/** @endcond */

#define FIND_ID_TEMPLATE1( type )                                       \
    template EQ_API void eq::Config::Super::find< type >( const uint128_t&, \
                                                             type** );

FIND_ID_TEMPLATE1( eq::Window );
FIND_ID_TEMPLATE1( eq::Channel );
FIND_ID_TEMPLATE1( eq::Layout );
FIND_ID_TEMPLATE1( eq::Observer );
FIND_ID_TEMPLATE1( eq::Canvas );

#define FIND_ID_TEMPLATE2( type )                                       \
    template EQ_API type* eq::Config::Super::find< type >( const uint128_t& );

FIND_ID_TEMPLATE2( eq::Window );
FIND_ID_TEMPLATE2( eq::Observer );
FIND_ID_TEMPLATE2( eq::Layout );
FIND_ID_TEMPLATE2( eq::View );
FIND_ID_TEMPLATE2( eq::Canvas );

#define CONST_FIND_ID_TEMPLATE2( type )                                       \
    template EQ_API const type* eq::Config::Super::find< type >( const uint128_t& ) const;

CONST_FIND_ID_TEMPLATE2( eq::Window );
CONST_FIND_ID_TEMPLATE2( eq::Observer );
CONST_FIND_ID_TEMPLATE2( eq::Layout );
CONST_FIND_ID_TEMPLATE2( eq::View );
CONST_FIND_ID_TEMPLATE2( eq::Canvas );

#define FIND_NAME_TEMPLATE1( type )                                     \
    template EQ_API void \
    eq::Config::Super::find< type >(const std::string&, const type** ) const;
FIND_NAME_TEMPLATE1( eq::Window );
FIND_NAME_TEMPLATE1( eq::Layout );
FIND_NAME_TEMPLATE1( eq::Observer );
FIND_NAME_TEMPLATE1( eq::Canvas );


#define CONST_FIND_NAME_TEMPLATE2( type )                               \
    template EQ_API const type*                                      \
    eq::Config::Super::find< type >( const std::string& ) const;

CONST_FIND_NAME_TEMPLATE2( eq::Window );
CONST_FIND_NAME_TEMPLATE2( eq::Canvas );
CONST_FIND_NAME_TEMPLATE2( eq::Channel );
CONST_FIND_NAME_TEMPLATE2( eq::Layout );
CONST_FIND_NAME_TEMPLATE2( eq::Observer );
CONST_FIND_NAME_TEMPLATE2( eq::View );

#define FIND_NAME_TEMPLATE2( type )                               \
    template EQ_API type*                                      \
    eq::Config::Super::find< type >( const std::string& );

FIND_NAME_TEMPLATE2( eq::Window );
FIND_NAME_TEMPLATE2( eq::Canvas );
FIND_NAME_TEMPLATE2( eq::Channel );
FIND_NAME_TEMPLATE2( eq::Layout );
FIND_NAME_TEMPLATE2( eq::Observer );
FIND_NAME_TEMPLATE2( eq::View );
