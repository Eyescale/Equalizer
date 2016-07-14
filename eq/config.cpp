
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric Stalder@gmail.com>
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
#include "eventICommand.h"
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

#include <eq/fabric/commands.h>
#include <eq/fabric/task.h>

#include <co/object.h>
#include <co/connectionDescription.h>
#include <co/global.h>

#include <lunchbox/clock.h>
#include <lunchbox/monitor.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/spinLock.h>
#include <pression/plugins/compressor.h>

#ifdef EQUALIZER_USE_GLSTATS
#  include <GLStats/GLStats.h>
#else
    namespace GLStats { class Data {} _fakeStats; }
#endif

#include "exitVisitor.h"
#include "frameVisitor.h"
#include "initVisitor.h"

#ifdef EQUALIZER_USE_QT5WIDGETS
#  include <QApplication>
#endif

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
    virtual void getInstanceData( co::DataOStream& ){ LBDONTCALL }
    virtual void applyInstanceData( co::DataIStream& ){ LBDONTCALL }
    virtual uint32_t chooseCompressor() const { return _compressor; }

private:
    const ChangeType _changeType;
    const uint32_t _compressor;
};
#ifdef EQUALIZER_USE_GLSTATS
namespace
{
enum
{
    THREAD_MAIN,
    THREAD_ASYNC1,
    THREAD_ASYNC2,
};
}
#endif
}

namespace detail
{
class Config
{
public:
    Config()
        : eventQueue( co::Global::getCommandQueueLimit( ))
        , currentFrame( 0 )
        , unlockedFrame( 0 )
        , finishedFrame( 0 )
        , running( false )
    {
        lunchbox::Log::setClock( &clock );
    }

    ~Config()
    {
        appNode = 0;
        lunchbox::Log::setClock( 0 );
    }

    /** The node running the application thread. */
    co::NodePtr appNode;

    /** The receiver->app thread event queue. */
    CommandQueue eventQueue;

    /** The last received event to be released. */
    co::ICommand lastEvent;

    /** The connections configured by the server for this config. */
    co::Connections connections;

#ifdef EQUALIZER_USE_GLSTATS
    /** Global statistics data. */
    lunchbox::Lockable< GLStats::Data, lunchbox::SpinLock > statistics;
#endif

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

    /** Errors from last call to update() */
    Errors errors;
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

void Config::attach( const uint128_t& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

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
    registerCommand( fabric::CMD_CONFIG_EVENT_OLD, ConfigFunc( 0, 0 ),
                     &_impl->eventQueue );
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
    LBASSERT( getAppNodeID().isUUID() )
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

bool Config::init( const uint128_t& initID )
{
    LBASSERT( !_impl->running );
    _impl->currentFrame = 0;
    _impl->unlockedFrame = 0;
    _impl->finishedFrame = 0;
    _impl->frameTimes.clear();

    ClientPtr client = getClient();
    detail::InitVisitor initVisitor( client->getActiveLayouts(),
                                     client->getModelUnit( ));
    if( accept( initVisitor ) == TRAVERSE_TERMINATE )
    {
        LBWARN << "Application-local initialization failed" << std::endl;
        return false;
    }
    if( initVisitor.needsUpdate( ))
        update();

    co::LocalNodePtr localNode = getLocalNode();
    lunchbox::Request< bool > request = localNode->registerRequest< bool >();
    send( getServer(), fabric::CMD_CONFIG_INIT ) << initID << request;

    while( !request.isReady( ))
    {
#ifdef EQUALIZER_USE_QT5WIDGETS
        if( QApplication::instance( ))
            QApplication::instance()->processEvents();
#endif
        client->processCommand();
    }

    _impl->running = request.wait();
    localNode->enableSendOnRegister();

    handleEvents();
    if( !_impl->running )
        LBWARN << "Config initialization failed" << std::endl
               << "    Consult client log for further information" << std::endl;
    return _impl->running;
}

bool Config::exit()
{
    update();
    finishAllFrames();

    co::LocalNodePtr localNode = getLocalNode();
    localNode->disableSendOnRegister();

    lunchbox::Request< bool > request = localNode->registerRequest< bool >();
    send( getServer(), fabric::CMD_CONFIG_EXIT ) << request;

    ClientPtr client = getClient();
    while( !request.isReady( ))
        client->processCommand();
    bool ret = request.wait();

    detail::ExitVisitor exitVisitor;
    if( accept( exitVisitor ) == TRAVERSE_TERMINATE )
    {
        LBWARN << "Application-local de-initialization failed" << std::endl;
        ret = false;
    }
    _impl->lastEvent.clear();
    _impl->eventQueue.flush();
    _impl->running = false;
    return ret;
}

bool Config::update()
{
    commit( CO_COMMIT_NEXT );

    // send update req to server
    ClientPtr client = getClient();

    lunchbox::Request< uint128_t > reqVersion =
        client->registerRequest< uint128_t >();
    lunchbox::Request< uint32_t > reqFinish =
        client->registerRequest< uint32_t >();
    lunchbox::Request< bool > request = client->registerRequest< bool >();
    send( getServer(), fabric::CMD_CONFIG_UPDATE )
            << reqVersion << reqFinish << request;

    // wait for new version
    const uint128_t& version = reqVersion.wait();
    const uint32_t finishID = reqFinish.wait();

    if( finishID == LB_UNDEFINED_UINT32 )
    {
        sync( version );
        request.unregister();
        handleEvents();
        return true;
    }

    client->disableSendOnRegister();
    while( _impl->finishedFrame < _impl->currentFrame )
        client->processCommand();

    sync( version );
    client->ackRequest( getServer(), finishID );

    while( !request.isReady( ))
        client->processCommand();

    const bool result = request.wait();
    client->enableSendOnRegister();
#ifdef EQUALIZER_USE_GLSTATS
    _impl->statistics->clear();
#endif
    handleEvents();
    return result;
}

uint32_t Config::startFrame( const uint128_t& frameID )
{
    // Update
    ConfigStatistics stat( Statistic::CONFIG_START_FRAME, this );
    detail::FrameVisitor visitor( _impl->currentFrame + 1 );
    accept( visitor );
    update();

    // New frame
    ++_impl->currentFrame;
    send( getServer(), fabric::CMD_CONFIG_START_FRAME ) << frameID;

    LBLOG( LOG_TASKS ) << "---- Started Frame ---- " << _impl->currentFrame
                       << std::endl;
    stat.event.data.statistic.frameNumber = _impl->currentFrame;
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
    stat.event.data.statistic.frameNumber = frameToFinish;
    {
        ConfigStatistics waitStat( Statistic::CONFIG_WAIT_FINISH_FRAME, this );
        waitStat.event.data.statistic.frameNumber = frameToFinish;

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
            while( !_impl->finishedFrame.timedWaitGE( frameToFinish,
                                                      timeout ))
            {
                send( getServer(), fabric::CMD_CONFIG_CHECK_FRAME )
                    << frameToFinish;
            }
        }
        LBLOG( LOG_TASKS ) << "Global sync " << frameToFinish << " @ "
                           << _impl->currentFrame << std::endl;
    }

    handleEvents();
    _updateStatistics();
    _releaseObjects();

    LBLOG( LOG_TASKS ) << "---- Finished Frame --- " << frameToFinish
                       << " (" << _impl->currentFrame << ')' << std::endl;
    return frameToFinish;
}

uint32_t Config::finishAllFrames()
{
    if( _impl->finishedFrame == _impl->currentFrame )
        return _impl->currentFrame;

    LBLOG( LOG_TASKS ) << "-- Finish All Frames --" << std::endl;
    send( getServer(), fabric::CMD_CONFIG_FINISH_ALL_FRAMES );

    ClientPtr client = getClient();
    const uint32_t timeout = getTimeout();
    while( _impl->finishedFrame < _impl->currentFrame )
        client->processCommand( timeout );
    handleEvents();
    _updateStatistics();
    _releaseObjects();
    LBLOG( LOG_TASKS ) << "-- Finished All Frames --" << std::endl;
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
    explicit ChangeLatencyVisitor( const uint32_t latency )
        : _latency( latency ) {}

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

void Config::sendEvent( ConfigEvent& event )
{
    LBASSERT( event.data.type != Event::STATISTIC ||
              event.data.statistic.type != Statistic::NONE );
    LBASSERT( getAppNodeID() != 0 );
    LBASSERT( _impl->appNode );

    send( _impl->appNode, fabric::CMD_CONFIG_EVENT_OLD )
        << event.size << co::Array< void >( &event, event.size );
}

#ifndef EQ_2_0_API
const ConfigEvent* Config::nextEvent()
{
    EventICommand command = getNextEvent( LB_TIMEOUT_INDEFINITE );
    const ConfigEvent* newEvent = _convertEvent( command );
    return newEvent ? newEvent : nextEvent();
}

const ConfigEvent* Config::tryNextEvent()
{
    EventICommand command = getNextEvent( 0 );
    if( !command.isValid( ))
        return 0;
    const ConfigEvent* newEvent = _convertEvent( command );
    return newEvent ? newEvent : tryNextEvent();
}
#endif

const ConfigEvent* Config::_convertEvent( co::ObjectICommand command )
{
    LBASSERT( command.isValid( ));

    if( command.getCommand() != fabric::CMD_CONFIG_EVENT_OLD )
    {
        _impl->lastEvent.clear();
        return 0;
    }

    _impl->lastEvent = command;
    const uint64_t size = command.read< uint64_t >();
    return reinterpret_cast< const ConfigEvent* >
                                          ( command.getRemainingBuffer( size ));
}

bool Config::handleEvent( const ConfigEvent* event )
{
    return _handleEvent( event->data );
}

EventOCommand Config::sendEvent( const uint32_t type )
{
    LBASSERT( getAppNodeID() != 0 );
    LBASSERT( _impl->appNode );

    EventOCommand cmd( send( _impl->appNode, fabric::CMD_CONFIG_EVENT ));
    cmd << type;
    return cmd;
}

EventOCommand Config::sendError( const uint32_t type, const Error& error )
{
    return Super::sendError( getApplicationNode(), type, error );
}

Errors Config::getErrors()
{
    Errors errors;
    errors.swap(_impl->errors );
    return errors;
}

EventICommand Config::getNextEvent( const uint32_t timeout ) const
{
    if( timeout == 0 )
        return _impl->eventQueue.tryPop();
    return _impl->eventQueue.pop( timeout );
}

bool Config::handleEvent( EventICommand command )
{
    switch( command.getCommand( ))
    {
    case fabric::CMD_CONFIG_EVENT_OLD:
    {
        const ConfigEvent* configEvent = _convertEvent( command );
        LBASSERT( configEvent );
        if( configEvent )
            return handleEvent( configEvent );
        return false;
    }

    default:
        LBUNIMPLEMENTED;
        // no break;
    case fabric::CMD_CONFIG_EVENT:
        return _handleNewEvent( command );
    }
}

bool Config::checkEvent() const
{
    return !_impl->eventQueue.isEmpty();
}

void Config::handleEvents()
{
    for( ;; )
    {
        EventICommand event = getNextEvent( 0 );
        if( !event.isValid( ))
            break;

        handleEvent( event );
    }
#ifdef EQUALIZER_USE_QT5WIDGETS
    if( QApplication::instance( ))
        QApplication::instance()->processEvents();
#endif
}

bool Config::_handleNewEvent( EventICommand& command )
{
    switch( command.getEventType( ))
    {
    case Event::OBSERVER_MOTION:
    {
        const uint128_t& originator = command.read< uint128_t >();
        LBASSERT( originator != 0 );
        Observer* observer = find< Observer >( originator );
        if( observer )
            return observer->handleEvent( command );
        break;
    }

    case Event::NODE_ERROR:
    case Event::PIPE_ERROR:
    case Event::WINDOW_ERROR:
    case Event::CHANNEL_ERROR:
    {
        const Error& error = command.read< Error >();
        LBWARN << error;
        if( error.getCode() < ERROR_CUSTOM )
        {
            while( command.hasData( ))
            {
                const std::string& text = command.read< std::string >();
                LBWARN << ": " << text;
            }
        }
        LBWARN << std::endl;
        _impl->errors.push_back( error );
        return false;
    }
    }
    return false;
}

bool Config::_handleEvent( const Event& event )
{
    switch( event.type )
    {
        case Event::EXIT:
        case Event::WINDOW_CLOSE:
            _impl->running = false;
            return true;

        case Event::KEY_PRESS:
            if( event.keyPress.key == KC_ESCAPE )
            {
                _impl->running = false;
                return true;
            }
            break;

        case Event::WINDOW_POINTER_BUTTON_PRESS:
        case Event::CHANNEL_POINTER_BUTTON_PRESS:
            if( event.pointerButtonPress.buttons ==
                ( PTR_BUTTON1 | PTR_BUTTON2 | PTR_BUTTON3 ))
            {
                _impl->running = false;
                return true;
            }
            break;

        case Event::STATISTIC:
            LBLOG( LOG_STATS ) << event << std::endl;
            addStatistic( event.serial, event.statistic );
            break;

        case Event::VIEW_RESIZE:
        {
            LBASSERT( event.originator != 0 );
            View* view = find< View >( event.originator );
            if( view )
                return view->handleEvent( event );
            break;
        }
    }

    return false;
}

void Config::addStatistic( const uint32_t originator LB_UNUSED,
                           const Statistic& stat LB_UNUSED )
{
#ifdef EQUALIZER_USE_GLSTATS
    const uint32_t frame = stat.frameNumber;
    LBASSERT( stat.type != Statistic::NONE );

     // Not a frame-related stat event OR no event-type set
    if( frame == 0 || stat.type == Statistic::NONE )
        return;

    lunchbox::ScopedFastWrite mutex( _impl->statistics );
    GLStats::Item item;
    item.entity = originator;
    item.type = stat.type;
    item.frame = stat.frameNumber;
    item.start = stat.startTime;
    item.end = stat.endTime;

    GLStats::Entity entity;
    entity.name = stat.resourceName;

    GLStats::Type type;
    const Vector3f& color = Statistic::getColor( stat.type );

    type.color[0] = color[0];
    type.color[1] = color[1];
    type.color[2] = color[2];
    type.name = Statistic::getName( stat.type );

    switch( stat.type )
    {
      case Statistic::CHANNEL_FRAME_COMPRESS:
      case Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN:
          type.subgroup = "transmit";
          item.thread = THREAD_ASYNC2;
          // no break;
      case Statistic::CHANNEL_FRAME_WAIT_READY:
          type.group = "channel";
          item.layer = 1;
          break;
      case Statistic::CHANNEL_CLEAR:
      case Statistic::CHANNEL_DRAW:
      case Statistic::CHANNEL_DRAW_FINISH:
      case Statistic::CHANNEL_ASSEMBLE:
      case Statistic::CHANNEL_READBACK:
      case Statistic::CHANNEL_VIEW_FINISH:
          type.group = "channel";
          break;
      case Statistic::CHANNEL_ASYNC_READBACK:
          type.group = "channel";
          type.subgroup = "transfer";
          item.thread = THREAD_ASYNC1;
          break;
      case Statistic::CHANNEL_FRAME_TRANSMIT:
          type.group = "channel";
          type.subgroup = "transmit";
          item.thread = THREAD_ASYNC2;
          break;

      case Statistic::WINDOW_FINISH:
      case Statistic::WINDOW_THROTTLE_FRAMERATE:
      case Statistic::WINDOW_SWAP_BARRIER:
      case Statistic::WINDOW_SWAP:
          type.group = "window";
          break;
      case Statistic::NODE_FRAME_DECOMPRESS:
          type.group = "node";
          break;

      case Statistic::CONFIG_WAIT_FINISH_FRAME:
          item.layer = 1;
          // no break;
      case Statistic::CONFIG_START_FRAME:
      case Statistic::CONFIG_FINISH_FRAME:
          type.group = "config";
          break;

      case Statistic::PIPE_IDLE:
      {
          const std::string& string = _impl->statistics->getText();
          const float idle = stat.idleTime * 100ll / stat.totalTime;
          std::stringstream text;
          if( string.empty( ))
              text <<  "Idle: " << stat.resourceName << ' ' << idle << "%";
          else
          {
              const size_t pos = string.find( stat.resourceName );

              if( pos == std::string::npos ) // append new pipe
                  text << string << ", " << stat.resourceName << ' '
                       << idle << "%";
              else // replace existing text
              {
                  const std::string& left = string.substr( pos + 1 );

                  text << string.substr( 0, pos ) << stat.resourceName << ' '
                       << idle << left.substr( left.find( '%' ));
              }
          }
          _impl->statistics->setText( text.str( ));
      }
      // no break;

      case Statistic::WINDOW_FPS:
      case Statistic::NONE:
      case Statistic::ALL:
          return;
    }
    switch( stat.type )
    {
      case Statistic::CHANNEL_FRAME_COMPRESS:
      case Statistic::CHANNEL_ASYNC_READBACK:
      case Statistic::CHANNEL_READBACK:
      {
          std::stringstream text;
          text << unsigned( 100.f * stat.ratio ) << '%';

          if( stat.plugins[ 0 ] > EQ_COMPRESSOR_NONE )
              text << " 0x" << std::hex << stat.plugins[0] << std::dec;
          if( stat.plugins[ 1 ] > EQ_COMPRESSOR_NONE &&
              stat.plugins[ 0 ] != stat.plugins[ 1 ] )
          {
              text << " 0x" << std::hex << stat.plugins[1] << std::dec;
          }
          item.text = text.str();
          break;
      }
      default:
          break;
    }

    _impl->statistics->setType( stat.type, type );
    _impl->statistics->setEntity( originator, entity );
    _impl->statistics->addItem( item );
#endif
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

void Config::_updateStatistics()
{
#ifdef EQUALIZER_USE_GLSTATS
    // keep statistics for three frames
    lunchbox::ScopedFastWrite mutex( _impl->statistics );
    _impl->statistics->obsolete( 2 /* frames to keep */ );
#endif
}

GLStats::Data Config::getStatistics() const
{
#ifdef EQUALIZER_USE_GLSTATS
    lunchbox::ScopedFastRead mutex( _impl->statistics );
    return _impl->statistics.data;
#else
    return GLStats::_fakeStats;
#endif
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

    if( isThreaded && !windowSystem.hasMainThreadEvents( ))
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

void Config::setupServerConnections( const std::string& connectionData )
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
    const lunchbox::Request< void > request =
        getLocalNode()->registerRequest< void >();
    send( client, fabric::CMD_CONFIG_SWAP_OBJECT ) << request << object;
}

bool Config::mapObject( co::Object* object, const uint128_t& id,
                        const uint128_t& version )
{
    return mapObjectSync( mapObjectNB( object, id, version ));
}

uint32_t Config::mapObjectNB( co::Object* object, const uint128_t& id,
                              const uint128_t& version )
{
    return getClient()->mapObjectNB( object, id, version );
}

uint32_t Config::mapObjectNB( co::Object* object, const uint128_t& id,
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

f_bool_t Config::syncObject( co::Object* object, const uint128_t& id,
                             co::NodePtr master, const uint32_t instanceID )
{
    return getClient()->syncObject( object, id, master, instanceID );
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
bool Config::_cmdCreateNode( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBVERB << "Handle create node " << command << std::endl;

    Node* node = Global::getNodeFactory()->createNode( this );
    LBCHECK( mapObject( node, command.read< uint128_t >( )));
    return true;
}

bool Config::_cmdDestroyNode( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBVERB << "Handle destroy node " << command << std::endl;

    Node* node = _findNode( command.read< uint128_t >( ));
    LBASSERT( node );
    if( !node )
        return true;

    const bool isStopped = node->isStopped();

    LBASSERT( node->getPipes().empty( ));
    unmapObject( node );
    node->send( getServer(), fabric::CMD_NODE_CONFIG_EXIT_REPLY ) << isStopped;
    Global::getNodeFactory()->releaseNode( node );

    return true;
}

bool Config::_cmdInitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBVERB << "handle init reply " << command << std::endl;

    const uint128_t& version = command.read< uint128_t >();
    const uint32_t requestID = command.read< uint32_t >();
    const bool result = command.read< bool >();

    sync( version );
    getLocalNode()->serveRequest( requestID, result );
    return true;
}

bool Config::_cmdExitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBVERB << "handle exit reply " << command << std::endl;

    const uint32_t requestID = command.read< uint32_t >();
    const bool result = command.read< bool >();

    _exitMessagePump();
    getLocalNode()->serveRequest( requestID, result );
    return true;
}

bool Config::_cmdUpdateVersion( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& version = command.read< uint128_t >();
    const uint32_t versionID = command.read< uint32_t >();
    const uint32_t finishID = command.read< uint32_t >();
    const uint32_t requestID = command.read< uint32_t >();

    getClient()->serveRequest( versionID, version );
    getClient()->serveRequest( finishID, requestID );
    return true;
}

bool Config::_cmdUpdateReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& version = command.read< uint128_t >();
    const uint32_t requestID = command.read< uint32_t >();
    const bool result = command.read< bool >();

    sync( version );
    getClient()->serveRequest( requestID, result );
    return true;
}

bool Config::_cmdReleaseFrameLocal( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    _frameStart(); // never happened from node
    releaseFrameLocal( command.read< uint32_t >( ));
    return true;
}

bool Config::_cmdFrameFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    _impl->finishedFrame = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "frame finish " << command
                       << " frame " << _impl->finishedFrame << std::endl;

    if( _impl->unlockedFrame < _impl->finishedFrame.get( ))
    {
        LBWARN << "Finished frame " << _impl->unlockedFrame
               << " was not locally unlocked, enforcing unlock" << std::endl;
        _impl->unlockedFrame = _impl->finishedFrame.get();
    }

    co::ICommand empty;
    getMainThreadQueue()->push( empty ); // wakeup signal
    return true;
}

bool Config::_cmdSyncClock( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const int64_t time = command.read< int64_t >();

    LBVERB << "sync global clock to " << time << ", drift "
           << time - _impl->clock.getTime64() << std::endl;

    _impl->clock.set( time );
    return true;
}

bool Config::_cmdSwapObject( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    LBVERB << "Cmd swap object " << command << std::endl;

    const uint32_t requestID = command.read< uint32_t >();
    co::Object* object =
            reinterpret_cast< co::Object* >( command.read< void* >( ));

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

#include <eq/fabric/config.ipp>
#include <eq/fabric/view.ipp>
#include <eq/fabric/observer.ipp>

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
