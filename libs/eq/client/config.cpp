
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "global.h"
#include "layout.h"
#include "log.h"
#include "messagePump.h"
#include "node.h"
#include "nodeFactory.h"
#include "observer.h"
#include "pipe.h"
#include "server.h"
#include "view.h"
#include "window.h"

#include <eq/fabric/configVisitor.h>
#include <eq/fabric/commands.h>
#include <eq/fabric/task.h>

#include <co/exception.h>
#include <co/object.h>
#include <co/connectionDescription.h>
#include <co/global.h>
#include <co/objectCommand.h>
#include <lunchbox/scopedMutex.h>

namespace eq
{
namespace
{
/** A proxy object to keep master data within latency. */
class LatencyObject : public co::Object
{
public:
    LatencyObject( const ChangeType type, const uint32_t compressor,
                   const uint32_t frame )
            : frameNumber( frame ), _changeType( type )
            , _compressor( compressor ) {}

    const uint32_t frameNumber;

protected:
    virtual ChangeType getChangeType() const { return _changeType; }
    virtual void getInstanceData( co::DataOStream& os ){ LBDONTCALL }
    virtual void applyInstanceData( co::DataIStream& is ){ LBDONTCALL }
    virtual uint32_t chooseCompressor() const { return _compressor; }

private:
    const ChangeType _changeType;
    const uint32_t _compressor;
};
}

namespace detail
{
class Config
{
public:
    Config()
            : lastEvent( 0 )
            , currentFrame( 0 )
            , unlockedFrame( 0 )
            , finishedFrame( 0 )
            , running( false )
    {
        lunchbox::Log::setClock( &clock );
    }

    ~Config()
    {
        delete lastEvent;
        appNode = 0;
        lunchbox::Log::setClock( 0 );
    }

    /** The node running the application thread. */
    co::NodePtr appNode;

    /** The receiver->app thread event queue. */
    CommandQueue eventQueue;

    /** The last received event to be released. */
    ConfigEvent* lastEvent;

    /** The connections configured by the server for this config. */
    co::Connections connections;

    /** Global statistics events, index per frame and channel. */
    lunchbox::Lockable< std::deque< FrameStatistics >, lunchbox::SpinLock >
        statistics;

    /** The last started frame. */
    uint32_t currentFrame;
    /** The last locally released frame. */
    uint32_t unlockedFrame;
    /** The last completed frame. */
    lunchbox::Monitor< uint32_t > finishedFrame;

    /** The global clock. */
    lunchbox::Clock clock;

    std::deque< int64_t > frameTimes; //!< Start time of last frames

    /** list of the current latency object */
    typedef std::vector< LatencyObject* > LatencyObjects;

    /** protected list of the current latency object */
    lunchbox::Lockable< LatencyObjects, lunchbox::SpinLock > latencyObjects;

    /** true while the config is initialized and no window has exited. */
    bool running;
};
}

/** @cond IGNORE */
typedef co::CommandFunc<Config> ConfigFunc;
/** @endcond */

Config::Config( ServerPtr server )
        : Super( server )
        , _impl( new detail::Config )
{}

Config::~Config()
{
    LBASSERT( getObservers().empty( ));
    LBASSERT( getLayouts().empty( ));
    LBASSERT( getCanvases().empty( ));
    LBASSERT( getNodes().empty( ));
    LBASSERT( _impl->latencyObjects->empty() );

    _impl->eventQueue.flush();
    delete _impl;
}

void Config::attach( const UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    ServerPtr server = getServer();
    co::CommandQueue* queue = getMainThreadQueue();

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
                     &_impl->eventQueue );
    registerCommand( fabric::CMD_CONFIG_SYNC_CLOCK,
                     ConfigFunc( this, &Config::_cmdSyncClock ), 0 );
    registerCommand( fabric::CMD_CONFIG_SWAP_OBJECT,
                     ConfigFunc( this, &Config::_cmdSwapObject ), 0 );
}

void Config::notifyAttached()
{
    fabric::Object::notifyAttached();
    LBASSERT( !_impl->appNode )
    LBASSERT( getAppNodeID().isGenerated() )
    co::LocalNodePtr localNode = getLocalNode();
    _impl->appNode = localNode->connect( getAppNodeID( ));
    if( !_impl->appNode )
        LBWARN << "Connection to application node failed -- misconfigured "
               << "connections on appNode?" << std::endl;
}

void Config::notifyDetach()
{
    {
        ClientPtr client = getClient();
        lunchbox::ScopedFastWrite mutex( _impl->latencyObjects );
        while( !_impl->latencyObjects->empty() )
        {
            LatencyObject* latencyObject = _impl->latencyObjects->back();
            _impl->latencyObjects->pop_back();
            client->deregisterObject( latencyObject );
            delete latencyObject;
            latencyObject = 0;
        }
    }

    getClient()->removeListeners( _impl->connections );
    _impl->connections.clear();
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

co::NodePtr Config::getApplicationNode()
{
    return _impl->appNode;
}

namespace
{
class SetDefaultVisitor : public ConfigVisitor
{
public:
    SetDefaultVisitor( const Strings& activeLayouts, const float modelUnit )
            : _layouts( activeLayouts ), _modelUnit( modelUnit )
            , _update( false ) {}

    virtual VisitorResult visit( View* view )
        {
            if( view->setModelUnit( _modelUnit ))
                _update = true;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visitPre( Canvas* canvas )
        {
            const Layouts& layouts = canvas->getLayouts();

            for( StringsCIter i = _layouts.begin(); i != _layouts.end(); ++i )
            {
                const std::string& name = *i;
                for( LayoutsCIter j = layouts.begin(); j != layouts.end(); ++j )
                {
                    const Layout* layout = *j;
                    if( layout->getName() == name &&
                        canvas->useLayout( j - layouts.begin( )))
                    {
                        _update = true;
                    }
                }
            }
            return TRAVERSE_CONTINUE;
        }

    bool needsUpdate() const { return _update; }

private:
    const Strings& _layouts;
    const float _modelUnit;
    bool _update;
};
}

bool Config::init( const uint128_t& initID )
{
    LBASSERT( !_impl->running );
    _impl->currentFrame = 0;
    _impl->unlockedFrame = 0;
    _impl->finishedFrame = 0;
    _impl->frameTimes.clear();

    ClientPtr client = getClient();
    SetDefaultVisitor defaults( client->getActiveLayouts(),
                                client->getModelUnit( ));
    accept( defaults );
    if( defaults.needsUpdate( ))
        update();

    co::LocalNodePtr localNode = getLocalNode();
    const uint32_t requestID = localNode->registerRequest();
    send( getServer(), fabric::CMD_CONFIG_INIT ) << initID << requestID;

    while( !localNode->isRequestServed( requestID ))
        client->processCommand();
    localNode->waitRequest( requestID, _impl->running );
    localNode->enableSendOnRegister();

    if( _impl->running )
        handleEvents();
    else
        LBWARN << "Config initialization failed: " << getError() << std::endl
               << "    Consult client log for further information" << std::endl;
    return _impl->running;
}

bool Config::exit()
{
    update();
    finishAllFrames();

    co::LocalNodePtr localNode = getLocalNode();
    localNode->disableSendOnRegister();

    const uint32_t requestID = localNode->registerRequest();
    send( getServer(), fabric::CMD_CONFIG_EXIT ) << requestID;

    ClientPtr client = getClient();
    while( !localNode->isRequestServed( requestID ))
        client->processCommand();

    bool ret = false;
    localNode->waitRequest( requestID, ret );

    delete _impl->lastEvent;
    _impl->lastEvent = 0;
    _impl->eventQueue.flush();
    _impl->running = false;
    return ret;
}

bool Config::update()
{
    commit( CO_COMMIT_NEXT );

    // send update req to server
    ClientPtr client = getClient();

    const uint32_t reqVersionID = client->registerRequest();
    const uint32_t reqFinishID = client->registerRequest();
    const uint32_t requestID = client->registerRequest();
    send( getServer(), fabric::CMD_CONFIG_UPDATE )
            << reqVersionID << reqFinishID << requestID;

    // wait for new version
    uint128_t version = co::VERSION_INVALID;
    client->waitRequest( reqVersionID, version );
    uint32_t finishID = 0;
    client->waitRequest( reqFinishID, finishID );

    if( finishID == LB_UNDEFINED_UINT32 )
    {
        sync( version );
        client->unregisterRequest( requestID );
        return true;
    }

    client->disableSendOnRegister();
    while( _impl->finishedFrame < _impl->currentFrame )
        client->processCommand();

    sync( version );
    client->ackRequest( getServer(), finishID );

    while( !client->isRequestServed( requestID ))
        client->processCommand();

    bool result = false;
    client->waitRequest( requestID, result );
    client->enableSendOnRegister();
    return result;
}

uint32_t Config::startFrame( const uint128_t& frameID )
{
    ConfigStatistics stat( Statistic::CONFIG_START_FRAME, this );
    update();

    // Request new frame
    send( getServer(), fabric::CMD_CONFIG_START_FRAME ) << frameID;

    ++_impl->currentFrame;
    LBLOG( lunchbox::LOG_ANY ) << "---- Started Frame ---- "
                               << _impl->currentFrame << std::endl;
    stat.event.statistic.frameNumber = _impl->currentFrame;
    return _impl->currentFrame;
}

void Config::_frameStart()
{
    _impl->frameTimes.push_back( _impl->clock.getTime64( ));
    while( _impl->frameTimes.size() > getLatency() )
    {
        const int64_t age = _impl->frameTimes.back() -_impl->frameTimes.front();
        getClient()->expireInstanceData( age );
        _impl->frameTimes.pop_front();
    }
}

uint32_t Config::finishFrame()
{
    ClientPtr client = getClient();
    const uint32_t latency = getLatency();
    const uint32_t frameToFinish = (_impl->currentFrame >= latency) ?
                                       _impl->currentFrame - latency : 0;

    ConfigStatistics stat( Statistic::CONFIG_FINISH_FRAME, this );
    stat.event.statistic.frameNumber = frameToFinish;
    {
        ConfigStatistics waitStat( Statistic::CONFIG_WAIT_FINISH_FRAME, this );
        waitStat.event.statistic.frameNumber = frameToFinish;

        // local draw sync
        if( _needsLocalSync( ))
        {
            while( _impl->unlockedFrame < _impl->currentFrame )
                client->processCommand();
            LBLOG( LOG_TASKS ) << "Local frame sync " << _impl->currentFrame
                               << std::endl;
        }

        // local node finish (frame-latency) sync
        const Nodes& nodes = getNodes();
        if( !nodes.empty( ))
        {
            LBASSERT( nodes.size() == 1 );
            const Node* node = nodes.front();

            while( node->getFinishedFrame() < frameToFinish )
                client->processCommand();
            LBLOG( LOG_TASKS ) << "Local total sync " << frameToFinish
                               << " @ " << _impl->currentFrame << std::endl;
        }

        // global sync
        const uint32_t timeout = getTimeout();
        if( timeout == LB_TIMEOUT_INDEFINITE )
            _impl->finishedFrame.waitGE( frameToFinish );
        else
        {
            const int64_t pingTimeout = co::Global::getKeepaliveTimeout();
            const int64_t time = getTime() + timeout;

            while( !_impl->finishedFrame.timedWaitGE( frameToFinish,
                                                      pingTimeout ))
            {
                if( getTime() >= time || !getLocalNode()->pingIdleNodes( ))
                {
                    LBWARN << "Timeout waiting for nodes to finish frame "
                           << frameToFinish << std::endl;
                    break;
                }
            }
        }
        LBLOG( LOG_TASKS ) << "Global sync " << frameToFinish << " @ "
                           << _impl->currentFrame << std::endl;
    }

    handleEvents();
    _updateStatistics( frameToFinish );
    _releaseObjects();

    LBLOG( lunchbox::LOG_ANY )
        << "---- Finished Frame --- " << frameToFinish << " ("
        << _impl->currentFrame << ')' << std::endl;
    return frameToFinish;
}

uint32_t Config::finishAllFrames()
{
    if( _impl->finishedFrame == _impl->currentFrame )
        return _impl->currentFrame;

    LBLOG( lunchbox::LOG_ANY ) << "-- Finish All Frames --" << std::endl;
    send( getServer(), fabric::CMD_CONFIG_FINISH_ALL_FRAMES );

    ClientPtr client = getClient();
    const uint32_t timeout = getTimeout();
    while( _impl->finishedFrame < _impl->currentFrame )
    {
        try
        {
            client->processCommand( timeout );
        }
        catch( const co::Exception& e )
        {
            LBWARN << e.what() << std::endl;
            break;
        }
    }
    handleEvents();
    _updateStatistics( _impl->currentFrame );
    _releaseObjects();
    LBLOG( lunchbox::LOG_ANY ) << "-- Finished All Frames --" << std::endl;
    return _impl->currentFrame;
}

void Config::releaseFrameLocal( const uint32_t frameNumber )
{
    _impl->unlockedFrame = frameNumber;
}

void Config::stopFrames()
{
    send( getServer(), fabric::CMD_CONFIG_STOP_FRAMES );
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
    commit( CO_COMMIT_NEXT );

    // update views
    ChangeLatencyVisitor changeLatencyVisitor( latency );
    accept( changeLatencyVisitor );
}

co::ObjectOCommand Config::sendEvent( const Event& event )
{
    LBASSERT( event.type != Event::STATISTIC ||
              event.statistic.type != Statistic::NONE );
    LBASSERT( getAppNodeID() != co::NodeID::ZERO );
    LBASSERT( _impl->appNode );

    co::ObjectOCommand cmd( send( _impl->appNode, fabric::CMD_CONFIG_EVENT ));
    cmd << event;
    return cmd;
}

co::ObjectOCommand Config::sendEvent( const uint32_t eventType )
{
    Event event;
    event.type = eventType;
    return sendEvent( event );
}

const ConfigEvent* Config::nextEvent()
{
    delete _impl->lastEvent;
    _impl->lastEvent = new ConfigEvent( _impl->eventQueue.pop( ));
    return _impl->lastEvent;
}

const ConfigEvent* Config::tryNextEvent()
{
    const co::Command& command = _impl->eventQueue.tryPop();
    if( !command.isValid( ))
        return 0;

    delete _impl->lastEvent;
    _impl->lastEvent = new ConfigEvent( command );
    return _impl->lastEvent;
}

bool Config::checkEvent() const
{
    return !_impl->eventQueue.isEmpty();
}

void Config::handleEvents()
{
    for( const ConfigEvent* event = tryNextEvent(); event; event=tryNextEvent())
        handleEvent( event );
}

bool Config::handleEvent( const ConfigEvent* event )
{
    switch( event->data.type )
    {
        case Event::EXIT:
        case Event::WINDOW_CLOSE:
            _impl->running = false;
            return true;

        case Event::KEY_PRESS:
            if( event->data.keyPress.key == KC_ESCAPE )
            {
                _impl->running = false;
                return true;
            }
            break;

        case Event::WINDOW_POINTER_BUTTON_PRESS:
        case Event::CHANNEL_POINTER_BUTTON_PRESS:
            if( event->data.pointerButtonPress.buttons ==
                ( PTR_BUTTON1 | PTR_BUTTON2 | PTR_BUTTON3 ))
            {
                _impl->running = false;
                return true;
            }
            break;

        case Event::STATISTIC:
        {
            LBLOG( LOG_STATS ) << event->data << std::endl;

            const uint32_t originator = event->data.serial;
            LBASSERTINFO( originator != EQ_INSTANCE_INVALID, event->data );
            if( originator == 0 )
                return false;

            const Statistic& statistic = event->data.statistic;
            const uint32_t   frame     = statistic.frameNumber;
            LBASSERT( statistic.type != Statistic::NONE )

            if( frame == 0 ||      // Not a frame-related stat event or
                statistic.type == Statistic::NONE ) // No event-type set
            {
                return false;
            }

            lunchbox::ScopedFastWrite mutex( _impl->statistics );

            for( std::deque<FrameStatistics>::iterator i =
                     _impl->statistics->begin();
                 i != _impl->statistics->end(); ++i )
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

            _impl->statistics->push_back( FrameStatistics( ));
            FrameStatistics& frameStats = _impl->statistics->back();
            frameStats.first = frame;

            SortedStatistics& sortedStats = frameStats.second;
            Statistics&       statistics  = sortedStats[ originator ];
            statistics.push_back( statistic );

            return false;
        }

        case Event::VIEW_RESIZE:
        {
            LBASSERT( event->data.originator != UUID::ZERO );
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

    LBASSERT( nodes.size() == 1 );
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
            LBUNIMPLEMENTED;
    }

    return true;
}

void Config::_updateStatistics( const uint32_t finishedFrame )
{
    // keep statistics for three frames
    lunchbox::ScopedMutex< lunchbox::SpinLock > mutex( _impl->statistics );
    while( !_impl->statistics->empty() &&
           finishedFrame - _impl->statistics->front().first > 2 )
    {
        _impl->statistics->pop_front();
    }
}

void Config::getStatistics( std::vector< FrameStatistics >& statistics )
{
    lunchbox::ScopedMutex< lunchbox::SpinLock > mutex( _impl->statistics );

    for( std::deque<FrameStatistics>::const_iterator i =
             _impl->statistics->begin(); i != _impl->statistics->end(); ++i )
    {
        if( (*i).first <= _impl->finishedFrame.get( ))
            statistics.push_back( *i );
    }
}

uint32_t Config::getCurrentFrame() const
{
    return _impl->currentFrame;
}

uint32_t Config::getFinishedFrame() const
{
    return _impl->finishedFrame.get();
}

bool Config::isRunning() const
{
    return _impl->running;
}

void Config::stopRunning()
{
    _impl->running = false;
}

int64_t Config::getTime() const
{
    return _impl->clock.getTime64();
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

    if( isThreaded && windowSystem.getName() != "AGL" )
        return;

    // called from pipe threads - but only during init
    static lunchbox::Lock _lock;
    lunchbox::ScopedWrite mutex( _lock );

    if( _impl->eventQueue.getMessagePump( )) // Already done
        return;

    MessagePump* pump = pipe->createMessagePump();
    _impl->eventQueue.setMessagePump( pump );

    ClientPtr client = getClient();
    CommandQueue* queue = LBSAFECAST( CommandQueue*,
                                      client->getMainThreadQueue( ));
    LBASSERT( queue );
    LBASSERT( !queue->getMessagePump( ));

    queue->setMessagePump( pump );
}

void Config::_exitMessagePump()
{
    MessagePump* pump = _impl->eventQueue.getMessagePump();
    _impl->eventQueue.setMessagePump( 0 );

    ClientPtr client = getClient();
    CommandQueue* queue = LBSAFECAST( CommandQueue*,
                                      client->getMainThreadQueue( ));
    LBASSERT( queue );
    LBASSERT( queue->getMessagePump() == pump );

    queue->setMessagePump( 0 );
    delete pump;
}

MessagePump* Config::getMessagePump()
{
    ClientPtr client = getClient();
    CommandQueue* queue = LBSAFECAST( CommandQueue*,
                                      client->getMainThreadQueue( ));
    if( queue )
        return queue->getMessagePump();
    return 0;
}

void Config::setupServerConnections( const char* connectionData )
{
    std::string data = connectionData;
    co::ConnectionDescriptions descriptions;
    LBCHECK( co::deserialize( data, descriptions ));
    LBASSERTINFO( data.empty(), data << " left from " << connectionData );

    for( co::ConnectionDescriptionsCIter i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        co::ConnectionPtr connection = getClient()->addListener( *i );
        if( connection )
            _impl->connections.push_back( connection );
    }
}

bool Config::registerObject( co::Object* object )
{
    if( !getClient()->registerObject( object ))
        return false;
    object->setAutoObsolete( getLatency() + 1 );
    return true;
}

void Config::deregisterObject( co::Object* object )
{
    LBASSERT( object )
    LBASSERT( object->isMaster( ));

    if( !object->isAttached( )) // not registered
        return;

    const uint32_t latency = getLatency();
    ClientPtr client = getClient();
    if( latency == 0 || !_impl->running || !object->isBuffered( )) // OPT
    {
        client->deregisterObject( object );
        return;
    }

    // Keep a distributed object latency frames.
    // Replaces the object with a dummy proxy object using the
    // existing master change manager.
    const uint32_t requestID = getLocalNode()->registerRequest();
    send( client, fabric::CMD_CONFIG_SWAP_OBJECT ) << requestID << object;
    client->waitRequest( requestID );
}

bool Config::mapObject( co::Object* object, const UUID& id,
                        const uint128_t& version )
{
    return mapObjectSync( mapObjectNB( object, id, version ));
}

uint32_t Config::mapObjectNB( co::Object* object, const UUID& id,
                              const uint128_t& version )
{
    return getClient()->mapObjectNB( object, id, version );
}

uint32_t Config::mapObjectNB( co::Object* object, const UUID& id,
                              const uint128_t& version, co::NodePtr master )
{
    return getClient()->mapObjectNB( object, id, version, master );
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

    lunchbox::ScopedFastWrite mutex( _impl->latencyObjects );
    while( !_impl->latencyObjects->empty() )
    {
        LatencyObject* latencyObject = _impl->latencyObjects->front();
        if( latencyObject->frameNumber > _impl->currentFrame )
            break;

        client->deregisterObject( latencyObject );
        _impl->latencyObjects->erase( _impl->latencyObjects->begin() );

        delete latencyObject;
    }
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Config::_cmdCreateNode( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "Handle create node " << command << std::endl;

    Node* node = Global::getNodeFactory()->createNode( this );
    LBCHECK( mapObject( node, command.get< UUID >( )));
    return true;
}

bool Config::_cmdDestroyNode( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "Handle destroy node " << command << std::endl;

    const UUID nodeID = command.get< UUID >();

    Node* node = _findNode( nodeID );
    LBASSERT( node );
    if( !node )
        return true;

    const bool isStopped = node->isStopped();

    LBASSERT( node->getPipes().empty( ));
    unmapObject( node );
    Global::getNodeFactory()->releaseNode( node );

    getServer()->send2( fabric::CMD_NODE_CONFIG_EXIT_REPLY,
                       nodeID ) << isStopped;
    return true;
}

bool Config::_cmdInitReply( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "handle init reply " << command << std::endl;

    const uint128_t version = command.get< uint128_t >();
    const uint32_t requestID = command.get< uint32_t >();
    const bool result = command.get< bool >();

    sync( version );
    getLocalNode()->serveRequest( requestID, result );
    return true;
}

bool Config::_cmdExitReply( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "handle exit reply " << command << std::endl;

    const uint32_t requestID = command.get< uint32_t >();
    const bool result = command.get< bool >();

    _exitMessagePump();
    getLocalNode()->serveRequest( requestID, result );
    return true;
}

bool Config::_cmdUpdateVersion( co::Command& cmd )
{
    co::ObjectCommand command( cmd );
    const uint128_t version = command.get< uint128_t >();
    const uint32_t versionID = command.get< uint32_t >();
    const uint32_t finishID = command.get< uint32_t >();
    const uint32_t requestID = command.get< uint32_t >();

    getClient()->serveRequest( versionID, version );
    getClient()->serveRequest( finishID, requestID );
    return true;
}

bool Config::_cmdUpdateReply( co::Command& cmd )
{
    co::ObjectCommand command( cmd );
    const uint128_t version = command.get< uint128_t >();
    const uint32_t requestID = command.get< uint32_t >();
    const bool result = command.get< bool >();

    sync( version );
    getClient()->serveRequest( requestID, result );
    return true;
}

bool Config::_cmdReleaseFrameLocal( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    _frameStart(); // never happened from node
    releaseFrameLocal( command.get< uint32_t >( ));
    return true;
}

bool Config::_cmdFrameFinish( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    _impl->finishedFrame = command.get< uint32_t >();

    LBLOG( LOG_TASKS ) << "frame finish " << command
                       << " frame " << _impl->finishedFrame << std::endl;

    if( _impl->unlockedFrame < _impl->finishedFrame.get( ))
    {
        LBWARN << "Finished frame " << _impl->unlockedFrame
               << " was not locally unlocked, enforcing unlock" << std::endl;
        _impl->unlockedFrame = _impl->finishedFrame.get();
    }

    co::Command empty;
    getMainThreadQueue()->push( empty ); // wakeup signal
    return true;
}

bool Config::_cmdSyncClock( co::Command& cmd )
{
    co::ObjectCommand command( cmd );
    const int64_t time = command.get< int64_t >();

    LBVERB << "sync global clock to " << time << ", drift "
           << time - _impl->clock.getTime64() << std::endl;

    _impl->clock.set( time );
    return true;
}

bool Config::_cmdSwapObject( co::Command& cmd )
{
    co::ObjectCommand command( cmd );
    LBVERB << "Cmd swap object " << command << std::endl;

    const uint32_t requestID = command.get< uint32_t >();
    co::Object* object =
            reinterpret_cast< co::Object* >( command.get< void* >( ));

    LatencyObject* latencyObject = new LatencyObject( object->getChangeType(),
                                                     object->chooseCompressor(),
                                       _impl->currentFrame + getLatency() + 1 );
    getLocalNode()->swapObject( object, latencyObject  );
    {
        lunchbox::ScopedFastWrite mutex( _impl->latencyObjects );
        _impl->latencyObjects->push_back( latencyObject );
    }
    LBASSERT( requestID != LB_UNDEFINED_UINT32 );
    getLocalNode()->serveRequest( requestID );
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
