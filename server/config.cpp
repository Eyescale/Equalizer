
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
#include "configSerializer.h"
#include "equalizers/equalizer.h"
#include "global.h"
#include "idFinder.h"
#include "layout.h"
#include "log.h"
#include "node.h"
#include "observer.h"
#include "paths.h"
#include "segment.h"
#include "server.h"
#include "view.h"
#include "window.h"

#include <eq/client/configEvent.h>
#include <eq/fabric/paths.h>
#include <eq/net/command.h>
#include <eq/net/global.h>
#include <eq/base/sleep.h>

#include "configSyncVisitor.h"
#include "configUnmapVisitor.h"
#include "nameFinder.h"

using eq::net::ConnectionDescriptionVector;

namespace eq
{
namespace server
{
typedef net::CommandFunc<Config> ConfigFunc;
typedef fabric::Config< Server, Config, Observer > Super;

#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_CONFIG_") + #attr )
std::string Config::_fAttributeStrings[FATTR_ALL] = 
{
    MAKE_ATTR_STRING( FATTR_EYE_BASE ),
    MAKE_ATTR_STRING( FATTR_VERSION ),
    MAKE_ATTR_STRING( FATTR_FILL1 ),
    MAKE_ATTR_STRING( FATTR_FILL2 )
};

void Config::_construct()
{
    _latency       = 1;
    _currentFrame  = 0;
    _finishedFrame = 0;
    _state         = STATE_STOPPED;
    _appNode       = 0;
    _serializer    = 0;

    EQINFO << "New config @" << (void*)this << std::endl;
}

Config::Config( ServerPtr parent )
        : Super( parent )
{
    _construct();
    const Global* global = Global::instance();    
    for( int i=0; i<FATTR_ALL; ++i )
        _fAttributes[i] = global->getConfigFAttribute( (FAttribute)i );
}

Config::Config( const Config& from, ServerPtr parent )
        : Super( from, parent )
{
    _construct();
    _appNetNode = from._appNetNode;
    _latency    = from._latency;
    for( int i=0; i<FATTR_ALL; ++i )
        _fAttributes[i] = from.getFAttribute( (FAttribute)i );

    const NodeVector& nodes = from.getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Node* node      = *i;
        Node*       nodeClone = new Node( *node, this );
        
        if( node == from._appNode )
            _appNode = nodeClone;
    }

    const LayoutVector& layouts = from.getLayouts();
    for( LayoutVector::const_iterator i = layouts.begin(); 
         i != layouts.end(); ++i )
    {
        new Layout( **i, this );
    }

    const CanvasVector& canvases = from.getCanvases();
    for( CanvasVector::const_iterator i = canvases.begin(); 
         i != canvases.end(); ++i )
    {
        new Canvas( **i, this );
    }

    const CompoundVector& compounds = from.getCompounds();
    for( CompoundVector::const_iterator i = compounds.begin(); 
         i != compounds.end(); ++i )
    {
        new Compound( **i, this, 0 );
    }
}

Config::~Config()
{
    EQINFO << "Delete config @" << (void*)this << std::endl;
    _appNode    = 0;
    _appNetNode = 0;

    for( CompoundVector::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->_config = 0;
        delete compound;
    }
    _compounds.clear();

    for( CanvasVector::const_iterator i = _canvases.begin(); 
         i != _canvases.end(); ++i )
    {
        Canvas* canvas = *i;

        canvas->_config = 0;
        delete canvas;
    }
    _canvases.clear();

    while( !_layouts.empty( ))
    {
        Layout* layout = _layouts.back();
        _removeLayout( layout );
        delete layout;
    }

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;

        node->_config = 0;
        delete node;
    }
    _nodes.clear();
}

void Config::notifyMapped( net::NodePtr node )
{
    net::Session::notifyMapped( node );

    net::CommandQueue* serverQueue  = getServerThreadQueue();
    net::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( CMD_CONFIG_INIT,
                     ConfigFunc( this, &Config::_cmdInit), serverQueue );
    registerCommand( CMD_CONFIG_EXIT,
                     ConfigFunc( this, &Config::_cmdExit ), serverQueue );
    registerCommand( CMD_CONFIG_CREATE_REPLY,
                     ConfigFunc( this, &Config::_cmdCreateReply ),
                     commandQueue );
    registerCommand( CMD_CONFIG_START_FRAME, 
                     ConfigFunc( this, &Config::_cmdStartFrame ), serverQueue );
    registerCommand( CMD_CONFIG_FINISH_ALL_FRAMES, 
                     ConfigFunc( this, &Config::_cmdFinishAllFrames ),
                     serverQueue );
    registerCommand( CMD_CONFIG_FREEZE_LOAD_BALANCING, 
                     ConfigFunc( this, &Config::_cmdFreezeLoadBalancing ), 
                     serverQueue );
    registerCommand( CMD_CONFIG_UNMAP_REPLY,
                     ConfigFunc( this, &Config::_cmdUnmapReply ), 
                     commandQueue );
    registerCommand( CMD_CONFIG_CHANGE_LATENCY, 
                     ConfigFunc( this, &Config::_cmdChangeLatency ), 
                     serverQueue );
}

void Config::addNode( Node* node )
{
    _nodes.push_back( node ); 
    
    node->_config = this; 
}

bool Config::removeNode( Node* node )
{
    NodeVector::iterator i = find( _nodes.begin(), _nodes.end(), node );
    if( i == _nodes.end( ))
        return false;

    _nodes.erase( i );
    node->_config = 0; 

    return true;
}

void Config::_addLayout( Layout* layout )
{
    EQASSERT( layout->getConfig() == this );
    _layouts.push_back( layout );
}

bool Config::_removeLayout( Layout* layout )
{
    LayoutVector::iterator i = find( _layouts.begin(), _layouts.end(), layout );
    if( i == _layouts.end( ))
        return false;

    EQASSERT( layout->getConfig() == this );
    _layouts.erase( i );
    return true;
}


Layout* Config::findLayout( const std::string& name )
{
    LayoutFinder finder( name );
    accept( finder );
    return finder.getResult();
}
const Layout* Config::findLayout( const std::string& name ) const
{
    ConstLayoutFinder finder( name );
    accept( finder );
    return finder.getResult();
}
Layout* Config::findLayout( const uint32_t id )
{
    LayoutIDFinder finder( id );
    accept( finder );
    return finder.getResult();
}

View* Config::findView( const std::string& name )
{
    ViewFinder finder( name );
    accept( finder );
    return finder.getResult();
}

const View* Config::findView( const std::string& name ) const
{
    ConstViewFinder finder( name );
    accept( finder );
    return finder.getResult();
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

Channel* Config::findChannel( const Segment* segment, const View* view )
{
    ChannelViewFinder finder( segment, view );
    accept( finder );
    return finder.getResult();
}

void Config::addCanvas( Canvas* canvas )
{
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

    canvas->_config = this;
    _canvases.push_back( canvas );
}

bool Config::removeCanvas( Canvas* canvas )
{
    CanvasVector::iterator i = find( _canvases.begin(), _canvases.end(),
                                     canvas );
    if( i == _canvases.end( ))
        return false;

    _canvases.erase( i );
    canvas->_config = 0;
    return true;
}

Canvas* Config::findCanvas( const std::string& name )
{
    CanvasFinder finder( name );
    accept( finder );
    return finder.getResult();
}

const Canvas* Config::findCanvas( const std::string& name ) const
{
    ConstCanvasFinder finder( name );
    accept( finder );
    return finder.getResult();
}

Segment* Config::findSegment( const std::string& name )
{
    SegmentFinder finder( name );
    accept( finder );
    return finder.getResult();
}
const Segment* Config::findSegment( const std::string& name ) const
{
    ConstSegmentFinder finder( name );
    accept( finder );
    return finder.getResult();
}


void Config::addCompound( Compound* compound )
{
    compound->_config = this;
    _compounds.push_back( compound );
}

bool Config::removeCompound( Compound* compound )
{
    CompoundVector::iterator i = find( _compounds.begin(), _compounds.end(),
                                       compound );
    if( i == _compounds.end( ))
        return false;

    _compounds.erase( i );
    compound->_config = 0;
    return true;
}

Channel* Config::findChannel( const std::string& name )
{
    ChannelFinder finder( name );
    accept( finder );
    return finder.getResult();
}

const Channel* Config::findChannel( const std::string& name ) const
{
    ConstChannelFinder finder( name );
    accept( finder );
    return finder.getResult();
}

void Config::addApplicationNode( Node* node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERTINFO( !_appNode, "Only one application node per config possible" );

    _appNode = node;
    addNode( node );
}

void Config::setApplicationNetNode( net::NodePtr node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERT( !_appNetNode );

    _appNetNode = node;
}

Channel* Config::getChannel( const ChannelPath& path )
{
    EQASSERTINFO( _nodes.size() > path.nodeIndex,
                  _nodes.size() << " <= " << path.nodeIndex );

    if( _nodes.size() <= path.nodeIndex )
        return 0;

    return _nodes[ path.nodeIndex ]->getChannel( path );
}

Canvas* Config::getCanvas( const CanvasPath& path )
{
    EQASSERTINFO( _canvases.size() > path.canvasIndex,
                  _canvases.size() << " <= " << path.canvasIndex );

    if( _canvases.size() <= path.canvasIndex )
        return 0;

    return _canvases[ path.canvasIndex ];
}

Segment* Config::getSegment( const SegmentPath& path )
{
    Canvas* canvas = getCanvas( path );
    EQASSERT( canvas );

    if( canvas )
        return canvas->getSegment( path );

    return 0;
}

Layout* Config::getLayout( const LayoutPath& path )
{
    EQASSERTINFO( _layouts.size() > path.layoutIndex,
                  _layouts.size() << " <= " << path.layoutIndex );

    if( _layouts.size() <= path.layoutIndex )
        return 0;

    return _layouts[ path.layoutIndex ];
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


//===========================================================================
// operations
//===========================================================================

uint32_t Config::getDistributorID()
{
    EQASSERT( !_serializer );

    _serializer = new ConfigSerializer( this );
    registerObject( _serializer );
    return _serializer->getID();
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

    _error.clear();

    if( !_connectNodes( ))
        return false;

    _startNodes();

    // Let all running nodes update their running state (incl. children)
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
        (*i)->updateRunning( _initID, _currentFrame );

    // Sync state updates
    bool success = true;
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( !node->syncRunning( ))
        {
            _error += "node " + node->getName() + ": '" +
                          node->getErrorMessage() + '\'';
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

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive() && !_connectNode( node ))
        {
            success = false;
            break;
        }
    }

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
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
        node->getIAttribute( eq::Node::IATTR_LAUNCH_TIMEOUT ));
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
    
    if( node == _appNode )
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
        
        _error += nodeString.str();
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
        _error += "Connection of node failed, node did not start ( " +
            data.str() + ") ";
        EQERROR << _error << std::endl;

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
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        const Node::State state = node->getState();

        if( node->isActive() && state != Node::STATE_RUNNING )
        {
            EQASSERT( state == Node::STATE_STOPPED );
            startingNodes.push_back( node );
            if( node != _appNode )
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
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->getState() != Node::STATE_STOPPED || node == _appNode )
            continue;

        net::NodePtr netNode = node->getNode();
        if( !netNode ) // already disconnected
            continue;

        EQLOG( LOG_INIT ) << "Exiting node" << std::endl;

        stoppingNodes.push_back( node );
        EQASSERT( !node->isActive( ));
        EQASSERT( netNode.isValid( ));

        ServerDestroyConfigPacket destroyConfigPacket;
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

        if( netNode->getState() == net::Node::STATE_CONNECTED )
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
    EQASSERT( node != _appNode );
    EQASSERT( node->isActive( ));

    // create config (session) on each non-app node
    //   app-node already has config from chooseConfig
    ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID  = getID();
    createConfigPacket.appNodeID = _appNetNode->getNodeID();
    createConfigPacket.requestID = getLocalNode()->registerRequest();

    const std::string&   name = getName();
    net::NodePtr netNode = node->getNode();
    netNode->send( createConfigPacket, name );

    return createConfigPacket.requestID;
}

void Config::_syncClock()
{
    ConfigSyncClockPacket packet;
    packet.time = getServer()->getTime();

    send( _appNetNode, packet );

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
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

    for( CanvasVector::const_iterator i = _canvases.begin();
         i != _canvases.end(); ++i )
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

    for( CanvasVector::const_iterator i = _canvases.begin();
         i != _canvases.end(); ++i )
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

void Config::unmap()
{
    net::NodePtr localNode = getLocalNode();
    ConfigUnmapPacket packet;
    packet.requestID = localNode->registerRequest();
    send( _appNetNode, packet );
    localNode->waitRequest( packet.requestID );

    if( _serializer ) // Config::init never happened
    {
        deregisterObject( _serializer );
        delete _serializer;
        _serializer = 0;
    }

    ConfigUnmapVisitor unmapper;
    accept( unmapper );
}

void Config::_startFrame( const uint32_t frameID )
{
    EQASSERT( _state == STATE_RUNNING );

    ++_currentFrame;
    EQLOG( base::LOG_ANY ) << "----- Start Frame ----- " << _currentFrame
                           << std::endl;

    if( !_appNode || !_appNode->isActive( )) // release appNode local sync
    {
        ConfigReleaseFrameLocalPacket packet;
        packet.frameNumber = _currentFrame;
        send( _appNetNode, packet );
    }

    for( CompoundVector::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->update( _currentFrame );
    }
    
    ConfigUpdateDataVisitor configDataVisitor;
    accept( configDataVisitor );

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive( ))
            node->update( frameID, _currentFrame );
    }
}

void Config::notifyNodeFrameFinished( const uint32_t frameNumber )
{
    if( _finishedFrame >= frameNumber ) // node finish already done
        return;

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
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

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive( ))
            node->flushFrames( _currentFrame );
    }

    EQLOG( base::LOG_ANY ) << "--- Flush All Frames -- " << std::endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Config::_cmdInit( net::Command& command )
{
    // clients have retrieved distributed data
    EQASSERT( _serializer );
    deregisterObject( _serializer );
    delete _serializer;
    _serializer = 0;

    const ConfigInitPacket* packet =
        command.getPacket<ConfigInitPacket>();
    EQVERB << "handle config start init " << packet << std::endl;

    ConfigInitReplyPacket reply( packet );
    reply.result = _init( packet->initID );
    std::string error = _error;

    if( !reply.result )
        exit();

    EQINFO << "Config init " << (reply.result ? "successful": "failed: ") 
           << error << std::endl;

    send( command.getNode(), reply, error );
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

    ConfigSyncVisitor syncer( packet->nChanges, packet->changes );
    accept( syncer );

    if( _updateRunning( ))
        _startFrame( packet->frameID );
    else
    {
        EQWARN << "Start frame failed, exiting config: " << _error << std::endl;
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
    const ConfigCreateReplyPacket* packet = 
        command.getPacket<ConfigCreateReplyPacket>();

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

net::CommandResult Config::_cmdChangeLatency( net::Command& command )
{
    const ConfigChangeLatency* packet = 
        command.getPacket<ConfigChangeLatency>();

    _latency = packet->latency;

    // update latency on all frame and barrier
    ChangeLatencyVisitor changeLatency( _latency );
    accept( changeLatency );

    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdUnmapReply( net::Command& command ) 
{
    const ConfigUnmapReplyPacket* packet = 
        command.getPacket< ConfigUnmapReplyPacket >();
    EQVERB << "Handle unmap reply " << packet << std::endl;

    getLocalNode()->serveRequest( packet->requestID );
    return net::COMMAND_HANDLED;
}


std::ostream& operator << ( std::ostream& os, const Config* config )
{
    if( !config )
        return os;

    os << base::disableFlush << base::disableHeader << "config " << std::endl;
    os << "{" << std::endl << base::indent;

    if( !config->getName().empty( ))
        os << "name    \"" << config->getName() << '"' << std::endl;

    if( config->getLatency() != 1 )
        os << "latency " << config->getLatency() << std::endl;
    os << std::endl;

    EQASSERTINFO( config->getFAttribute( Config::FATTR_VERSION ) ==
                  Global::instance()->getConfigFAttribute(Config::FATTR_VERSION)
                  , "Per-config versioning not implemented" );

    const float value = config->getFAttribute( Config::FATTR_EYE_BASE );
    if( value != 
        Global::instance()->getConfigFAttribute( Config::FATTR_EYE_BASE ))
    {
        os << "attributes" << std::endl << "{" << std::endl << base::indent
           << "eye_base     " << value << std::endl
           << base::exdent << "}" << std::endl;
    }

    const NodeVector& nodes = config->getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
        os << *i;

    const ObserverVector& observers = config->getObservers();
    for( ObserverVector::const_iterator i = observers.begin(); 
         i !=observers.end(); ++i )
    {
        os << *i;
    }
    const LayoutVector& layouts = config->getLayouts();
    for( LayoutVector::const_iterator i = layouts.begin(); 
         i !=layouts.end(); ++i )
    {
        os << *i;
    }
    const CanvasVector& canvases = config->getCanvases();
    for( CanvasVector::const_iterator i = canvases.begin(); 
         i != canvases.end(); ++i )
    {
        os << *i;
    }

    const CompoundVector& compounds = config->getCompounds();
    for( CompoundVector::const_iterator i = compounds.begin(); 
         i != compounds.end(); ++i )
    {
        os << *i;
    }
    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;

    return os;
}

}
}

#include "../lib/fabric/config.cpp"
template class eq::fabric::Config< eq::server::Server, eq::server::Config,
                                   eq::server::Observer >;

