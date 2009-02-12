
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "config.h"

#include "canvas.h"
#include "compound.h"
#include "compoundVisitor.h"
#include "configUpdateDataVisitor.h"
#include "configSerializer.h"
#include "constCompoundVisitor.h"
#include "global.h"
#include "layout.h"
#include "loadBalancer.h"
#include "log.h"
#include "nameFinder.h"
#include "node.h"
#include "paths.h"
#include "segment.h"
#include "server.h"
#include "view.h"
#include "window.h"

#include <eq/net/command.h>
#include <eq/net/global.h>
#include <eq/base/sleep.h>

using namespace eq::base;
using namespace std;
using eq::net::ConnectionDescriptionVector;

namespace eq
{
namespace server
{
typedef net::CommandFunc<Config> ConfigFunc;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_CONFIG_") + #attr )
std::string Config::_fAttributeStrings[FATTR_ALL] = 
{
    MAKE_ATTR_STRING( FATTR_EYE_BASE )
};

void Config::_construct()
{
    _latency       = 1;
    _currentFrame  = 0;
    _finishedFrame = 0;
    _state         = STATE_STOPPED;
    _appNode       = 0;
    _serializer    = 0;

    EQINFO << "New config @" << (void*)this << endl;
}

Config::Config()
{
    _construct();

    const Global* global = Global::instance();    
    for( int i=0; i<FATTR_ALL; ++i )
        _fAttributes[i] = global->getConfigFAttribute( (FAttribute)i );
}

Config::Config( const Config& from )
        : net::Session()
        , _name( from._name )
        , _server( from._server )
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
        Node*       nodeClone = new Node( *node );
        
        (node == from._appNode) ? 
            addApplicationNode( nodeClone ) : addNode( nodeClone );
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
    EQINFO << "Delete config @" << (void*)this << endl;
    _server     = 0;
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

    for( LayoutVector::const_iterator i = _layouts.begin(); 
         i != _layouts.end(); ++i )
    {
        Layout* layout = *i;

        layout->_config = 0;
        delete layout;
    }
    _layouts.clear();

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;

        node->_config = 0;
        delete node;
    }
    _nodes.clear();
}

void Config::setLocalNode( net::NodePtr node )
{
    net::Session::setLocalNode( node );
    
    if( !node ) 
        return;

    net::CommandQueue* serverQueue  = getServerThreadQueue();
    net::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( eq::CMD_CONFIG_START_INIT,
                     ConfigFunc( this, &Config::_cmdStartInit), serverQueue );
    registerCommand( eq::CMD_CONFIG_FINISH_INIT,
                     ConfigFunc(this, &Config::_cmdFinishInit), serverQueue );
    registerCommand( eq::CMD_CONFIG_EXIT,
                     ConfigFunc( this, &Config::_cmdExit ), serverQueue );
    registerCommand( eq::CMD_CONFIG_CREATE_REPLY,
                     ConfigFunc( this, &Config::_cmdCreateReply ),
                     commandQueue );
    registerCommand( eq::CMD_CONFIG_CREATE_NODE_REPLY,
                     ConfigFunc( this, &Config::_cmdCreateNodeReply ),
                     commandQueue );
    registerCommand( eq::CMD_CONFIG_START_FRAME, 
                     ConfigFunc( this, &Config::_cmdStartFrame ), serverQueue );
    registerCommand( eq::CMD_CONFIG_FINISH_ALL_FRAMES, 
                     ConfigFunc( this, &Config::_cmdFinishAllFrames ),
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_FREEZE_LOAD_BALANCING, 
                     ConfigFunc( this, &Config::_cmdFreezeLoadBalancing ), 
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

void Config::addLayout( Layout* layout )
{
    if( layout->getName().empty( ))
    {
        std::stringstream name;
        name << "layout" << _layouts.size() + 1;
        layout->setName( name.str( ));
    }

    layout->_config = this;
    _layouts.push_back( layout );
}

bool Config::removeLayout( Layout* layout )
{
    vector<Layout*>::iterator i = find( _layouts.begin(), _layouts.end(),
                                          layout );
    if( i == _layouts.end( ))
        return false;

    _layouts.erase( i );
//    layout->_config = 0;
    return true;
}


namespace
{
class CompoundCanvasInitVisitor : public ConfigVisitor
{
public:
    CompoundCanvasInitVisitor( Canvas* canvas,
                               std::string channelName )
        : _channelName( channelName )
    {
        _segments = canvas->getSegments() ;
    };
    
    virtual VisitorResult visit( Compound* compound )
    {
        if ( !compound->isDestination() )
            return TRAVERSE_CONTINUE;
        
        Channel* destinationChannel = compound->getChannel();
        EQASSERT( destinationChannel );
        
        if( _channelName != destinationChannel->getName())
            return TRAVERSE_CONTINUE;
        
        for( SegmentVector::const_iterator j = _segments.begin(); 
            j != _segments.end(); ++j )
        {
            Segment* segment = *j;
            Channel* outputChannel = segment->getChannel();
            
            if(  _channelName.find( outputChannel->getName()) != string::npos )
            {   
                // if segment has frustum
                switch ( segment->getCurrentType())
                {
                    case View::TYPE_WALL:
                    {
                        // set compound frustum =
                        //         segment frustum X channel/segment coverage
                        const Viewport& outputViewPortChannel = 
                            outputChannel->getViewport();
                        Viewport computeViewport = 
                            destinationChannel->getViewport( ); 
                        computeViewport.intersect( outputViewPortChannel );
                        computeViewport.transform( outputViewPortChannel );

                        Wall wallSegment = segment->getWall();
                        wallSegment.apply( computeViewport );
                        compound->setWall( wallSegment );
                        EQINFO << "Compound Wall = " << wallSegment
                               << std::endl;
                        break;
                    }
                    case View::TYPE_PROJECTION:

                        EQUNIMPLEMENTED;

                    default: 
                        break;
                }
            }
        }
     return TRAVERSE_CONTINUE;   
    }
private:
    string _channelName;
    SegmentVector _segments;
};
    
class CanvasInitVisitor : public ConfigVisitor
{
public:
    
    CanvasInitVisitor(  Config* config  )
         : _layout( 0 )
         , _canvas( 0 )
         , _config( config ) {}
    virtual ~CanvasInitVisitor(){}

    virtual VisitorResult visitPre( Canvas* canvas )
    {
        _layout = canvas->getLayout();
        
        if( _layout )
        {
            _canvas = canvas;
            _layout->accept( this );
        }
        _layout = 0;

        return TRAVERSE_CONTINUE;
    }
    
    virtual VisitorResult visit( View* view )
    { 
        if( !_layout )
            return TRAVERSE_CONTINUE;
            
        //for each Channel
        ChannelVector channels = view->getChannels();
        
        // for each channel named "channelName.newLayoutName.*"       
        for( ChannelVector::const_iterator i = channels.begin(); 
             i != channels.end(); ++i )
        {
           Channel* channel = *i;
           string channelName = channel->getName();        
           if( channelName.find( '.' + _layout->getName()) != string::npos ) 
           {
               // increase channel, window, pipe, node activation count*/
               channel->activate();
               _setCompoundFrustum( channel );
           }
        }
        
        return TRAVERSE_CONTINUE;
    }
       
    
private:    
    Layout* _layout;
    Canvas* _canvas;
    Config* _config;

    void _setCompoundFrustum(Channel* channel)
    {
        // find compounds where channel is a destination channel
        CompoundVector compounds = _config->getCompounds();

        for( CompoundVector::const_iterator j = compounds.begin();
             j != compounds.end(); ++j )
        {
            CompoundCanvasInitVisitor visitor( _canvas, channel->getName() );
            Compound* compound = *j;
            compound->accept( &visitor, false );
        }
    }
};
}

Layout* Config::findLayout( const std::string& name )
{
    LayoutFinder finder( name );
    accept( &finder );
    return finder.getResult();
}
const Layout* Config::findLayout( const std::string& name ) const
{
    ConstLayoutFinder finder( name );
    accept( &finder );
    return finder.getResult();
}

View* Config::findView( const std::string& name )
{
    ViewFinder finder( name );
    accept( &finder );
    return finder.getResult();
}

const View* Config::findView( const std::string& name ) const
{
    ConstViewFinder finder( name );
    accept( &finder );
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
    accept( &finder );
    return finder.getResult();
}

namespace
{
class AddCanvasVisitor : public ConfigVisitor
{
public:
    AddCanvasVisitor( Canvas* canvas, Config* config  )  
                                       : _canvas( canvas )
                                       , _config( config )
        {}
    virtual ~AddCanvasVisitor() {}

    virtual VisitorResult visit( View* view )
        {
            _view = view;
            _canvas->accept( this );
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visit( Canvas* canvas )
        {
            if( canvas != _canvas ) // only consider our canvas
                return TRAVERSE_PRUNE;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visit( Segment* segment )
        {
            Viewport viewport = segment->getViewport();
            viewport.intersect( _view->getViewport( ));

            if( !viewport.hasArea())
            {
                EQINFO << "View " << _view->getName() << _view->getViewport()
                       << " doesn't intersect " << segment->getName() 
                       << segment->getViewport() << std::endl;
                
                return TRAVERSE_CONTINUE;
            }
                      
            Channel* segmentChannel = segment->getChannel( );
            if (!segmentChannel)
            {
                EQWARN << "Segment " << segment->getName()
                       << " has no output channel" << endl;
                return TRAVERSE_CONTINUE;
            }

            // try to reuse channel
            ChannelViewFinder finder( segment, _view );
            _view->getConfig()->accept( &finder );
            Channel* channel = finder.getResult();

            if( !channel ) // create and add new channel
            {
                channel = new Channel( *segmentChannel );

                _view->addChannel( channel );
                segment->addDestinationChannel( channel );
                
                Window* window = segmentChannel->getWindow();
                window->addChannel( channel );            
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
            
            // decrement channel activation count [inactivates channel]
            // TODO implement useLayout: channel->deactivate(); 
            
            EQINFO << "View " << _view->getName() << _view->getViewport()
                   << " intersects " << segment->getName() 
                   << segment->getViewport() << " at " << subViewport
                   << std::endl;

            return TRAVERSE_CONTINUE;
        }

protected:
    Canvas* const _canvas;
    Config* const _config; // For find channel
    View*         _view; // The current view
};
}

void Config::addCanvas( Canvas* canvas )
{
    if( canvas->getName().empty( ))
    {
        std::stringstream name;
        name << "canvas" << _canvases.size() + 1;
        canvas->setName( name.str( ));
    }

    AddCanvasVisitor visitor( canvas, this );
    accept( &visitor );

    canvas->_config = this;
    _canvases.push_back( canvas );
}

bool Config::removeCanvas( Canvas* canvas )
{
    vector<Canvas*>::iterator i = find( _canvases.begin(), _canvases.end(),
                                          canvas );
    if( i == _canvases.end( ))
        return false;

    _canvases.erase( i );
//    canvas->_config = 0;
    return true;
}

Canvas* Config::findCanvas( const std::string& name )
{
    CanvasFinder finder( name );
    accept( &finder );
    return finder.getResult();
}

Segment* Config::findSegment( const std::string& name )
{
    SegmentFinder finder( name );
    accept( &finder );
    return finder.getResult();
}
const Segment* Config::findSegment( const std::string& name ) const
{
    ConstSegmentFinder finder( name );
    accept( &finder );
    return finder.getResult();
}


void Config::addCompound( Compound* compound )
{
    compound->_config = this;
    _compounds.push_back( compound );
}

bool Config::removeCompound( Compound* compound )
{
    vector<Compound*>::iterator i = find( _compounds.begin(), _compounds.end(),
                                          compound );
    if( i == _compounds.end( ))
        return false;

    _compounds.erase( i );
    compound->_config = 0;
    return true;
}

namespace
{
class CompoundUpdateVisitor : public ConfigVisitor
{
public:
    CompoundUpdateVisitor( View* view,
                              Channel* channel )
        : _view( view)
        , _channel( channel ){}

    virtual VisitorResult visit( Compound* compound )
    {
        if ( !compound->isDestination() )
            return TRAVERSE_CONTINUE;
            
        Channel* channelCompound = compound->getChannel();
            
        if( !( channelCompound && 
             ( channelCompound->getName() == _channel->getName()) &&
             compound->isDestination()))
             return TRAVERSE_CONTINUE;
            
        switch ( _view->getCurrentType())
        {
            case View::TYPE_WALL :
            {
                Viewport ouptutViewPortChannel = _view->getViewport();
                Viewport computeViewport = _channel->getViewport( );
                    
                computeViewport.intersect( ouptutViewPortChannel );
                computeViewport.transform( ouptutViewPortChannel );
                    
                Wall wallView = _view->getWall();
                wallView.apply( computeViewport );
                compound->setWall( wallView );
                    
                EQINFO << "set compound frustum = view frustum X channel/view coverage" << std::endl;
                EQINFO << "Compound " << wallView << std::endl;
                break;
            }
            case View::TYPE_PROJECTION:
            {
                EQUNIMPLEMENTED;
            }
            default: 
                break;  
        }      
        return TRAVERSE_CONTINUE;
    }
private:
    View* _view;
    Channel* _channel;
};
    
class InitCompoundFrustrumView : public ConfigVisitor
{
public:

    InitCompoundFrustrumView( Config* config ): _config( config ){}

    virtual ~InitCompoundFrustrumView(){}

    virtual VisitorResult visit( View* view )
    {
         //for each Channel
         ChannelVector channels = view->getChannels();
    
         // for each channel named "channelName.newLayoutName.*"       
         for( ChannelVector::const_iterator i = channels.begin(); 
                i != channels.end(); ++i )
         {
            Channel* channel = *i;
            CompoundVector compounds = _config->getCompounds();
            for( CompoundVector::const_iterator j = compounds.begin();
                j != compounds.end(); ++j )
            {
               CompoundUpdateVisitor visitor( view, channel );
               Compound* compound = *j;
               compound->accept( &visitor, false );
            }
         }
         return TRAVERSE_CONTINUE;
    }

private:
    Config* _config ;
};


}

Channel* Config::findChannel( const std::string& name )
{
    ChannelFinder finder( name );
    accept( &finder );
    return finder.getResult();
}

const Channel* Config::findChannel( const std::string& name ) const
{
    ConstChannelFinder finder( name );
    accept( &finder );
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
    EQASSERTINFO( _nodes.size() >= path.nodeIndex,
                  _nodes.size() << " < " << path.nodeIndex );

    if( _nodes.size() < path.nodeIndex )
        return 0;

    return _nodes[ path.nodeIndex ]->getChannel( path );
}

Canvas* Config::getCanvas( const CanvasPath& path )
{
    EQASSERTINFO( _canvases.size() >= path.canvasIndex,
                  _canvases.size() << " < " << path.canvasIndex );

    if( _canvases.size() < path.canvasIndex )
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
    EQASSERTINFO( _layouts.size() >= path.layoutIndex,
                  _layouts.size() << " < " << path.layoutIndex );

    if( _layouts.size() < path.layoutIndex )
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
template< class C, class V >
VisitorResult _accept( C* config, V* visitor )
{ 
    VisitorResult result = visitor->visitPre( config );
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
        switch( (*i)->accept( visitor, false /*activeOnly*/ ))
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

    switch( visitor->visitPost( config ))
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

VisitorResult Config::accept( ConfigVisitor* visitor )
{
    return _accept( this, visitor );
}

VisitorResult Config::accept( ConstConfigVisitor* visitor ) const
{
    return _accept( this, visitor );
}


//===========================================================================
// operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
uint32_t Config::getDistributorID()
{
    EQASSERT( !_serializer );

    _serializer = new ConfigSerializer( this );
    registerObject( _serializer );
    return _serializer->getID();
}

bool Config::_startInit( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _currentFrame  = 0;
    _finishedFrame = 0;
    _initID = initID;

    CanvasInitVisitor canvasInit( this );
    accept( &canvasInit );

    InitCompoundFrustrumView compoundInit( this );
    accept( &compoundInit );

    for( vector< Compound* >::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->init();
    }

    if( !_connectNodes() || !_initNodes( initID ))
    {
        exit();
        return false;
    }

    _state = STATE_INITIALIZED;
    return true;
}

static net::NodePtr _createNode( Node* node )
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

    netNode->setAutoLaunch( true );
    return netNode;
}

bool Config::_connectNodes()
{
    net::NodePtr localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( !node->isUsed( ))
            continue;

        net::NodePtr netNode;

        if( node == _appNode )
            netNode = _appNetNode;
        else
        {
            netNode = _createNode( node );
            netNode->setProgramName( _renderClient );
            netNode->setWorkDir( _workDir );
        }

        if( !localNode->initConnect( netNode ))
        {
            stringstream nodeString;
            nodeString << node;

            _error = string( "Connection to node failed, node does not run " ) +
                     string( "and launch command failed:\n " ) + 
                     nodeString.str();
            EQERROR << "Connection to " << netNode->getNodeID() << " failed." 
                    << endl;
            return false;
        }

        node->setNode( netNode );
    }
    return true;
}

bool Config::_initNodes( const uint32_t initID )
{
    const string& name    = getName();
    bool          success = true;

    net::NodePtr localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));

    eq::ServerCreateConfigPacket createConfigPacket;
    deque<uint32_t>              createConfigRequests;
    createConfigPacket.configID  = getID();
    createConfigPacket.appNodeID = _appNetNode->getNodeID();
    createConfigPacket.appNodeID.convertToNetwork();

    // sync node connect and create configs
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( !node->isUsed( ))
            continue;
        
        net::NodePtr netNode = node->getNode();

        if( !localNode->syncConnect( netNode ))
        {
            stringstream nodeString;
            nodeString << node;

            _error = "Connection of node failed, node did not start:\n " +
                     nodeString.str();
            EQERROR << "Connection of node " << node << " failed." << endl;
            success = false;
        }
        
        if( !success ) 
            continue;

        // create config (session) on each non-app node
        //   app-node already has config
        if( node != _appNode )
        {
            createConfigPacket.requestID = _requestHandler.registerRequest();
            createConfigRequests.push_back( createConfigPacket.requestID );

            netNode->send( createConfigPacket, name );
        }
    }

    if( !success )
    {
        // sync already issued requests
        while( !createConfigRequests.empty( ))
        {
            _requestHandler.waitRequest( createConfigRequests.front( ));
            createConfigRequests.pop_front();
        }
        return false;
    }

    // sync config creation and start node init
    eq::ConfigCreateNodePacket createNodePacket;
    vector<uint32_t>           createNodeRequests;
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( !node->isUsed( ))
            continue;

        registerObject( node );

        if( node != _appNode )
        {
            _requestHandler.waitRequest( createConfigRequests.front( ));
            createConfigRequests.pop_front();
        }

        net::NodePtr netNode = node->getNode();

        createNodePacket.nodeID = node->getID();
        createNodePacket.requestID = _requestHandler.registerRequest();
        createNodeRequests.push_back( createNodePacket.requestID );
        send( netNode, createNodePacket );

        // start node-pipe-window-channel init in parallel on all nodes
        node->startConfigInit( initID );

#ifdef EQ_TRANSMISSION_API
        // TODO move me
        nodeIDs.resize( nodeIDs.size() + 1 );
        netNode->getNodeID().getData( nodeIDs.back( ));
#endif
    }

    // Need to sync eq::Node creation: It is possible, though unlikely, that
    // Config::startInit returns and Config::broadcastData is used before the
    // NodeCreatePacket has been processed by the render node.
    for( vector<uint32_t>::const_iterator i = createNodeRequests.begin();
         i != createNodeRequests.end(); ++i )
    {
        _requestHandler.waitRequest( *i );
    }
    return success;
}

bool Config::_finishInit()
{
    bool success = true;
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node*               node    = *i;
        net::NodePtr netNode = node->getNode();
        if( !node->isUsed() || !netNode->isConnected( ))
            continue;
        
        if( !node->syncConfigInit( ))
        {
            _error += ( ' ' + node->getErrorMessage( ));
            EQERROR << "Init of node " << (void*)node << " failed." 
                    << endl;
            success = false;
        }
    }

    // start global clock on all nodes
    eq::ConfigStartClockPacket packet;
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node*               node    = *i;
        net::NodePtr netNode = node->getNode();
        if( !node->isUsed() || !netNode->isConnected( ))
            continue;

        send( netNode, packet );
    }
    send( _appNetNode, packet );
    return success;
}

bool Config::exit()
{
    if( _state != STATE_INITIALIZED )
        EQWARN << "Exiting non-initialized config" << endl;

    bool cleanExit = _exitNodes();
    if( !cleanExit )
        EQERROR << "nodes exit failed" << endl;

    for( vector< Compound* >::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->exit();
    }

    if( _headMatrix.getID() != EQ_ID_INVALID )
        deregisterObject( &_headMatrix );

    _currentFrame  = 0;
    _finishedFrame = 0;
    _state         = STATE_STOPPED;
    return cleanExit;
}

bool Config::_exitNodes()
{
    // invoke configExit task methods and delete resource instances on clients 
    NodeVector exitingNodes;
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node*          node = *i;
        net::NodePtr netNode = node->getNode();

        if( !node->isUsed() || !netNode.isValid() ||
            netNode->getState() == net::Node::STATE_STOPPED )

            continue;

        exitingNodes.push_back( node );
        node->startConfigExit();
    }

    // wait for the above and then delete the config and request disconnect
    eq::ServerDestroyConfigPacket destroyConfigPacket;
    destroyConfigPacket.configID  = getID();

    eq::ConfigDestroyNodePacket destroyNodePacket;
    eq::ClientExitPacket        clientExitPacket;
    net::NodePtr         localNode         = getLocalNode();
    EQASSERT( localNode.isValid( ));

    bool success = true;
    for( NodeVector::const_iterator i = exitingNodes.begin();
         i != exitingNodes.end(); ++i )
    {
        Node*          node    = *i;
        net::NodePtr netNode = node->getNode();

        if( !node->syncConfigExit( ))
        {
            EQERROR << "Could not exit cleanly: " << node << endl;
            success = false;
        }
        
        destroyNodePacket.nodeID = node->getID();
        send( netNode, destroyNodePacket );

        if( node != _appNode )
        {
            netNode->send( destroyConfigPacket );
            netNode->send( clientExitPacket );
        }

        deregisterObject( node );
    }

    // now wait that the clients disconnect
    uint32_t nSleeps = 50; // max 5 seconds for all clients
    for( NodeVector::const_iterator i = exitingNodes.begin();
        i != exitingNodes.end(); ++i )
    {
        Node*        node    = *i;
        net::NodePtr netNode = node->getNode();

        node->setNode( 0 );

        if( node != _appNode )
        {
            while( netNode->getState() == net::Node::STATE_CONNECTED &&
                   nSleeps-- )
            {
                base::sleep( 100 ); // ms
            }

            if( netNode->getState() == net::Node::STATE_CONNECTED )
                localNode->disconnect( netNode );
        }
    }
    return success;
}

void Config::unmap()
{
    if( _serializer ) // Config::init never happened
    {
        _serializer->deregister();
        delete _serializer;
        _serializer = 0;
    }
}

namespace
{
class ViewUpdater : public ConfigVisitor
{
public:
    virtual ~ViewUpdater(){}

    // No need to go down on nodes.
    virtual VisitorResult visitPre( Node* node ) { return TRAVERSE_PRUNE; }

    virtual VisitorResult visit( Compound* compound )
        { 
            if( compound->getLatestFrustum() != eq::Frustum::TYPE_NONE )
                compound->getFrustum().updateHead();

            return TRAVERSE_CONTINUE; 
        }
};
}

void Config::_updateHead()
{
    const uint32_t oldVersion = _headMatrix.getVersion();
    const uint32_t newVersion = _headMatrix.sync();

    if( oldVersion == newVersion )
        return;

    ViewUpdater updater;
    accept( &updater );
}

void Config::_prepareFrame( std::vector< net::NodeID >& nodeIDs )
{
    EQASSERT( _state == STATE_INITIALIZED );
    ++_currentFrame;
    EQLOG( LOG_ANY ) << "----- Start Frame ----- " << _currentFrame << endl;

    _updateHead();

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isUsed( ))
        {
            net::NodePtr netNode = node->getNode();
            nodeIDs.push_back( netNode->getNodeID( ));
        }
    }
}

void Config::_startFrame( const uint32_t frameID )
{
    EQASSERT( _state == STATE_INITIALIZED );

    for( vector< Compound* >::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->update( _currentFrame );
    }
    
    ConfigUpdateDataVisitor configDataVisitor;
    accept( &configDataVisitor );

    for( vector< Node* >::const_iterator i = _nodes.begin(); 
         i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isUsed( ))
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
        if( node->isUsed() && node->getFinishedFrame() < frameNumber )
            return;
    }

    // All nodes have finished the frame. Notify the application's config that
    // the frame is finished
    eq::ConfigFrameFinishPacket packet;
    packet.frameNumber = frameNumber;

    // do not use send/_bufferedTasks, not thread-safe!
    packet.sessionID   = getID();
    _appNetNode->send( packet );
    EQLOG( eq::LOG_TASKS ) << "TASK config frame finished  " << &packet << endl;
}

void Config::_flushFrames()
{
    if( _currentFrame == 0 )
        return;

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isUsed( ))
            node->flushFrames( _currentFrame );
    }

    EQLOG( LOG_ANY ) << "--- Flush All Frames -- " << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Config::_cmdStartInit( net::Command& command )
{
    // clients have retrieved distributed data
    EQASSERT( _serializer );
    _serializer->deregister();
    delete _serializer;
    _serializer = 0;

    const eq::ConfigStartInitPacket* packet = 
        command.getPacket<eq::ConfigStartInitPacket>();
    eq::ConfigStartInitReplyPacket   reply( packet );
    EQINFO << "handle config start init " << packet << endl;

    _error.clear();
    reply.result = _startInit( packet->initID );
    
    EQINFO << "Config start init " << (reply.result ? "successful": "failed: ") 
           << _error << endl;

    send( command.getNode(), reply, _error );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdFinishInit( net::Command& command )
{
    const eq::ConfigFinishInitPacket* packet = 
        command.getPacket<eq::ConfigFinishInitPacket>();
    eq::ConfigFinishInitReplyPacket   reply( packet );
    EQINFO << "handle config finish init " << packet << endl;

    _error.clear();
    reply.result = _finishInit();

    EQINFO << "Config finish init " << (reply.result ? "successful":"failed: ") 
           << _error << endl;

    if( reply.result )
        mapObject( &_headMatrix, packet->headMatrixID );

    send( command.getNode(), reply, _error );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdExit( net::Command& command ) 
{
    const eq::ConfigExitPacket* packet = 
        command.getPacket<eq::ConfigExitPacket>();
    eq::ConfigExitReplyPacket   reply( packet );
    EQINFO << "handle config exit " << packet << endl;

    if( _state == STATE_INITIALIZED )
        reply.result = exit();
    else
        reply.result = false;

    EQINFO << "config exit result: " << reply.result << endl;
    send( command.getNode(), reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdStartFrame( net::Command& command ) 
{
    const eq::ConfigStartFramePacket* packet = 
        command.getPacket<eq::ConfigStartFramePacket>();
    EQVERB << "handle config frame start " << packet << endl;

    vector< net::NodeID > nodeIDs;

    _prepareFrame( nodeIDs );
    _startFrame( packet->frameID );

    eq::ConfigStartFrameReplyPacket reply( packet );
    reply.frameNumber = _currentFrame;
    reply.nNodeIDs    = nodeIDs.size();

    for( vector< net::NodeID >::iterator i = nodeIDs.begin(); 
         i != nodeIDs.end(); ++i )

         (*i).convertToNetwork();

    command.getNode()->send( reply, nodeIDs );

    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdFinishAllFrames( net::Command& command ) 
{
    const eq::ConfigFinishAllFramesPacket* packet = 
        command.getPacket<eq::ConfigFinishAllFramesPacket>();
    EQVERB << "handle config all frames finish " << packet << endl;

    _flushFrames();
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdCreateReply( net::Command& command ) 
{
    const eq::ConfigCreateReplyPacket* packet = 
        command.getPacket<eq::ConfigCreateReplyPacket>();

    _requestHandler.serveRequest( packet->requestID );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdCreateNodeReply( net::Command& command ) 
{
    const eq::ConfigCreateNodeReplyPacket* packet = 
        command.getPacket<eq::ConfigCreateNodeReplyPacket>();

    _requestHandler.serveRequest( packet->requestID );
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

    /** Visit a non-leaf compound on the down traversal. */
    virtual VisitorResult visitPre( Compound* compound )
        { 
            LoadBalancer* loadBalancer = compound->getLoadBalancer();
            if( loadBalancer )
                loadBalancer->setFreeze( _freeze );
            
            return TRAVERSE_CONTINUE; 
        }

private:
    const bool _freeze;
};
}

net::CommandResult Config::_cmdFreezeLoadBalancing( net::Command& command ) 
{
    const eq::ConfigFreezeLoadBalancingPacket* packet = 
        command.getPacket<eq::ConfigFreezeLoadBalancingPacket>();

    FreezeVisitor visitor( packet->freeze );
    accept( &visitor );

    return net::COMMAND_HANDLED;
}

ostream& operator << ( ostream& os, const Config* config )
{
    if( !config )
        return os;

    os << disableFlush << disableHeader << "config " << endl;
    os << "{" << endl << indent;

    if( !config->getName().empty( ))
        os << "name    \"" << config->getName() << '"' << endl;

    if( config->getLatency() != 1 )
        os << "latency " << config->getLatency() << endl;
    os << endl;

    const float value = config->getFAttribute( Config::FATTR_EYE_BASE );
    if( value != 
        Global::instance()->getConfigFAttribute( Config::FATTR_EYE_BASE ))
    {
        os << "attributes" << endl << "{" << endl << indent
           << "eye_base     " << value << endl
           << exdent << "}" << endl;
    }

    const NodeVector& nodes = config->getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
        os << *i;

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
    os << exdent << "}" << endl << enableHeader << enableFlush;

    return os;
}

}
}
