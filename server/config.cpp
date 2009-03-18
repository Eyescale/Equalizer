
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
#include "idFinder.h"
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

#include <eq/client/configEvent.h>
#include <eq/net/command.h>
#include <eq/net/global.h>
#include <eq/base/sleep.h>

#include "configSyncVisitor.h"
#include "configAddCanvasVisitor.h"

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
    MAKE_ATTR_STRING( FATTR_EYE_BASE ),
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

    _eyes[eq::EYE_CYCLOP] = vmml::Vector3f::ZERO;
    _eyes[eq::EYE_LEFT]   = vmml::Vector3f::ZERO;
    _eyes[eq::EYE_RIGHT]  = vmml::Vector3f::ZERO;

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

    registerCommand( eq::CMD_CONFIG_INIT,
                     ConfigFunc( this, &Config::_cmdInit), serverQueue );
    registerCommand( eq::CMD_CONFIG_EXIT,
                     ConfigFunc( this, &Config::_cmdExit ), serverQueue );
    registerCommand( eq::CMD_CONFIG_CREATE_REPLY,
                     ConfigFunc( this, &Config::_cmdCreateReply ),
                     commandQueue );
    registerCommand( eq::CMD_CONFIG_START_FRAME, 
                     ConfigFunc( this, &Config::_cmdStartFrame ), serverQueue );
    registerCommand( eq::CMD_CONFIG_FINISH_ALL_FRAMES, 
                     ConfigFunc( this, &Config::_cmdFinishAllFrames ),
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_FREEZE_LOAD_BALANCING, 
                     ConfigFunc( this, &Config::_cmdFreezeLoadBalancing ), 
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_UNMAP_REPLY,
                     ConfigFunc( this, &Config::_cmdUnmapReply ), 
                     commandQueue );
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

Channel* Config::findChannel( const Segment* segment, const View* view )
{
    ChannelViewFinder finder( segment, view );
    accept( finder );
    return finder.getResult();
}

void Config::addCanvas( Canvas* canvas )
{
    ConfigAddCanvasVisitor visitor( canvas, this );
    accept( visitor );

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
    vector<Compound*>::iterator i = find( _compounds.begin(), _compounds.end(),
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
template< class C, class V >
VisitorResult _accept( C* config, V& visitor )
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

VisitorResult Config::accept( ConstConfigVisitor& visitor ) const
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
        if( node->isActive() && !_syncConnectNode( node ))
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

    netNode->setAutoLaunch( true );
    return netNode;
}
}

bool Config::_connectNode( Node* node )
{
    EQASSERT( node->isActive( ));

    net::NodePtr netNode = node->getNode();
    if( netNode.isValid( ))
        return ( netNode->getState() == net::Node::STATE_CONNECTED );

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
        stringstream nodeString;
        nodeString << node;
        
        _error += string( "Connection to node failed, node does not run " ) +
            string( "and launch command failed:\n " ) + 
            nodeString.str();
        EQERROR << "Connection to " << netNode->getNodeID() << " failed." 
                << endl;
        return false;
    }

    node->setNode( netNode );
    return true;
}

bool Config::_syncConnectNode( Node* node )
{
    EQASSERT( node->isActive( ));

    net::NodePtr netNode = node->getNode();
    if( !netNode )
        return false;

    net::NodePtr localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));

    if( !localNode->syncConnect( netNode ))
    {
        stringstream nodeString;
        nodeString << netNode->serialize();

        _error += "Connection of node failed, node did not start (" +
            nodeString.str() + ") ";
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
        _requestHandler.waitRequest( *i );
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

        eq::ServerDestroyConfigPacket destroyConfigPacket;
        destroyConfigPacket.configID = getID();
        netNode->send( destroyConfigPacket );

        eq::ClientExitPacket clientExitPacket;
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

        while( netNode->getState() == net::Node::STATE_CONNECTED && --nSleeps )
            base::sleep( 100 ); // ms

        if( netNode->getState() == net::Node::STATE_CONNECTED )
        {
            net::NodePtr localNode = getLocalNode();
            EQASSERT( localNode.isValid( ));

            EQWARN << "Forcefully disconnection exited render client node"
                   << std::endl;
            localNode->disconnect( netNode );
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
    eq::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID  = getID();
    createConfigPacket.appNodeID = _appNetNode->getNodeID();
    createConfigPacket.appNodeID.convertToNetwork();
    createConfigPacket.requestID = _requestHandler.registerRequest();

    const string&   name = getName();
    net::NodePtr netNode = node->getNode();
    netNode->send( createConfigPacket, name );

    return createConfigPacket.requestID;
}

void Config::_syncClock()
{
    eq::ConfigSyncClockPacket packet;
    packet.time = _clock.getTime64();

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

    _updateEyes();

    for( vector< Canvas* >::const_iterator i = _canvases.begin();
         i != _canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        canvas->init();
    }

    for( vector< Compound* >::const_iterator i = _compounds.begin();
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
        EQWARN << "Exiting non-initialized config" << endl;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_INITIALIZING );
    _state = STATE_EXITING;

    for( vector< Compound* >::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->exit();
    }

    for( vector< Canvas* >::const_iterator i = _canvases.begin();
         i != _canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        canvas->exit();
    }

    const bool success = _updateRunning();

    if( _headMatrix.getID() != EQ_ID_INVALID )
        deregisterObject( &_headMatrix );

    eq::ConfigEvent exitEvent;
    exitEvent.data.type = eq::Event::EXIT;
    send( _appNetNode, exitEvent );
    
    _state         = STATE_STOPPED;
    return success;
}

namespace
{
class UnmapVisitor : public ConfigVisitor
{
public:
    virtual ~UnmapVisitor(){}

    virtual VisitorResult visitPre( Canvas* canvas )
        {
            _unmap( canvas );
            return TRAVERSE_CONTINUE; 
        }
    virtual VisitorResult visit( Segment* segment )
        { 
            _unmap( segment );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPre( Layout* layout )
        { 
            _unmap( layout );
            return TRAVERSE_CONTINUE; 
        }
    virtual VisitorResult visit( View* view )
        { 
            _unmap( view );
            return TRAVERSE_CONTINUE; 
        }

private:
    void _unmap( net::Object* object )
        {
            EQASSERT( object->getID() != EQ_ID_INVALID );

            net::Session* session = object->getSession();
            EQASSERT( session );

            if( object->isMaster( ))
                session->deregisterObject( object );
            else
                session->unmapObject( object );
        }

};
}

void Config::unmap()
{
    eq::ConfigUnmapPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( _appNetNode, packet );

    if( _serializer ) // Config::init never happened
    {
        deregisterObject( _serializer );
        delete _serializer;
        _serializer = 0;
    }

    UnmapVisitor unmapper;
    accept( unmapper );

    _requestHandler.waitRequest( packet.requestID );
}

void Config::_updateEyes()
{
    const float eyeBase_2 = .5f * getFAttribute( FATTR_EYE_BASE );

    // eye_world = (+-eye_base/2., 0, 0 ) x head_matrix
    // OPT: don't use vector operator* due to possible simplification

    _eyes[eq::EYE_CYCLOP].x = _headMatrix.m03;
    _eyes[eq::EYE_CYCLOP].y = _headMatrix.m13;
    _eyes[eq::EYE_CYCLOP].z = _headMatrix.m23;
    _eyes[eq::EYE_CYCLOP]  /= _headMatrix.m33;

    _eyes[eq::EYE_LEFT].x = ( -eyeBase_2 * _headMatrix.m00 + _headMatrix.m03 );
    _eyes[eq::EYE_LEFT].y = ( -eyeBase_2 * _headMatrix.m10 + _headMatrix.m13 );
    _eyes[eq::EYE_LEFT].z = ( -eyeBase_2 * _headMatrix.m20 + _headMatrix.m23 );
    _eyes[eq::EYE_LEFT]  /= ( -eyeBase_2 * _headMatrix.m30 + _headMatrix.m33 );

    _eyes[eq::EYE_RIGHT].x = ( eyeBase_2 * _headMatrix.m00 + _headMatrix.m03 );
    _eyes[eq::EYE_RIGHT].y = ( eyeBase_2 * _headMatrix.m10 + _headMatrix.m13 );
    _eyes[eq::EYE_RIGHT].z = ( eyeBase_2 * _headMatrix.m20 + _headMatrix.m23 );
    _eyes[eq::EYE_RIGHT]  /= ( eyeBase_2 * _headMatrix.m30 + _headMatrix.m33 );

    EQVERB << "Eye position: " << _eyes[ eq:: EYE_CYCLOP ] << std::endl;
}

bool Config::_startFrame( const uint32_t frameID )
{
    if( !_updateRunning( ))
    {
        ++_currentFrame;
        return false;
    }

    ++_currentFrame;
    EQLOG( LOG_ANY ) << "----- Start Frame ----- " << _currentFrame << endl;

    if( _state == STATE_STOPPED )
        return true;
    EQASSERT( _state == STATE_RUNNING );

    if( !_appNode->isActive( )) // release appNode local sync
    {
        ConfigReleaseFrameLocalPacket packet;
        packet.frameNumber = _currentFrame;
        send( _appNetNode, packet );
    }

    for( vector< Compound* >::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->update( _currentFrame );
    }
    
    ConfigUpdateDataVisitor configDataVisitor;
    accept( configDataVisitor );

    for( vector< Node* >::const_iterator i = _nodes.begin(); 
         i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isActive( ))
            node->update( frameID, _currentFrame );
    }
    return true;
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
    eq::ConfigFrameFinishPacket packet;
    packet.frameNumber = frameNumber;
    packet.sessionID   = getID();

    // do not use send/_bufferedTasks, not thread-safe!
    _appNetNode->send( packet );
    EQLOG( eq::LOG_TASKS ) << "TASK config frame finished  " << &packet << endl;
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

    EQLOG( LOG_ANY ) << "--- Flush All Frames -- " << endl;
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

    const eq::ConfigInitPacket* packet =
        command.getPacket<eq::ConfigInitPacket>();
    EQVERB << "handle config start init " << packet << endl;

    eq::ConfigInitReplyPacket reply( packet );
    reply.result = _init( packet->initID );
    std::string error = _error;

    if( reply.result )
        mapObject( &_headMatrix, packet->headMatrixID );
    else
        exit();

    EQINFO << "Config init " << (reply.result ? "successful": "failed: ") 
           << error << endl;

    send( command.getNode(), reply, error );
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdExit( net::Command& command ) 
{
    const eq::ConfigExitPacket* packet = 
        command.getPacket<eq::ConfigExitPacket>();
    eq::ConfigExitReplyPacket   reply( packet );
    EQVERB << "handle config exit " << packet << endl;

    if( _state == STATE_RUNNING )
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

    if( packet->nChanges > 0 )
    {
        ConfigSyncVisitor syncer( packet->nChanges, packet->changes );
        EQCHECK( accept( syncer ) == TRAVERSE_TERMINATE );
    }

    if( !_startFrame( packet->frameID ))
    {
        EQWARN << "Start frame failed, exiting config: " << _error << std::endl;
        exit();
    }

    if( _state == STATE_STOPPED )
    {
        // unlock app
        eq::ConfigFrameFinishPacket frameFinishPacket;
        frameFinishPacket.frameNumber = _currentFrame;
        send( command.getNode(), frameFinishPacket );        
    }

    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdFinishAllFrames( net::Command& command ) 
{
    const eq::ConfigFinishAllFramesPacket* packet = 
        command.getPacket<eq::ConfigFinishAllFramesPacket>();
    EQVERB << "handle config all frames finish " << packet << endl;

    _flushAllFrames();
    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdCreateReply( net::Command& command ) 
{
    const eq::ConfigCreateReplyPacket* packet = 
        command.getPacket<eq::ConfigCreateReplyPacket>();

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
            const LoadBalancerVector& loadBalancers = 
                compound->getLoadBalancers();
            for( LoadBalancerVector::const_iterator i = loadBalancers.begin();
                 i != loadBalancers.end(); ++i )
            {
                (*i)->setFreeze( _freeze );
            }
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
    accept( visitor );

    return net::COMMAND_HANDLED;
}

net::CommandResult Config::_cmdUnmapReply( net::Command& command ) 
{
    const eq::ConfigUnmapReplyPacket* packet = 
        command.getPacket< eq::ConfigUnmapReplyPacket >();
    EQVERB << "Handle unmap reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID );
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
