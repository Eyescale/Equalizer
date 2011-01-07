
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

#include <eq/clientPackets.h>
#include <eq/configEvent.h>
#include <eq/configPackets.h>
#include <eq/error.h>
#include <eq/fabric/configPackets.h>
#include <eq/fabric/iAttribute.h>
#include <eq/fabric/paths.h>
#include <eq/fabric/serverPackets.h>
#include <co/command.h>
#include <co/base/sleep.h>

#include "channelStopFrameVisitor.h"
#include "configDeregistrator.h"
#include "configRegistrator.h"
#include "configUpdateVisitor.h"
#include "configUpdateSyncVisitor.h"

namespace eq
{
namespace server
{
typedef co::CommandFunc<Config> ConfigFunc;
using fabric::ON;
using fabric::OFF;

Config::Config( ServerPtr parent )
        : Super( parent )
        , _currentFrame( 0 )
        , _finishedFrame( 0 )
        , _state( STATE_UNUSED )
        , _needsFinish( false )
{
    const Global* global = Global::instance();
    for( int i=0; i<FATTR_ALL; ++i )
    {
        const FAttribute attr = static_cast< FAttribute >( i );
        setFAttribute( attr, global->getConfigFAttribute( attr ));
    }
    for( int i=0; i<IATTR_ALL; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getConfigIAttribute( attr ));
    }
}

Config::~Config()
{
    while( !_compounds.empty( ))
    {
        Compound* compound = _compounds.back();
        removeCompound( compound );
        delete compound;
    }
}

void Config::attach( const UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    
    co::CommandQueue* mainQ = getMainThreadQueue();
    co::CommandQueue* cmdQ = getCommandThreadQueue();

    registerCommand( fabric::CMD_CONFIG_INIT,
                     ConfigFunc( this, &Config::_cmdInit), mainQ );
    registerCommand( fabric::CMD_CONFIG_EXIT,
                     ConfigFunc( this, &Config::_cmdExit ), mainQ );
    registerCommand( fabric::CMD_CONFIG_UPDATE,
                     ConfigFunc( this, &Config::_cmdUpdate ), mainQ );
    registerCommand( fabric::CMD_CONFIG_CREATE_REPLY,
                     ConfigFunc( this, &Config::_cmdCreateReply ), cmdQ );
    registerCommand( fabric::CMD_CONFIG_START_FRAME, 
                     ConfigFunc( this, &Config::_cmdStartFrame ), mainQ );
    registerCommand( fabric::CMD_CONFIG_STOP_FRAMES, 
                     ConfigFunc( this, &Config::_cmdStopFrames ), cmdQ );
    registerCommand( fabric::CMD_CONFIG_FINISH_ALL_FRAMES, 
                     ConfigFunc( this, &Config::_cmdFinishAllFrames ), mainQ );
    registerCommand( fabric::CMD_CONFIG_FREEZE_LOAD_BALANCING, 
                     ConfigFunc( this, &Config::_cmdFreezeLoadBalancing ),
                     mainQ );
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

Node* Config::findApplicationNode()
{
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isApplicationNode( ))
            return node;
    }
    return 0;
}

co::NodePtr Config::findApplicationNetNode()
{
    Node* node = findApplicationNode();
    EQASSERT( node );
    return node ? node->getNode() : 0;
}

void Config::activateCanvas( Canvas* canvas )
{
    EQASSERT( canvas->isStopped( ));
    EQASSERT( stde::find( getCanvases(), canvas ) != getCanvases().end( ));

    const Layouts& layouts = canvas->getLayouts();
    const Segments& segments = canvas->getSegments();

    for( Layouts::const_iterator i = layouts.begin();
         i != layouts.end(); ++i )
    {
        const Layout* layout = *i;
        if( !layout )
            continue;

        const Views& views = layout->getViews();
        for( Views::const_iterator j = views.begin(); 
             j != views.end(); ++j )
        {
            View* view = *j;

            for( Segments::const_iterator k = segments.begin();
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
                      
                Channel* segmentChannel = segment->getChannel();
                if (!segmentChannel)
                {
                    EQWARN << "Segment " << segment->getName()
                           << " has no output channel" << std::endl;
                    continue;
                }

                // create and add new channel
                EQASSERT( !findChannel( segment, view ));
                Channel* channel = new Channel( *segmentChannel );
                channel->init(); // not in ctor, virtual method
                channel->setOutput( view, segment );

                //----- compute channel viewport:
                // segment/view intersection in canvas space...
                Viewport contribution = viewport;
                // ... in segment space...
                contribution.transform( segment->getViewport( ));
            
                // segment output area
                Viewport subViewport = segmentChannel->getViewport();
                if( !subViewport.isValid( ))
                    subViewport = eq::fabric::Viewport::FULL;

                // ...our part of it
                subViewport.apply( contribution );

                channel->setViewport( subViewport );
                if( channel->getWindow()->isAttached() )
                    // parent is already registered - register channel as well
                    getServer()->registerObject( channel );

                EQLOG( LOG_VIEW ) 
                    << "View @" << (void*)view << ' ' << view->getViewport()
                    << " intersects " << segment->getName()
                    << segment->getViewport() << " at " << subViewport
                    << " using channel @" << (void*)channel << std::endl;
            }
        }
    }
}

void Config::updateCanvas( Canvas* canvas )
{
    postNeedsFinish();
    activateCanvas( canvas );

    // Create compounds for all new output channels
    const Segments& segments = canvas->getSegments();
    Compound* group = new Compound( this );

    for( Segments::const_iterator i=segments.begin(); i != segments.end(); ++i )
    {
        const Segment* segment = *i;
        const Channels& channels = segment->getDestinationChannels();

        if( channels.empty( ))
            EQWARN << "New segment without destination channels will be ignored"
                   << std::endl;
        
        for( Channels::const_iterator j = channels.begin();
             j != channels.end(); ++j )
        {
            Channel* channel = *j;
            EQASSERT( !channel->isActive( ));

            Compound* compound = new Compound( group );
            compound->setChannel( channel );
        }
    }

    group->init();
    canvas->init();
    EQINFO << *this << std::endl;
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

template< class T > bool Config::_postDelete( const co::base::UUID& id )
{
    T* child = find< T >( id );
    if( !child )
        return false;

    child->postDelete();
    return true;
}

void Config::removeChild( const co::base::UUID& id )
{
    EQASSERT( isRunning( ));

    if( _postDelete< Observer >( id ) || _postDelete< Layout >( id ) ||
        _postDelete< Canvas >( id ))
    {
        return;
    }
    EQUNIMPLEMENTED;
}

void Config::addCompound( Compound* compound )
{
    EQASSERT( compound->_config == this );
    _compounds.push_back( compound );
}

bool Config::removeCompound( Compound* compound )
{
    EQASSERT( compound->_config == this );
    Compounds::iterator i = stde::find( _compounds, compound );
    if( i == _compounds.end( ))
        return false;

    _compounds.erase( i );
    return true;
}

void Config::setApplicationNetNode( co::NodePtr netNode )
{
    if( netNode.isValid( ))
    {
        EQASSERT( _state == STATE_UNUSED );
        _state = STATE_STOPPED;
        setAppNodeID( netNode->getNodeID( ));
    }
    else
    {
        EQASSERT( _state == STATE_STOPPED );
        _state = STATE_UNUSED;
        setAppNodeID( co::NodeID::ZERO );
    }

    Node* node = findApplicationNode();
    EQASSERT( node );
    if( node )
        node->setNode( netNode );
}

Channel* Config::getChannel( const ChannelPath& path )
{
    Nodes nodes = getNodes();
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
    const Compounds& compounds = config->getCompounds();
    for( Compounds::const_iterator i = compounds.begin();
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

void Config::register_()
{
    ServerPtr server = getServer();
    ConfigRegistrator registrator;
    accept( registrator );
}

void Config::deregister()
{
    sync();
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
// update running entities (init/exit/runtime change)
//---------------------------------------------------------------------------
bool Config::_updateRunning()
{
    if( _state == STATE_STOPPED )
        return true;

    const bool failValue =
        (getIAttribute( IATTR_ROBUSTNESS ) == OFF) ? false : true;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_INITIALIZING ||
              _state == STATE_EXITING );

    if( !_connectNodes() && !failValue )
        return false;

    _startNodes();
    _updateCanvases();
    const bool result = _updateNodes() ? true : failValue;
    _stopNodes();

    // Don't use visitor, it would get confused with modified child vectors
    _deleteEntities( getCanvases( ));
    _deleteEntities( getLayouts( ));
    _deleteEntities( getObservers( ));
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Pipes& pipes = (*i)->getPipes();
        for( Pipes::const_iterator j = pipes.begin(); j != pipes.end(); ++j )
        {
            const Windows& windows = (*j)->getWindows();
            _deleteEntities( windows );
        }
    }

    return result;
}

void Config::_updateCanvases()
{
    const Canvases& canvases = getCanvases();
    for( Canvases::const_iterator i = canvases.begin(); i != canvases.end();++i)
    {
        Canvas* canvas = *i;
        if( canvas->needsDelete( ))
            canvas->exit();
    }
}

void Config::_startNodes()
{
    // start up newly running nodes
    std::vector< uint32_t > requests;
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;

        if( node->isActive() && node->isStopped( ))
        {
            if( !node->isApplicationNode( ))
                requests.push_back( _createConfig( node ));
        }
        else
        {
            EQASSERT( !node->isActive() || node->getState() == STATE_FAILED ||
                      node->getState() == STATE_RUNNING );
        }
    }

    // sync create config requests on starting nodes
    for( std::vector< uint32_t >::const_iterator i = requests.begin();
         i != requests.end(); ++i )
    {
        getLocalNode()->waitRequest( *i );
    }
}

//----- connect new nodes
bool Config::_connectNodes()
{
    bool success = true;
    co::base::Clock clock;
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive( ))
        {
            if( !node->connect( ))
            {
                setError( node->getError( ));
                success = false;
                break;
            }
        }
    }

    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive() && !node->syncLaunch( clock ))
        {
            setError( node->getError( ));
            success = false;
        }
    }

    return success;
}


void Config::_stopNodes()
{
    // wait for the nodes to stop, destroy entities, disconnect
    Nodes stoppingNodes;
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        const State state = node->getState();

        if( !node->isActive() && state == STATE_FAILED )
            node->setState( STATE_STOPPED );

        if( state != STATE_STOPPED )
            continue;

        EQASSERT( !node->isActive( ));
        if( node->isApplicationNode( ))
            continue;

        co::NodePtr netNode = node->getNode();
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
    for( Nodes::const_iterator i = stoppingNodes.begin();
         i != stoppingNodes.end(); ++i )
    {
        Node*        node    = *i;
        co::NodePtr netNode = node->getNode();
        node->setNode( 0 );

        if( nSleeps )
        {
            while( netNode->isConnected() && --nSleeps )
            {
                co::base::sleep( 100 ); // ms
            }
        }

        if( netNode->isConnected( ))
        {
            co::LocalNodePtr localNode = getLocalNode();
            EQASSERT( localNode.isValid( ));

            EQWARN << "Forcefully disconnecting exited render client node"
                   << std::endl;
            localNode->disconnect( netNode );
        }

        EQLOG( LOG_INIT ) << "Disconnected node" << std::endl;
    }
}

bool Config::_updateNodes()
{
    ConfigUpdateVisitor update( _initID, _currentFrame );
    accept( update );
    
    ConfigUpdateSyncVisitor syncUpdate;
    accept( syncUpdate );

    const bool result = syncUpdate.getResult();
    if( !result )
        setError( syncUpdate.getError( ));

    if( syncUpdate.needsSync( )) // init failure, call again (exit pending)
    {
        EQASSERT( !result );
        accept( syncUpdate );
        if( !syncUpdate.getResult( ))
            setError( syncUpdate.getError( ));
        EQASSERT( !syncUpdate.needsSync( ));
    }

    return result;
}

template< class T >
void Config::_deleteEntities( const std::vector< T* >& entities )
{
    for( size_t i = 0; i < entities.size(); ) // don't use iterator! (delete)
    {
        T* entity = entities[ i ];
        if( entity->needsDelete( ))
        {
            getServer()->deregisterObject( entity );
            delete entity;
        }
        else
            ++i;
    }
}

uint32_t Config::_createConfig( Node* node )
{
    EQASSERT( !node->isApplicationNode( ));
    EQASSERT( node->isActive( ));

    // create config on each non-app node
    //   app-node already has config from chooseConfig
    fabric::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.requestID = getLocalNode()->registerRequest();
    createConfigPacket.configVersion = this;

    co::NodePtr netNode = node->getNode();
    netNode->send( createConfigPacket );

    return createConfigPacket.requestID;
}

void Config::_syncClock()
{
    ConfigSyncClockPacket packet;
    packet.time = getServer()->getTime();

    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isRunning() || node->isApplicationNode( ))
        {
            EQASSERT( node->isApplicationNode() || node->isActive( ));
            co::NodePtr netNode = node->getNode();
            EQASSERT( netNode->isConnected( ));

            send( netNode, packet );
        }
    }
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
bool Config::_init( const uint128_t& initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;
    _currentFrame  = 0;
    _finishedFrame = 0;
    _initID = initID;

    for( CompoundsCIter i = _compounds.begin(); i != _compounds.end(); ++i )
        (*i)->init();

    const Observers& observers = getObservers();
    for( ObserversCIter i = observers.begin(); i != observers.end(); ++i )
        (*i)->init();

    const Canvases& canvases = getCanvases();
    for( CanvasesCIter i = canvases.begin(); i != canvases.end(); ++i )
        (*i)->init();

    if( !_updateRunning( ))
        return false;

    // Needed to set up active state for first LB update
    for( CompoundsCIter i = _compounds.begin(); i != _compounds.end(); ++i )
        (*i)->update( 0 );

    _needsFinish = false;
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

    const Canvases& canvases = getCanvases();
    for( Canvases::const_iterator i = canvases.begin();
         i != canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        canvas->exit();
    }

    for( Compounds::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->exit();
    }

    const bool success = _updateRunning();

    ConfigEvent exitEvent;
    exitEvent.data.type = Event::EXIT;
    send( findApplicationNetNode(), exitEvent );
    
    _needsFinish = false;
    _state = STATE_STOPPED;
    return success;
}

//---------------------------------------------------------------------------
// frame
//---------------------------------------------------------------------------
void Config::_startFrame( const uint128_t& frameID )
{
    EQASSERT( _state == STATE_RUNNING );
    
    _syncClock();

    ++_currentFrame;
    EQLOG( co::base::LOG_ANY ) << "----- Start Frame ----- " << _currentFrame
                           << std::endl;

    for( Compounds::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->update( _currentFrame );
    }
    
    ConfigUpdateDataVisitor configDataVisitor;
    accept( configDataVisitor );

    const Nodes& nodes = getNodes();
    co::NodePtr appNode = findApplicationNetNode();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isRunning( ))
        {
            EQASSERT( node->isActive( ));
            node->update( frameID, _currentFrame );
            if( node->isApplicationNode( ))
                appNode = 0; // release sent (see below)
        }
    }

    if( appNode.isValid( )) // release appNode local sync
    {
        ConfigReleaseFrameLocalPacket packet;
        packet.frameNumber = _currentFrame;
        send( appNode, packet );
    }

    // Fix 2976899: Config::finishFrame deadlocks when no nodes are active
    notifyNodeFrameFinished( _currentFrame );
}

void Config::notifyNodeFrameFinished( const uint32_t frameNumber )
{
    if( _finishedFrame >= frameNumber ) // node finish already done
        return;

    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Node* node = *i;
        if( node->isRunning() && node->getFinishedFrame() < frameNumber )
        {
            EQASSERT( node->isActive( ));
            return;
        }
    }

    _finishedFrame = frameNumber;

    // All nodes have finished the frame. Notify the application's config that
    // the frame is finished
    ConfigFrameFinishPacket packet;
    packet.frameNumber = frameNumber;

    // do not use send/_bufferedTasks, not thread-safe!
    send( findApplicationNetNode(), packet );
    EQLOG( LOG_TASKS ) << "TASK config frame finished  " << &packet
                           << std::endl;
}

void Config::_flushAllFrames()
{
    if( _currentFrame == 0 )
        return;

    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isRunning( ))
            node->flushFrames( _currentFrame );
    }

    EQLOG( co::base::LOG_ANY ) << "--- Flush All Frames -- " << std::endl;
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
bool Config::_cmdInit( co::Command& command )
{
    EQ_TS_THREAD( _mainThread );
    const ConfigInitPacket* packet =
        command.getPacket<ConfigInitPacket>();
    EQVERB << "handle config start init " << packet << std::endl;

    sync();
    setError( ERROR_NONE );
    commit();

    ConfigInitReplyPacket reply( packet );
    reply.result = _init( packet->initID );

    if( !reply.result )
        exit();

    sync( co::VERSION_HEAD );
    EQINFO << "Config init " << (reply.result ? "successful: ": "failed: ") 
           << getError() << std::endl;

    reply.version = commit();
    send( command.getNode(), reply );
    setError( ERROR_NONE );
    return true;
}

bool Config::_cmdExit( co::Command& command ) 
{
    const ConfigExitPacket* packet = 
        command.getPacket<ConfigExitPacket>();
    ConfigExitReplyPacket   reply( packet );
    EQVERB << "handle config exit " << packet << std::endl;
    setError( ERROR_NONE );

    if( _state == STATE_RUNNING )
        reply.result = exit();
    else
        reply.result = false;

    EQINFO << "config exit result: " << reply.result << std::endl;
    send( command.getNode(), reply );
    return true;
}

bool Config::_cmdUpdate( co::Command& command )
{
    const ConfigUpdatePacket* packet = 
        command.getPacket<ConfigUpdatePacket>();

    EQVERB << "handle config update " << packet << std::endl;

    sync();
    setError( ERROR_NONE );
    commit();    

    co::NodePtr node = command.getNode();
    if( !_needsFinish )
    {
        ConfigUpdateVersionPacket reply( packet, getVersion(),
                                         EQ_UNDEFINED_UINT32 );
        send( node, reply );
        return true;
    }

    co::LocalNodePtr localNode = getLocalNode();
    ConfigUpdateVersionPacket replyVersion( packet, getVersion(),
                                            localNode->registerRequest( ));
    send( node, replyVersion );
    
    _flushAllFrames();
    _finishedFrame.waitEQ( _currentFrame ); // wait for render clients idle
    localNode->waitRequest( replyVersion.requestID ); // wait for app sync
    _needsFinish = false;

    ConfigUpdateReplyPacket reply( packet );
    reply.result = _updateRunning();
    if( !reply.result && getIAttribute( IATTR_ROBUSTNESS ) == OFF )
    {
        EQWARN << "Config update failed, exiting config: " << getError()
               << std::endl;
        exit();
    }
    EQINFO << "Updated " << *this << std::endl;

    reply.version = commit();
    send( command.getNode(), reply );
    return true;
}

bool Config::_cmdStartFrame( co::Command& command ) 
{
    const ConfigStartFramePacket* packet = 
        command.getPacket<ConfigStartFramePacket>();
    EQVERB << "handle config frame start " << packet << std::endl;

    _startFrame( packet->frameID );

    if( _state == STATE_STOPPED )
    {
        // unlock app
        ConfigFrameFinishPacket frameFinishPacket;
        frameFinishPacket.frameNumber = _currentFrame;
        send( command.getNode(), frameFinishPacket );
    }

    return true;
}

bool Config::_cmdFinishAllFrames( co::Command& command ) 
{
    const ConfigFinishAllFramesPacket* packet = 
        command.getPacket<ConfigFinishAllFramesPacket>();
    EQVERB << "handle config all frames finish " << packet << std::endl;

    _flushAllFrames();
    return true;
}

bool Config::_cmdStopFrames( co::Command& command )
{
    const ConfigStopFramesPacket* packet = 
        command.getPacket<ConfigStopFramesPacket>();
    EQVERB << "handle config stop frames " << packet << std::endl;

    ChannelStopFrameVisitor visitor( _currentFrame );
    accept( visitor );

    return true;
}

bool Config::_cmdCreateReply( co::Command& command ) 
{
    EQ_TS_THREAD( _cmdThread );
    EQ_TS_NOT_THREAD( _mainThread );
    const fabric::ConfigCreateReplyPacket* packet = 
        command.getPacket< fabric::ConfigCreateReplyPacket >();

    getLocalNode()->serveRequest( packet->requestID );
    return true;
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
            const Equalizers& equalizers = compound->getEqualizers();
            for( Equalizers::const_iterator i = equalizers.begin();
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

bool Config::_cmdFreezeLoadBalancing( co::Command& command ) 
{
    const ConfigFreezeLoadBalancingPacket* packet = 
        command.getPacket<ConfigFreezeLoadBalancingPacket>();

    FreezeVisitor visitor( packet->freeze );
    accept( visitor );

    return true;
}

void Config::output( std::ostream& os ) const
{
    os << co::base::disableFlush << co::base::disableHeader;

    for( Compounds::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        os << **i;
    }

    os << co::base::enableHeader << co::base::enableFlush;
}

}
}

#include "nodeFactory.h"
#include "../fabric/config.ipp"
template class eq::fabric::Config< eq::server::Server, eq::server::Config,
                                   eq::server::Observer, eq::server::Layout,
                                   eq::server::Canvas, eq::server::Node,
                                   eq::server::ConfigVisitor >;

/** @cond IGNORE */
template std::ostream& eq::fabric::operator <<
    ( std::ostream&, const eq::server::Config::Super& );
/** @endcond */

#define FIND_ID_TEMPLATE1( type )                                       \
    template void eq::server::Config::Super::find< type >( const uint128_t&, \
                                                           type** );

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
    template type* eq::server::Config::Super::find< type >( const uint128_t& );

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
    template void eq::server::Config::Super::find< type >( const std::string&, \
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
    template type*                                                      \
    eq::server::Config::Super::find< type >( const std::string& );

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
    template const type*                                                \
    eq::server::Config::Super::find< type >( const std::string& ) const;

CONST_FIND_NAME_TEMPLATE2( eq::server::Canvas );
CONST_FIND_NAME_TEMPLATE2( eq::server::Channel );
CONST_FIND_NAME_TEMPLATE2( eq::server::Layout );
CONST_FIND_NAME_TEMPLATE2( eq::server::Node );
CONST_FIND_NAME_TEMPLATE2( eq::server::Observer );
CONST_FIND_NAME_TEMPLATE2( eq::server::Pipe );
CONST_FIND_NAME_TEMPLATE2( eq::server::Segment );
CONST_FIND_NAME_TEMPLATE2( eq::server::View );
CONST_FIND_NAME_TEMPLATE2( eq::server::Window );

