
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
#include "client.h"
#include "configDeserializer.h"
#include "configEvent.h"
#include "configStatistics.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "layout.h"
#include "log.h"
#include "messagePump.h"
#include "nameFinder.h"
#include "node.h"
#include "nodeFactory.h"
#include "observer.h"
#include "packets.h"
#include "pipe.h"
#include "server.h"
#include "view.h"

#include <eq/fabric/task.h>
#include <eq/net/command.h>
#include <eq/net/global.h>

#include "configCommitVisitor.h"

namespace eq
{
/** @cond IGNORE */
typedef net::CommandFunc<Config> ConfigFunc;
typedef fabric::Config< Server, Config, Observer, Layout > Super;
/** @endcond */

Config::Config( ServerPtr server )
        : Super( server )
        , _eyeBase( 0.f )
        , _lastEvent( 0 )
        , _latency( 0 )
        , _currentFrame( 0 )
        , _unlockedFrame( 0 )
        , _finishedFrame( 0 )
        , _running( false )
{
    base::Log::setClock( &_clock );
    EQINFO << "New config @" << (void*)this << std::endl;
}

Config::~Config()
{
    EQINFO << "Delete config @" << (void*)this << std::endl;
    EQASSERT( getObservers().empty( ));
    EQASSERT( getLayouts().empty( ));
    EQASSERT( _canvases.empty( ));
    
    while( tryNextEvent( )) /* flush all pending events */ ;
    if( _lastEvent )
        _lastEvent->release();
    _eventQueue.flush();
    _lastEvent = 0;

    _appNodeID = net::NodeID::ZERO;
    _appNode   = 0;
    base::Log::setClock( 0 );
}

void Config::notifyMapped( net::NodePtr node )
{
    net::Session::notifyMapped( node );

    ServerPtr          server = getServer();
    net::CommandQueue* queue  = server->getNodeThreadQueue();

    registerCommand( CMD_CONFIG_CREATE_NODE,
                     ConfigFunc( this, &Config::_cmdCreateNode ), queue );
    registerCommand( CMD_CONFIG_DESTROY_NODE,
                     ConfigFunc( this, &Config::_cmdDestroyNode ), queue );
    registerCommand( CMD_CONFIG_START_FRAME_REPLY,
                     ConfigFunc( this, &Config::_cmdStartFrameReply ), queue );
    registerCommand( CMD_CONFIG_INIT_REPLY, 
                     ConfigFunc( this, &Config::_cmdInitReply ), queue );
    registerCommand( CMD_CONFIG_EXIT_REPLY, 
                     ConfigFunc( this, &Config::_cmdExitReply ), queue );
    registerCommand( CMD_CONFIG_RELEASE_FRAME_LOCAL, 
                     ConfigFunc( this, &Config::_cmdReleaseFrameLocal ), queue);
    registerCommand( CMD_CONFIG_FRAME_FINISH, 
                     ConfigFunc( this, &Config::_cmdFrameFinish ), 0 );
    registerCommand( CMD_CONFIG_EVENT, 
                     ConfigFunc( this, &Config::_cmdUnknown ), &_eventQueue );
    registerCommand( CMD_CONFIG_SYNC_CLOCK, 
                     ConfigFunc( this, &Config::_cmdSyncClock ), 0 );
    registerCommand( CMD_CONFIG_UNMAP, ConfigFunc( this, &Config::_cmdUnmap ),
                     queue );
}

CommandQueue* Config::getNodeThreadQueue()
{
    return getClient()->getNodeThreadQueue();
}

namespace
{
template< typename T > class IDFinder : public ConfigVisitor
{
public:
    IDFinder( const uint32_t id ) : _id( id ), _result( 0 ) {}
    virtual ~IDFinder(){}

    virtual VisitorResult visitPre( T* node ) { return visit( node ); }
    virtual VisitorResult visit( T* node )
        {
            if( node->getID() == _id )
            {
                _result = node;
                return TRAVERSE_TERMINATE;
            }
            return TRAVERSE_CONTINUE;
        }

    T* getResult() { return _result; }

private:
    const uint32_t _id;
    T*             _result;
};

typedef IDFinder< View > ViewIDFinder;
}

namespace
{
template< class C >
VisitorResult _accept( C* config, ConfigVisitor& visitor )
{ 
    VisitorResult result = visitor.visitPre( config );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const NodeVector& nodes = config->getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
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

    const ObserverVector& observers = config->getObservers();
    for( ObserverVector::const_iterator i = observers.begin(); 
         i != observers.end(); ++i )
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

    const LayoutVector& layouts = config->getLayouts();
    for( LayoutVector::const_iterator i = layouts.begin(); 
         i != layouts.end(); ++i )
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

    const CanvasVector& canvases = config->getCanvases();
    for( CanvasVector::const_iterator i = canvases.begin();
         i != canvases.end(); ++i )
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

    switch( visitor.visitPost( config ))
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

    return result;
}
}

VisitorResult Config::accept( ConfigVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Config::accept( ConfigVisitor& visitor ) const
{
    return _accept( this, visitor );
}

ClientPtr Config::getClient()
{ 
    return getServer()->getClient(); 
}

void Config::_addNode( Node* node )
{
    EQASSERT( node->getConfig() == this );
    _nodes.push_back( node );
}

void Config::_removeNode( Node* node )
{
    NodeVector::iterator i = std::find( _nodes.begin(), _nodes.end(), node );
    EQASSERT( i != _nodes.end( ));
    _nodes.erase( i );
}

Node* Config::_findNode( const uint32_t id )
{
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); 
         ++i )
    {
        Node* node = *i;
        if( node->getID() == id )
            return node;
    }
    return 0;
}

void Config::_addCanvas( Canvas* canvas )
{
    canvas->_config = this;
    _canvases.push_back( canvas );
}

void Config::_removeCanvas( Canvas* canvas )
{
    CanvasVector::iterator i = std::find( _canvases.begin(), _canvases.end(),
                                          canvas );
    EQASSERT( i != _canvases.end( ));
    _canvases.erase( i );
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

    handleEvents();
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

    _appNode   = 0;
    _appNodeID = net::NodeID::ZERO;
    return ret;
}

uint32_t Config::startFrame( const uint32_t frameID )
{
    ConfigStatistics stat( Statistic::CONFIG_START_FRAME, this );

    ConfigCommitVisitor committer;
    accept( committer );
    const std::vector< net::ObjectVersion >& changes = committer.getChanges();
    
    ConfigStartFramePacket packet;
    ClientPtr client = getClient();

    if( committer.needsFinish( ))
    {
        packet.requestID = client->registerRequest();
        finishAllFrames();
    }

    // Request new frame
    packet.frameID  = frameID;
    packet.nChanges = changes.size();
    send( packet, changes );

    if( packet.requestID != EQ_ID_INVALID )
    {
        while( !client->isRequestServed( packet.requestID ))
            client->processCommand();

        client->waitRequest( packet.requestID );
    }
    ++_currentFrame;

    EQLOG( base::LOG_ANY ) << "---- Started Frame ---- " << _currentFrame
                           << std::endl;
    stat.event.data.statistic.frameNumber = _currentFrame;
    return _currentFrame;
}

void Config::_frameStart()
{
    _frameTimes.push_back( _clock.getTime64( ));
    while( _frameTimes.size() > _latency )
    {
        const int64_t age = _frameTimes.back() - _frameTimes.front();
        expireInstanceData( age );
        _frameTimes.pop_front();
    }
}

uint32_t Config::finishFrame()
{
    ClientPtr client = getClient();
    const uint32_t frameToFinish = (_currentFrame >= _latency) ? 
                                      _currentFrame - _latency : 0;

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
        if( !_nodes.empty( ))
        {
            EQASSERT( _nodes.size() == 1 );
            const Node* node = _nodes.front();

            while( node->getFinishedFrame() < frameToFinish )
                client->processCommand();
        }

        // global sync
        _finishedFrame.waitGE( frameToFinish );
    }

    handleEvents();
    _updateStatistics( frameToFinish );

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
    EQLOG( base::LOG_ANY ) << "-- Finished All Frames --" << std::endl;
    return _currentFrame;
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
        view->setAutoObsolete( _latency );
        return TRAVERSE_CONTINUE;    
    }

private:
    const uint32_t _latency; 
};
}

void Config::changeLatency( const uint32_t latency )
{
    finishAllFrames();
    _latency = latency;
    

    // send latency to the server
    ConfigChangeLatency packet;
    packet.latency = latency;
    send( packet );

    // update views
    ChangeLatencyVisitor changeLatencyVisitor( latency );
    accept( changeLatencyVisitor );
}

void Config::sendEvent( ConfigEvent& event )
{
    EQASSERT( event.data.type != Event::STATISTIC ||
              event.data.statistic.type != Statistic::NONE );
    EQASSERT( _appNodeID != base::UUID::ZERO );

    if( !_appNode )
    {
        net::NodePtr localNode = getLocalNode();
        _appNode = localNode->connect( _appNodeID );
    }
    EQASSERT( _appNode );

    event.sessionID = getID();
    EQLOG( LOG_EVENTS ) << "send event " << &event << std::endl;
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
            EQASSERT( view );
            if( view )
                return view->handleEvent( event->data );
            break;
        }
    }

    return false;
}

bool Config::_needsLocalSync() const
{
    if( _nodes.empty( ))
        return false;

    EQASSERT( _nodes.size() == 1 );
    const Node* node = _nodes.front();
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

void Config::setupMessagePump( Pipe* pipe )
{
    const bool isThreaded = pipe->isThreaded();
    const WindowSystem windowSystem = pipe->getWindowSystem();

    if( isThreaded && windowSystem != WINDOW_SYSTEM_AGL )
        return;

    // called from pipe threads - but only during init
    static base::Lock _lock;
    base::ScopedMutex<> mutex( _lock );

    if( _eventQueue.getMessagePump( ))
        // Already done
        return;

    MessagePump* pump = pipe->createMessagePump();

    _eventQueue.setMessagePump( pump );

    ClientPtr client = getClient();
    CommandQueue* queue = client->getNodeThreadQueue();
    EQASSERT( queue );
    EQASSERT( !queue->getMessagePump( ));

    queue->setMessagePump( pump );
}

void Config::_exitMessagePump()
{
    MessagePump* pump = _eventQueue.getMessagePump();

    _eventQueue.setMessagePump( 0 );

    ClientPtr client = getClient();
    CommandQueue* queue = client->getNodeThreadQueue();
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

void Config::_initAppNode( const uint32_t distributorID )
{
    ConfigDeserializer distributor( this );
    EQCHECK( mapObject( &distributor, distributorID ));
    unmapObject( &distributor ); // data was retrieved, unmap
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Config::_cmdCreateNode( net::Command& command )
{
    const ConfigCreateNodePacket* packet = 
        command.getPacket<ConfigCreateNodePacket>();
    EQVERB << "Handle create node " << packet << std::endl;
    EQASSERT( packet->nodeID != EQ_ID_INVALID );

    Node* node = Global::getNodeFactory()->createNode( this );
    attachObject( node, packet->nodeID, EQ_ID_INVALID );

    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdDestroyNode( net::Command& command ) 
{
    const ConfigDestroyNodePacket* packet =
        command.getPacket<ConfigDestroyNodePacket>();
    EQVERB << "Handle destroy node " << packet << std::endl;

    Node* node = _findNode( packet->nodeID );
    if( !node )
        return net::COMMAND_HANDLED;

    detachObject( node );
    Global::getNodeFactory()->releaseNode( node );

    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdStartFrameReply( net::Command& command ) 
{
    const ConfigStartFrameReplyPacket* packet =
        command.getPacket< ConfigStartFrameReplyPacket >();

    getClient()->serveRequest( packet->requestID );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdInitReply( net::Command& command )
{
    const ConfigInitReplyPacket* packet = 
        command.getPacket<ConfigInitReplyPacket>();
    EQVERB << "handle init reply " << packet << std::endl;

    if( !packet->result )
        _error = packet->error;

    getLocalNode()->serveRequest( packet->requestID, (void*)(packet->result) );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdExitReply( net::Command& command )
{
    const ConfigExitReplyPacket* packet = 
        command.getPacket<ConfigExitReplyPacket>();
    EQVERB << "handle exit reply " << packet << std::endl;

    _exitMessagePump();
    getLocalNode()->serveRequest( packet->requestID, (void*)(packet->result) );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdReleaseFrameLocal( net::Command& command )
{
    const ConfigReleaseFrameLocalPacket* packet =
        command.getPacket< ConfigReleaseFrameLocalPacket >();

    _frameStart(); // never happened from node
    releaseFrameLocal( packet->frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdFrameFinish( net::Command& command )
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

    getNodeThreadQueue()->wakeup();
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdSyncClock( net::Command& command )
{
    const ConfigSyncClockPacket* packet = 
        command.getPacket< ConfigSyncClockPacket >();

    EQVERB << "sync global clock to " << packet->time << ", drift " 
           << packet->time - _clock.getTime64() << std::endl;

    _clock.set( packet->time );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdUnmap( net::Command& command )
{
    const ConfigUnmapPacket* packet = command.getPacket< ConfigUnmapPacket >();
    EQVERB << "Handle unmap " << packet << std::endl;

    NodeFactory* nodeFactory = Global::getNodeFactory();

    for( CanvasVector::const_iterator i = _canvases.begin();
         i != _canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        canvas->_deregister();
        canvas->_config = 0;
        nodeFactory->releaseCanvas( canvas );
    }
    _canvases.clear();

    const LayoutVector& layouts = getLayouts();
    while( !layouts.empty( ))
    {
        Layout* layout = layouts.back();;
        layout->_unmap();
        _removeLayout( layout );
        nodeFactory->releaseLayout( layout );
    }

    const ObserverVector& observers = getObservers();
    while( !observers.empty( ))
    {
        Observer* observer = observers.back();
        unmapObject( observer );
        _removeObserver( observer );
        nodeFactory->releaseObserver( observer );
    }

    ConfigUnmapReplyPacket reply( packet );
    send( command.getNode(), reply );

    return net::COMMAND_HANDLED;
}

}

#include "../fabric/config.cpp"
template class eq::fabric::Config< eq::Server, eq::Config, eq::Observer,
                                   eq::Layout >;
#define FIND_ID_TEMPLATE1( type )                                       \
    template void eq::fabric::Config< eq::Server, eq::Config, eq::Observer, \
                                      eq::Layout >::find< type >(       \
                                          const uint32_t, type** );
FIND_ID_TEMPLATE1( eq::Observer );

#define FIND_ID_TEMPLATE2( type )                                       \
    template type* eq::fabric::Config< eq::Server, eq::Config, eq::Observer, \
                                       eq::Layout >::find< type >(      \
                                           const uint32_t );
FIND_ID_TEMPLATE2( eq::Observer );
FIND_ID_TEMPLATE2( eq::Layout );
FIND_ID_TEMPLATE2( eq::View );

#define FIND_NAME_TEMPLATE1( type )\
    template void eq::fabric::Config< eq::Server, eq::Config, eq::Observer, \
                                      eq::Layout >::find< type >(       \
                                          const std::string&,           \
                                          const type** ) const;

FIND_NAME_TEMPLATE1( eq::Observer );
