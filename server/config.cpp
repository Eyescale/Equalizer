
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

#include <pthread.h>
#include "config.h"

#include "canvas.h"
#include "changeLatencyVisitor.h"
#include "compound.h"
#include "compoundVisitor.h"
#include "configUpdateDataVisitor.h"
#include "equalizers/equalizer.h"
#include "global.h"
#include "layout.h"
#include "log.h"
#include "node.h"
#include "observer.h"
#include "segment.h"
#include "server.h"
#include "view.h"
#include "window.h"

#include <eq/client/configEvent.h>
#include <eq/fabric/paths.h>
#include <eq/net/command.h>
#include <eq/net/global.h>
#include <eq/base/sleep.h>

#include "configDeregistrator.h"
#include "configRegistrator.h"
#include "configSyncVisitor.h"

using eq::net::ConnectionDescriptionVector;

namespace eq
{
namespace server
{
typedef net::CommandFunc<Config> ConfigFunc;
typedef fabric::Config< Server, Config, Observer, Layout, Canvas, Node,
                        ConfigVisitor > Super;

void Config::_construct()
{
    _currentFrame  = 0;
    _finishedFrame = 0;
    _state         = STATE_STOPPED;

    EQINFO << "New config @" << (void*)this << std::endl;
}

Config::Config( ServerPtr parent )
        : Super( parent )
{
    _construct();
    const Global* global = Global::instance();    
    for( int i=0; i<FATTR_ALL; ++i )
    {
        const FAttribute attr = static_cast< FAttribute >( i );
        setFAttribute( attr, global->getConfigFAttribute( attr ));
    }
}

Config::~Config()
{
    EQINFO << "Delete config @" << (void*)this << std::endl;
    _appNetNode = 0;

    for( CompoundVector::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->_config = 0;
        delete compound;
    }
    _compounds.clear();

    NodeVector nodes = getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        delete node;
    }
    nodes.clear();
}

void Config::notifyMapped( net::NodePtr node )
{
    net::Session::notifyMapped( node );

    net::CommandQueue* serverQueue  = getMainThreadQueue();
    net::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( fabric::CMD_CONFIG_INIT,
                     ConfigFunc( this, &Config::_cmdInit), serverQueue );
    registerCommand( fabric::CMD_CONFIG_EXIT,
                     ConfigFunc( this, &Config::_cmdExit ), serverQueue );
    registerCommand( fabric::CMD_CONFIG_CREATE_REPLY,
                     ConfigFunc( this, &Config::_cmdCreateReply ),
                     commandQueue );
    registerCommand( fabric::CMD_CONFIG_START_FRAME, 
                     ConfigFunc( this, &Config::_cmdStartFrame ), serverQueue );
    registerCommand( fabric::CMD_CONFIG_FINISH_ALL_FRAMES, 
                     ConfigFunc( this, &Config::_cmdFinishAllFrames ),
                     serverQueue );
    registerCommand( fabric::CMD_CONFIG_FREEZE_LOAD_BALANCING, 
                     ConfigFunc( this, &Config::_cmdFreezeLoadBalancing ), 
                     serverQueue );
}

namespace
{
class ChannelViewFinder : public ConfigVisitor
{
public:
    ChannelViewFinder( const Segment* const segment, const View* const view ) 
            : _segment( segment ), _view( view ), _result( 0 ) {}

    virtual ~ChannelViewFinder(){}

    virtual VisitorResult visit( Channel* channel )
        {
            if( channel->getView() != _view )
                return TRAVERSE_CONTINUE;

            if( channel->getSegment() != _segment )
                return TRAVERSE_CONTINUE;

            _result = channel;
            return TRAVERSE_TERMINATE;
        }

    Channel* getResult() { return _result; }

private:
    const Segment* const _segment;
    const View* const    _view;
    Channel*             _result;
};
}

const Channel* Config::findChannel( const std::string& name ) const
{
    return Super::find< Channel >( name );
}

Channel* Config::findChannel( const Segment* segment, const View* view )
{
    ChannelViewFinder finder( segment, view );
    accept( finder );
    return finder.getResult();
}

void Config::activateCanvas( Canvas* canvas )
{
    EQASSERT( stde::find( getCanvases(), canvas ) != getCanvases().end( ));

    const LayoutVector& layouts = canvas->getLayouts();
    const SegmentVector& segments = canvas->getSegments();

    for( LayoutVector::const_iterator i = layouts.begin();
         i != layouts.end(); ++i )
    {
        const Layout* layout = *i;
        if( !layout )
            continue;

        const ViewVector& views = layout->getViews();
        for( ViewVector::const_iterator j = views.begin(); 
             j != views.end(); ++j )
        {
            View* view = *j;

            for( SegmentVector::const_iterator k = segments.begin();
                 k != segments.end(); ++k )
            {
                Segment* segment = *k;
                Viewport viewport = segment->getViewport();
                viewport.intersect( view->getViewport( ));

                if( !viewport.hasArea())
                {
                    EQLOG( LOG_VIEW )
                        << "View " << view->getName() << view->getViewport()
                        << " doesn't intersect " << segment->getName()
                        << segment->getViewport() << std::endl;
                
                    continue;
                }
                      
                Channel* segmentChannel = segment->getChannel( );
                if (!segmentChannel)
                {
                    EQWARN << "Segment " << segment->getName()
                           << " has no output channel" << std::endl;
                    continue;
                }

                // try to reuse channel
                Channel* channel = findChannel( segment, view );

                if( !channel ) // create and add new channel
                {
                    Window* window = segmentChannel->getWindow();
                    channel = new Channel( *segmentChannel, window );

                    channel->setOutput( view, segment );
                }

                //----- compute channel viewport:
                // segment/view intersection in canvas space...
                Viewport contribution = viewport;
                // ... in segment space...
                contribution.transform( segment->getViewport( ));
            
                // segment output area
                Viewport subViewport = segmentChannel->getViewport();
                // ...our part of it    
                subViewport.apply( contribution );
            
                channel->setViewport( subViewport );
            
                EQLOG( LOG_VIEW ) 
                    << "View @" << (void*)view << ' ' << view->getViewport()
                    << " intersects " << segment->getName()
                    << segment->getViewport() << " at " << subViewport
                    << " using channel @" << (void*)channel << std::endl;
            }
        }
    }
}

Observer* Config::createObserver()
{
    return new Observer( this );
}

void Config::releaseObserver( Observer* observer )
{
    delete observer;
}

Layout* Config::createLayout()
{
    return new Layout( this );
}

void Config::releaseLayout( Layout* layout )
{
    delete layout;
}

Canvas* Config::createCanvas()
{
    return new Canvas( this );
}

void Config::releaseCanvas( Canvas* canvas )
{
    delete canvas;
}


void Config::addCompound( Compound* compound )
{
    compound->_config = this;
    _compounds.push_back( compound );
}

bool Config::removeCompound( Compound* compound )
{
    CompoundVector::iterator i = stde::find( _compounds, compound );
    if( i == _compounds.end( ))
        return false;

    _compounds.erase( i );
    compound->_config = 0;
    return true;
}

void Config::setApplicationNetNode( net::NodePtr node )
{
    EQASSERT( _state == STATE_STOPPED );

    _appNetNode = node;
    if( node.isValid( ))
        setAppNodeID( node->getNodeID( ));
    else
        setAppNodeID( net::NodeID::ZERO );
}

Channel* Config::getChannel( const ChannelPath& path )
{
    NodeVector nodes = getNodes();
    EQASSERTINFO( nodes.size() > path.nodeIndex,
                  nodes.size() << " <= " << path.nodeIndex );

    if( nodes.size() <= path.nodeIndex )
        return 0;

    return nodes[ path.nodeIndex ]->getChannel( path );
}

Segment* Config::getSegment( const SegmentPath& path )
{
    Canvas* canvas = getCanvas( path );
    EQASSERT( canvas );

    if( canvas )
        return canvas->getSegment( path );

    return 0;
}

View* Config::getView( const ViewPath& path )
{
    Layout* layout = getLayout( path );
    EQASSERT( layout );

    if( layout )
        return layout->getView( path );

    return 0;
}

namespace
{
template< class C >
static VisitorResult _accept( C* config, ConfigVisitor& visitor )
{ 
    VisitorResult result = TRAVERSE_CONTINUE;
    const CompoundVector& compounds = config->getCompounds();
    for( CompoundVector::const_iterator i = compounds.begin();
         i != compounds.end(); ++i )
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
    return result;
}
}

VisitorResult Config::_acceptCompounds( ConfigVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Config::_acceptCompounds( ConfigVisitor& visitor ) const
{
    return _accept( this, visitor );
}

//===========================================================================
// operations
//===========================================================================

uint32_t Config::register_()
{
    ConfigRegistrator registrator( this );
    accept( registrator );
    return Super::register_();
}

void Config::deregister()
{
    ConfigSyncVisitor syncer;
    accept( syncer );

    Super::deregister();
    ConfigDeregistrator deregistrator;
    accept( deregistrator );
}

void Config::restore()
{
    _currentFrame = 0;
    _finishedFrame = 0;
    setApplicationNetNode( 0 );
    _workDir.clear();
    _renderClient.clear();
    Super::restore();
}

//---------------------------------------------------------------------------
// update running entities (init/exit)
//---------------------------------------------------------------------------

bool Config::_updateRunning()
{
    if( _state == STATE_STOPPED )
        return true;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_INITIALIZING ||
              _state == STATE_EXITING );

    setErrorMessage( "" );

    if( !_connectNodes( ))
        return false;

    _startNodes();
    const NodeVector& nodes = getNodes();
    // Let all running nodes update their running state (incl. children)
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
        (*i)->updateRunning( _initID, _currentFrame );

    // Sync state updates
    bool success = true;
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( !node->syncRunning( ))
        {
            setErrorMessage( getErrorMessage() +
                             "node " + node->getName() + ": '" + 
                             node->getErrorMessage() + '\'' );
            success = false;
        }
    }

    _stopNodes();
    _syncClock();
    return success;
}

//----- connect new nodes
bool Config::_connectNodes()
{
    bool success = true;
    base::Clock clock;
    const NodeVector& nodes = getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive() && !_connectNode( node ))
        {
            success = false;
            break;
        }
    }

    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive() && !_syncConnectNode( node, clock ))
            success = false;
    }

    return success;
}


namespace
{
static net::NodePtr _createNetNode( Node* node )
{
    net::NodePtr netNode = new net::Node;

    const ConnectionDescriptionVector& descriptions = 
        node->getConnectionDescriptions();
    for( ConnectionDescriptionVector::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        const net::ConnectionDescription* desc = (*i).get();
        
        netNode->addConnectionDescription( 
            new net::ConnectionDescription( *desc ));
    }

    netNode->setLaunchTimeout( 
        node->getIAttribute( Node::IATTR_LAUNCH_TIMEOUT ));
    netNode->setLaunchCommand( 
        node->getSAttribute( Node::SATTR_LAUNCH_COMMAND ));
    netNode->setLaunchCommandQuote( 
        node->getCAttribute( Node::CATTR_LAUNCH_COMMAND_QUOTE ));
    netNode->setAutoLaunch( true );
    return netNode;
}
}

bool Config::_connectNode( Node* node )
{
    EQASSERT( node->isActive( ));

    net::NodePtr netNode = node->getNode();
    if( netNode.isValid( ))
        return netNode->isConnected();

    net::NodePtr localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));
    
    if( node->isApplicationNode( ))
        netNode = _appNetNode;
    else
    {
        netNode = _createNetNode( node );
        netNode->setProgramName( _renderClient );
        netNode->setWorkDir( _workDir );
    }

    EQLOG( LOG_INIT ) << "Connecting node" << std::endl;
    if( !localNode->initConnect( netNode ))
    {
        std::stringstream nodeString;
        nodeString << "Connection to node failed, node does not run and launch "
                   << "command failed: " << node;
        
        setErrorMessage( getErrorMessage() + nodeString.str( ));
        EQERROR << "Connection to " << netNode->getNodeID() << " failed." 
                << std::endl;
        return false;
    }

    node->setNode( netNode );
    return true;
}

bool Config::_syncConnectNode( Node* node, const base::Clock& clock )
{
    EQASSERT( node->isActive( ));

    net::NodePtr netNode = node->getNode();
    if( !netNode )
        return false;

    net::NodePtr localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));

    const int64_t timeLeft = netNode->getLaunchTimeout() - clock.getTime64();
    const uint32_t timeOut = EQ_MAX( timeLeft, 0 );

    if( !localNode->syncConnect( netNode, timeOut ))
    {
        std::ostringstream data;
        const net::ConnectionDescriptionVector& descs = 
            netNode->getConnectionDescriptions();

        for( net::ConnectionDescriptionVector::const_iterator i = descs.begin();
             i != descs.end(); ++i )
        {
            net::ConnectionDescriptionPtr desc = *i;
            data << desc->getHostname() << ' ';
        }
        setErrorMessage( getErrorMessage() + 
                         "Connection of node failed, node did not start ( " +
                         data.str() + ") " );
        EQERROR << getErrorMessage() << std::endl;

        node->setNode( 0 );
        EQASSERT( netNode->getRefCount() == 1 );
        return false;
    }
    return true;
}

void Config::_startNodes()
{
    // start up newly running nodes
    std::vector< uint32_t > requests;
    NodeVector startingNodes;
    const NodeVector& nodes = getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        const Node::State state = node->getState();

        if( node->isActive() && state != Node::STATE_RUNNING )
        {
            EQASSERT( state == Node::STATE_STOPPED );
            startingNodes.push_back( node );
            if( !node->isApplicationNode( ))
                requests.push_back( _createConfig( node ));
        }
    }

    // sync create config requests on starting nodes
    for( std::vector< uint32_t >::const_iterator i = requests.begin();
         i != requests.end(); ++i )
    {
        getLocalNode()->waitRequest( *i );
    }
}

void Config::_stopNodes()
{
    // wait for the nodes to stop, destroy entities, disconnect
    NodeVector stoppingNodes;
    const NodeVector& nodes = getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->getState() != Node::STATE_STOPPED )
            continue;

        if( node->isApplicationNode( ))
        {
            node->setNode( 0 );
            continue;
        }

        net::NodePtr netNode = node->getNode();
        if( !netNode ) // already disconnected
            continue;

        EQLOG( LOG_INIT ) << "Exiting node" << std::endl;

        stoppingNodes.push_back( node );
        EQASSERT( !node->isActive( ));
        EQASSERT( netNode.isValid( ));

        fabric::ServerDestroyConfigPacket destroyConfigPacket;
        destroyConfigPacket.configID = getID();
        netNode->send( destroyConfigPacket );

        ClientExitPacket clientExitPacket;
        netNode->send( clientExitPacket );
    }

    // now wait that the render clients disconnect
    uint32_t nSleeps = 50; // max 5 seconds for all clients
    for( NodeVector::const_iterator i = stoppingNodes.begin();
         i != stoppingNodes.end(); ++i )
    {
        Node*        node    = *i;
        net::NodePtr netNode = node->getNode();

        node->setNode( 0 );

        if( nSleeps )
        {
            while( netNode->isConnected() && --nSleeps )
            {
                base::sleep( 100 ); // ms
            }
        }

        if( netNode->isConnected( ))
        {
            net::NodePtr localNode = getLocalNode();
            EQASSERT( localNode.isValid( ));

            EQWARN << "Forcefully disconnecting exited render client node"
                   << std::endl;
            localNode->close( netNode );
        }

        EQLOG( LOG_INIT ) << "Disconnected node" << std::endl;
    }
}

uint32_t Config::_createConfig( Node* node )
{
    EQASSERT( !node->isApplicationNode( ));
    EQASSERT( node->isActive( ));

    // create config (session) on each non-app node
    //   app-node already has config from chooseConfig
    fabric::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID = getID();
    createConfigPacket.requestID = getLocalNode()->registerRequest();
    createConfigPacket.proxy.identifier = getProxyID();
    createConfigPacket.proxy.version    = commit();

    net::NodePtr netNode = node->getNode();
    netNode->send( createConfigPacket );

    return createConfigPacket.requestID;
}

void Config::_syncClock()
{
    ConfigSyncClockPacket packet;
    packet.time = getServer()->getTime();

    send( _appNetNode, packet );

    const NodeVector& nodes = getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive( ))
        {
            net::NodePtr netNode = node->getNode();
            EQASSERT( netNode->isConnected( ));

            send( netNode, packet );
        }
    }
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
bool Config::_init( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;
    _currentFrame  = 0;
    _finishedFrame = 0;
    _initID = initID;

    const ObserverVector& observers = getObservers();
    for( ObserverVector::const_iterator i = observers.begin();
         i != observers.end(); ++i )
    {
        Observer* observer = *i;
        observer->init();
    }

    const CanvasVector& canvases = getCanvases();
    for( CanvasVector::const_iterator i = canvases.begin();
         i != canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        canvas->init();
    }

    for( CompoundVector::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->init();
    }

    if( !_updateRunning( ))
        return false;

    _state = STATE_RUNNING;
    return true;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------

bool Config::exit()
{
    if( _state != STATE_RUNNING )
        EQWARN << "Exiting non-initialized config" << std::endl;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_INITIALIZING );
    _state = STATE_EXITING;

    for( CompoundVector::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->exit();
    }

    const CanvasVector& canvases = getCanvases();
    for( CanvasVector::const_iterator i = canvases.begin();
         i != canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        canvas->exit();
    }

    const bool success = _updateRunning();

    ConfigEvent exitEvent;
    exitEvent.data.type = Event::EXIT;
    send( _appNetNode, exitEvent );
    
    _state         = STATE_STOPPED;
    return success;
}

void Config::_startFrame( const uint32_t frameID )
{
    EQASSERT( _state == STATE_RUNNING );

    ++_currentFrame;
    EQLOG( base::LOG_ANY ) << "----- Start Frame ----- " << _currentFrame
                           << std::endl;

    for( CompoundVector::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->update( _currentFrame );
    }
    
    ConfigUpdateDataVisitor configDataVisitor;
    accept( configDataVisitor );

    const NodeVector& nodes = getNodes();
    bool appNodeRunning = false;
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive( ))
        {
            node->update( frameID, _currentFrame );
            if( node->isApplicationNode( ))
                appNodeRunning = true;
        }
    }

    if( !appNodeRunning ) // release appNode local sync
    {
        ConfigReleaseFrameLocalPacket packet;
        packet.frameNumber = _currentFrame;
        send( _appNetNode, packet );
    }

    // Fix 2976899: Config::finishFrame deadlocks when no nodes are active
    notifyNodeFrameFinished( _currentFrame );
}

void Config::notifyNodeFrameFinished( const uint32_t frameNumber )
{
    if( _finishedFrame >= frameNumber ) // node finish already done
        return;

    const NodeVector& nodes = getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Node* node = *i;
        if( node->isActive() && node->getFinishedFrame() < frameNumber )
            return;
    }

    _finishedFrame = frameNumber;

    // All nodes have finished the frame. Notify the application's config that
    // the frame is finished
    ConfigFrameFinishPacket packet;
    packet.frameNumber = frameNumber;
    packet.sessionID   = getID();

    // do not use send/_bufferedTasks, not thread-safe!
    _appNetNode->send( packet );
    EQLOG( LOG_TASKS ) << "TASK config frame finished  " << &packet
                           << std::endl;
}

void Config::_flushAllFrames()
{
    if( _currentFrame == 0 )
        return;

    const NodeVector& nodes = getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive( ))
            node->flushFrames( _currentFrame );
    }

    EQLOG( base::LOG_ANY ) << "--- Flush All Frames -- " << std::endl;
}

void Config::changeLatency( const uint32_t latency )
{
    if( getLatency() == latency )
        return;

    setLatency( latency );

    // update latency on all frames and barriers
    ChangeLatencyVisitor visitor( latency );
    accept( visitor );
}


//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Config::_cmdInit( net::Command& command )
{
    const ConfigInitPacket* packet =
        command.getPacket<ConfigInitPacket>();
    EQVERB << "handle config start init " << packet << std::endl;

    ConfigInitReplyPacket reply( packet );
    reply.result = _init( packet->initID );
    if( !reply.result )
        exit();

    sync( net::VERSION_HEAD );
    EQINFO << "Config init " << (reply.result ? "successful": "failed: ") 
           << getErrorMessage() << std::endl;

    reply.version = commit();
    send( command.getNode(), reply );
    setErrorMessage( "" );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdExit( net::Command& command ) 
{
    const ConfigExitPacket* packet = 
        command.getPacket<ConfigExitPacket>();
    ConfigExitReplyPacket   reply( packet );
    EQVERB << "handle config exit " << packet << std::endl;

    if( _state == STATE_RUNNING )
        reply.result = exit();
    else
        reply.result = false;

    EQINFO << "config exit result: " << reply.result << std::endl;
    send( command.getNode(), reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdStartFrame( net::Command& command ) 
{
    const ConfigStartFramePacket* packet = 
        command.getPacket<ConfigStartFramePacket>();
    EQVERB << "handle config frame start " << packet << std::endl;

    ConfigSyncVisitor syncer;
    accept( syncer );

    if( _updateRunning( ))
        _startFrame( packet->frameID );
    else
    {
        EQWARN << "Start frame failed, exiting config: " 
               << getErrorMessage() << std::endl;
        exit();
        ++_currentFrame;
    }
    
    if( packet->requestID != EQ_ID_INVALID ) // unlock app
    {
        ConfigStartFrameReplyPacket reply( packet );
        send( command.getNode(), reply );
    }

    if( _state == STATE_STOPPED )
    {
        // unlock app
        ConfigFrameFinishPacket frameFinishPacket;
        frameFinishPacket.frameNumber = _currentFrame;
        send( command.getNode(), frameFinishPacket );        
    }

    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdFinishAllFrames( net::Command& command ) 
{
    const ConfigFinishAllFramesPacket* packet = 
        command.getPacket<ConfigFinishAllFramesPacket>();
    EQVERB << "handle config all frames finish " << packet << std::endl;

    _flushAllFrames();
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdCreateReply( net::Command& command ) 
{
    const fabric::ConfigCreateReplyPacket* packet = 
        command.getPacket< fabric::ConfigCreateReplyPacket >();

    getLocalNode()->serveRequest( packet->requestID );
    return net::COMMAND_HANDLED;
}

namespace
{
class FreezeVisitor : public ConfigVisitor
{
public:
    // No need to go down on nodes.
    virtual VisitorResult visitPre( Node* node ) { return TRAVERSE_PRUNE; }

    FreezeVisitor( const bool freeze ) : _freeze( freeze ) {}

    virtual VisitorResult visit( Compound* compound )
        { 
            const EqualizerVector& equalizers = compound->getEqualizers();
            for( EqualizerVector::const_iterator i = equalizers.begin();
                 i != equalizers.end(); ++i )
            {
                (*i)->setFrozen( _freeze );
            }
            return TRAVERSE_CONTINUE; 
        }

private:
    const bool _freeze;
};
}

net::CommandResult Config::_cmdFreezeLoadBalancing( net::Command& command ) 
{
    const ConfigFreezeLoadBalancingPacket* packet = 
        command.getPacket<ConfigFreezeLoadBalancingPacket>();

    FreezeVisitor visitor( packet->freeze );
    accept( visitor );

    return net::COMMAND_HANDLED;
}

void Config::output( std::ostream& os ) const
{
    os << base::disableFlush << base::disableHeader;

    for( CompoundVector::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        os << **i;
    }

    os << base::enableHeader << base::enableFlush;
}

}
}

#include "nodeFactory.h"
#include "../lib/fabric/config.ipp"
template class eq::fabric::Config< eq::server::Server, eq::server::Config,
                                   eq::server::Observer, eq::server::Layout,
                                   eq::server::Canvas, eq::server::Node,
                                   eq::server::ConfigVisitor >;

/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::server::Super& );
/** @endcond */

#define FIND_ID_TEMPLATE1( type )                                       \
    template void eq::server::Super::find< type >( const uint32_t, type** );

FIND_ID_TEMPLATE1( eq::server::Canvas );
FIND_ID_TEMPLATE1( eq::server::Channel );
FIND_ID_TEMPLATE1( eq::server::Layout );
FIND_ID_TEMPLATE1( eq::server::Node );
FIND_ID_TEMPLATE1( eq::server::Observer );
FIND_ID_TEMPLATE1( eq::server::Pipe );
FIND_ID_TEMPLATE1( eq::server::Segment );
FIND_ID_TEMPLATE1( eq::server::View );
FIND_ID_TEMPLATE1( eq::server::Window );

#define FIND_ID_TEMPLATE2( type )                                       \
    template type* eq::server::Super::find< type >( const uint32_t );

FIND_ID_TEMPLATE2( eq::server::Canvas );
FIND_ID_TEMPLATE2( eq::server::Channel );
FIND_ID_TEMPLATE2( eq::server::Layout );
FIND_ID_TEMPLATE2( eq::server::Node );
FIND_ID_TEMPLATE2( eq::server::Observer );
FIND_ID_TEMPLATE2( eq::server::Pipe );
FIND_ID_TEMPLATE2( eq::server::Segment );
FIND_ID_TEMPLATE2( eq::server::View );
FIND_ID_TEMPLATE2( eq::server::Window );


#define FIND_NAME_TEMPLATE1( type )\
    template void eq::server::Super::find< type >( const std::string&,  \
                                                   const type** ) const;
FIND_NAME_TEMPLATE1( eq::server::Canvas );
FIND_NAME_TEMPLATE1( eq::server::Channel );
FIND_NAME_TEMPLATE1( eq::server::Layout );
FIND_NAME_TEMPLATE1( eq::server::Node );
FIND_NAME_TEMPLATE1( eq::server::Observer );
FIND_NAME_TEMPLATE1( eq::server::Pipe );
FIND_NAME_TEMPLATE1( eq::server::Segment );
FIND_NAME_TEMPLATE1( eq::server::View );
FIND_NAME_TEMPLATE1( eq::server::Window );

#define FIND_NAME_TEMPLATE2( type )                                     \
    template type* eq::server::Super::find< type >( const std::string& );

FIND_NAME_TEMPLATE2( eq::server::Canvas );
FIND_NAME_TEMPLATE2( eq::server::Channel );
FIND_NAME_TEMPLATE2( eq::server::Layout );
FIND_NAME_TEMPLATE2( eq::server::Node );
FIND_NAME_TEMPLATE2( eq::server::Observer );
FIND_NAME_TEMPLATE2( eq::server::Pipe );
FIND_NAME_TEMPLATE2( eq::server::Segment );
FIND_NAME_TEMPLATE2( eq::server::View );
FIND_NAME_TEMPLATE2( eq::server::Window );

#define CONST_FIND_NAME_TEMPLATE2( type )                               \
    template const type* eq::server::Super::find< type >( const std::string& ) \
        const;

CONST_FIND_NAME_TEMPLATE2( eq::server::Canvas );
CONST_FIND_NAME_TEMPLATE2( eq::server::Channel );
CONST_FIND_NAME_TEMPLATE2( eq::server::Layout );
CONST_FIND_NAME_TEMPLATE2( eq::server::Node );
CONST_FIND_NAME_TEMPLATE2( eq::server::Observer );
CONST_FIND_NAME_TEMPLATE2( eq::server::Pipe );
CONST_FIND_NAME_TEMPLATE2( eq::server::Segment );
CONST_FIND_NAME_TEMPLATE2( eq::server::View );
CONST_FIND_NAME_TEMPLATE2( eq::server::Window );

