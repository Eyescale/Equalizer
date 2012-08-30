
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/client/configEvent.h>
#include <eq/client/error.h>

#include <eq/fabric/commands.h>
#include <eq/fabric/iAttribute.h>
#include <eq/fabric/packetType.h>
#include <eq/fabric/paths.h>

#include <co/objectCommand.h>

#include <lunchbox/sleep.h>

#include "channelStopFrameVisitor.h"
#include "configDeregistrator.h"
#include "configRegistrator.h"
#include "configUpdateVisitor.h"
#include "configUpdateSyncVisitor.h"
#include "nodeFailedVisitor.h"

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
        , _incarnation( 1 )
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
                     ConfigFunc( this, &Config::_cmdStopFrames ), mainQ );
    registerCommand( fabric::CMD_CONFIG_FINISH_ALL_FRAMES,
                     ConfigFunc( this, &Config::_cmdFinishAllFrames ), mainQ );
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
    LBASSERT( node );
    return node ? node->getNode() : 0;
}

void Config::activateCanvas( Canvas* canvas )
{
    LBASSERT( canvas->isStopped( ));
    LBASSERT( stde::find( getCanvases(), canvas ) != getCanvases().end( ));

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

                if( !viewport.hasArea( ))
                {
                    LBLOG( LOG_VIEW )
                        << "View " << view->getName() << view->getViewport()
                        << " doesn't intersect " << segment->getName()
                        << segment->getViewport() << std::endl;

                    continue;
                }

                Channel* segmentChannel = segment->getChannel();
                if( !segmentChannel )
                {
                    LBWARN << "Segment " << segment->getName()
                           << " has no output channel" << std::endl;
                    continue;
                }

                // create and add new channel
                LBASSERT( !findChannel( segment, view ));
                Channel* channel = new Channel( *segmentChannel );
                channel->init(); // not in ctor, virtual method
                channel->setOutput( view, segment );

                //----- compute channel viewport:
                // segment/view intersection in canvas space...
                Viewport contribution = viewport;
                // ... in segment space...
                contribution.transform( segment->getViewport( ));

                // segment output area
                if( segmentChannel->hasFixedViewport( ))
                {
                    Viewport subViewport = segmentChannel->getViewport();
                    LBASSERT( subViewport.isValid( ));
                    if( !subViewport.isValid( ))
                        subViewport = eq::fabric::Viewport::FULL;

                    // ...our part of it
                    subViewport.apply( contribution );
                    channel->setViewport( subViewport );
                    LBLOG( LOG_VIEW )
                        << "View @" << (void*)view << ' ' << view->getViewport()
                        << " intersects " << segment->getName()
                        << segment->getViewport() << " at " << subViewport
                        << " using channel @" << (void*)channel << std::endl;
                }
                else
                {
                    PixelViewport pvp = segmentChannel->getPixelViewport();
                    LBASSERT( pvp.isValid( ));
                    pvp.apply( contribution );
                    channel->setPixelViewport( pvp );
                    LBLOG( LOG_VIEW )
                        << "View @" << (void*)view << ' ' << view->getViewport()
                        << " intersects " << segment->getName()
                        << segment->getViewport() << " at " << pvp
                        << " using channel @" << (void*)channel << std::endl;
                }

                if( channel->getWindow()->isAttached() )
                    // parent is already registered - register channel as well
                    getServer()->registerObject( channel );
            }
        }
    }
}

void Config::updateCanvas( Canvas* canvas )
{
    postNeedsFinish();
    activateCanvas( canvas );

    // Create one compound group for all new output channels of each layout
    const Layouts& layouts = canvas->getLayouts();
    for( LayoutsCIter i = layouts.begin(); i != layouts.end(); ++i )
    {
        Compound* group = new Compound( this );

        const Layout* layout = *i;
        const Views& views = layout->getViews();
        for( ViewsCIter j = views.begin(); j != views.end(); ++j )
        {
            const View* view = *j;
            const Channels& channels = view->getChannels();

            if( channels.empty( ))
                LBWARN << "New view without destination channels will be ignored"
                       << std::endl;

            for( ChannelsCIter k = channels.begin(); k != channels.end(); ++k )
            {
                Channel* channel = *k;
                LBASSERT( !channel->isActive( ));

                Compound* compound = new Compound( group );
                compound->setIAttribute( Compound::IATTR_STEREO_MODE,
                                         fabric::AUTO );
                compound->setChannel( channel );
            }
        }
        group->init();
    }

    canvas->init();
    LBINFO << *this << std::endl;
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

template< class T > bool Config::_postDelete( const UUID& id )
{
    T* child = find< T >( id );
    if( !child )
        return false;

    child->postDelete();
    return true;
}

void Config::removeChild( const UUID& id )
{
    LBASSERT( isRunning( ));

    if( _postDelete< Observer >( id ) || _postDelete< Layout >( id ) ||
        _postDelete< Canvas >( id ))
    {
        return;
    }
    LBUNIMPLEMENTED;
}

void Config::addCompound( Compound* compound )
{
    LBASSERT( compound->_config == this );
    _compounds.push_back( compound );
}

bool Config::removeCompound( Compound* compound )
{
    LBASSERT( compound->_config == this );
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
        LBASSERT( _state == STATE_UNUSED );
        _state = STATE_STOPPED;
        setAppNodeID( netNode->getNodeID( ));
    }
    else
    {
        LBASSERT( _state == STATE_STOPPED );
        _state = STATE_UNUSED;
        setAppNodeID( co::NodeID::ZERO );
    }

    Node* node = findApplicationNode();
    LBASSERT( node );
    if( node )
        node->setNode( netNode );
}

Channel* Config::getChannel( const ChannelPath& path )
{
    Nodes nodes = getNodes();
    LBASSERTINFO( nodes.size() > path.nodeIndex,
                  nodes.size() << " <= " << path.nodeIndex );

    if( nodes.size() <= path.nodeIndex )
        return 0;

    return nodes[ path.nodeIndex ]->getChannel( path );
}

Segment* Config::getSegment( const SegmentPath& path )
{
    Canvas* canvas = getCanvas( path );
    LBASSERT( canvas );

    if( canvas )
        return canvas->getSegment( path );

    return 0;
}

View* Config::getView( const ViewPath& path )
{
    Layout* layout = getLayout( path );
    LBASSERT( layout );

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

uint128_t Config::commit()
{
    return Super::commit( _incarnation );
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

    const bool canFail = (getIAttribute( IATTR_ROBUSTNESS ) != OFF);

    LBASSERT( _state == STATE_RUNNING || _state == STATE_INITIALIZING ||
              _state == STATE_EXITING );

    if( !_connectNodes() && !canFail )
        return false;

    _startNodes();
    _updateCanvases();
    const bool result = _updateNodes() ? true : canFail;
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

bool Config::_connectNodes()
{
    bool success = true;
    lunchbox::Clock clock;
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( !node->isActive( ))
            continue;

        if( !node->connect( ))
        {
            setError( node->getError( ));
            success = false;
            break;
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
            LBASSERT( !node->isActive() || node->getState() == STATE_FAILED ||
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

void Config::_stopNodes()
{
    // wait for the nodes to stop, destroy entities, disconnect
    Nodes stoppingNodes;
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        const State state = node->getState();
        if( state != STATE_STOPPED && state != STATE_FAILED )
            continue;

        LBASSERT( !node->isActive() || state == STATE_FAILED );
        if( node->isApplicationNode( ))
            continue;

        co::NodePtr netNode = node->getNode();
        if( !netNode ) // already disconnected
            continue;

        LBLOG( LOG_INIT ) << "Exiting node" << std::endl;

        if( state == STATE_FAILED )
            node->setState( STATE_STOPPED );

        stoppingNodes.push_back( node );
        LBASSERT( netNode.isValid( ));

        netNode->send( fabric::CMD_SERVER_DESTROY_CONFIG,
                       fabric::PACKETTYPE_EQ_SERVER )
                << getID() << LB_UNDEFINED_UINT32;

        netNode->send( fabric::CMD_CLIENT_EXIT, fabric::PACKETTYPE_EQ_CLIENT );
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
            while( netNode->isConnected() && --nSleeps )
                lunchbox::sleep( 100 ); // ms

        if( netNode->isConnected( ))
        {
            co::LocalNodePtr localNode = getLocalNode();
            LBASSERT( localNode.isValid( ));

            LBWARN << "Forcefully disconnecting exited render client node"
                   << std::endl;
            localNode->disconnect( netNode );
        }

        LBLOG( LOG_INIT ) << "Disconnected node" << std::endl;
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
        LBASSERT( !result );
        accept( syncUpdate );
        if( !syncUpdate.getResult( ))
            setError( syncUpdate.getError( ));
        LBASSERT( !syncUpdate.needsSync( ));
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
            LBASSERT( entity->isAttached( ));
            getServer()->deregisterObject( entity );
            delete entity;
        }
        else
            ++i;
    }
}

uint32_t Config::_createConfig( Node* node )
{
    LBASSERT( !node->isApplicationNode( ));
    LBASSERT( node->isActive( ));

    // create config on each non-app node
    //   app-node already has config from chooseConfig
    const uint32_t requestID = getLocalNode()->registerRequest();
    node->getNode()->send( fabric::CMD_SERVER_CREATE_CONFIG,
                           fabric::PACKETTYPE_EQ_SERVER )
            << co::ObjectVersion( this ) << requestID;

    return requestID;
}

void Config::_syncClock()
{
    const int64_t time = getServer()->getTime();

    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isRunning() || node->isApplicationNode( ))
        {
            LBASSERT( node->isApplicationNode() || node->isActive( ));
            co::NodePtr netNode = node->getNode();
            LBASSERT( netNode->isConnected( ));

            send( netNode, fabric::CMD_CONFIG_SYNC_CLOCK ) << time;
        }
    }
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
bool Config::_init( const uint128_t& initID )
{
    LBASSERT( _state == STATE_STOPPED );
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
        LBWARN << "Exiting non-initialized config" << std::endl;

    LBASSERT( _state == STATE_RUNNING || _state == STATE_INITIALIZING );
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

    send( findApplicationNetNode(), fabric::CMD_CONFIG_EVENT ) << Event::EXIT;

    _needsFinish = false;
    _state = STATE_STOPPED;
    return success;
}

//---------------------------------------------------------------------------
// frame
//---------------------------------------------------------------------------
void Config::_startFrame( const uint128_t& frameID )
{
    LBASSERT( _state == STATE_RUNNING );
    _verifyFrameFinished( _currentFrame );
    _syncClock();

    ++_currentFrame;
    ++_incarnation;
    LBLOG( lunchbox::LOG_ANY ) << "----- Start Frame ----- " << _currentFrame
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
        node->update( frameID, _currentFrame );
        if( node->isRunning() && node->isApplicationNode( ))
            appNode = 0; // release sent (see below)
    }

    if( appNode.isValid( )) // release appNode local sync
        send( appNode,
              fabric::CMD_CONFIG_RELEASE_FRAME_LOCAL ) << _currentFrame;

    // Fix 2976899: Config::finishFrame deadlocks when no nodes are active
    notifyNodeFrameFinished( _currentFrame );
}

void Config::_verifyFrameFinished( const uint32_t frameNumber )
{
    const Nodes& nodes = getNodes();
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isRunning() &&
            node->getFinishedFrame() + getLatency() < frameNumber )
        {
            NodeFailedVisitor nodeFailedVisitor;
            node->accept( nodeFailedVisitor );
        }
    }
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
            LBASSERT( _needsFinish || node->isActive( ));
            return;
        }
    }

    _finishedFrame = frameNumber;

    // All nodes have finished the frame. Notify the application's config that
    // the frame is finished

    // do not use send/_bufferedTasks, not thread-safe!
    send( findApplicationNetNode(),
          fabric::CMD_CONFIG_FRAME_FINISH ) << frameNumber;
    LBLOG( LOG_TASKS ) << "TASK config frame finished  " << " frame "
                       << frameNumber << std::endl;
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

    LBLOG( lunchbox::LOG_ANY ) << "--- Flush All Frames -- " << std::endl;
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
bool Config::_cmdInit( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LB_TS_THREAD( _mainThread );
    LBVERB << "handle config start init " << command << std::endl;

    sync();
    setError( ERROR_NONE );
    commit();

    const uint128_t initID = command.get< uint128_t >();
    const uint32_t requestID = command.get< uint32_t >();

    const bool result = _init( initID );
    if( !result )
        exit();

    sync();
    LBINFO << "Config init " << (result ? "successful: ": "failed: ")
           << getError() << std::endl;

    const uint128_t version = commit();
    send( command.getNode(),
          fabric::CMD_CONFIG_INIT_REPLY ) << version << requestID << result;
    setError( ERROR_NONE );
    return true;
}

bool Config::_cmdExit( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "handle config exit " << command << std::endl;
    setError( ERROR_NONE );

    bool result;
    if( _state == STATE_RUNNING )
        result = exit();
    else
        result = false;

    LBINFO << "config exit result: " << result << std::endl;
    send( command.getNode(), fabric::CMD_CONFIG_EXIT_REPLY )
            << command.get< uint32_t >() << result;
    return true;
}

bool Config::_cmdUpdate( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "handle config update " << command << std::endl;

    const uint32_t versionID = command.get< uint32_t >();
    const uint32_t finishID = command.get< uint32_t >();

    sync();
    setError( ERROR_NONE );
    commit();

    co::NodePtr node = command.getNode();
    if( !_needsFinish )
    {
        send( node, fabric::CMD_CONFIG_UPDATE_VERSION )
                << getVersion() << versionID << finishID << LB_UNDEFINED_UINT32;
        return true;
    }

    co::LocalNodePtr localNode = getLocalNode();
    const uint32_t requestID = localNode->registerRequest();

    send( node, fabric::CMD_CONFIG_UPDATE_VERSION )
            << getVersion() << versionID << finishID << requestID;

    _flushAllFrames();
    _finishedFrame.waitEQ( _currentFrame ); // wait for render clients idle
    localNode->waitRequest( requestID ); // wait for app sync
    _needsFinish = false;

    const bool result = _updateRunning();
    if( !result && getIAttribute( IATTR_ROBUSTNESS ) == OFF )
    {
        LBWARN << "Config update failed, exiting config: " << getError()
               << std::endl;
        exit();
    }

    const uint128_t version = commit();
    send( command.getNode(), fabric::CMD_CONFIG_UPDATE_REPLY )
            << version << command.get< uint32_t >() << result;
    return true;
}

bool Config::_cmdStartFrame( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "handle config frame start " << command << std::endl;

    _startFrame( command.get< uint128_t >( ));

    if( _state == STATE_STOPPED )
    {
        // unlock app
        send( command.getNode(),
              fabric::CMD_CONFIG_FRAME_FINISH ) << _currentFrame;
    }
    return true;
}

bool Config::_cmdFinishAllFrames( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "handle config all frames finish " << command << std::endl;

    _flushAllFrames();
    return true;
}

bool Config::_cmdStopFrames( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LBVERB << "handle config stop frames " << command << std::endl;

    ChannelStopFrameVisitor visitor( _currentFrame );
    accept( visitor );

    return true;
}

bool Config::_cmdCreateReply( co::Command& cmd )
{
    co::ObjectCommand command( cmd );

    LB_TS_THREAD( _cmdThread );
    LB_TS_NOT_THREAD( _mainThread );

    getLocalNode()->serveRequest( command.get< uint32_t >( ));
    return true;
}

void Config::output( std::ostream& os ) const
{
    os << std::endl << lunchbox::disableFlush << lunchbox::disableHeader;

    for( Compounds::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        os << **i;
    }

    os << lunchbox::enableHeader << lunchbox::enableFlush;
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

