
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "config.h"

#include "compound.h"
#include "node.h"
#include "server.h"
#include "global.h"

#include <eq/net/command.h>
#include <eq/net/global.h>

using namespace eqBase;
using namespace std;
using eqNet::ConnectionDescriptionVector;
using eqNet::CommandFunc;

namespace eqs
{

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
        : eqNet::Session()
        , _server( from._server )
{
    _construct();
    _appNetNode = from._appNetNode;
    _latency    = from._latency;

    const CompoundVector& compounds = from.getCompounds();
    for( CompoundVector::const_iterator i = compounds.begin(); 
         i != compounds.end(); ++i )
    {
        const Compound* compound      = *i;
        Compound*       compoundClone = new Compound( *compound );

        addCompound( compoundClone );
        // channel is replaced in channel copy constructor
    }

    for( int i=0; i<FATTR_ALL; ++i )
        _fAttributes[i] = from.getFAttribute( (FAttribute)i );
        
    const NodeVector& nodes = from.getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Node* node      = *i;
        Node*       nodeClone = new eqs::Node( *node, _compounds );
        
        (node == from._appNode) ? 
            addApplicationNode( nodeClone ) : addNode( nodeClone );
    }
}

Config::~Config()
{
    EQINFO << "Delete config @" << (void*)this << endl;
    _server     = 0;
    _appNode    = 0;

    for( vector<Compound*>::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;

        compound->_config = 0;
        delete compound;
    }
    _compounds.clear();

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;

        node->_config = 0;
        delete node;
    }
    _nodes.clear();
}

void Config::setLocalNode( eqBase::RefPtr< eqNet::Node > node )
{
    eqNet::Session::setLocalNode( node );
    
    if( !node ) 
        return;

    eqNet::CommandQueue* serverQueue  = getServerThreadQueue();
    eqNet::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( eq::CMD_CONFIG_START_INIT,
                     CommandFunc<Config>( this, &Config::_cmdStartInit),
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_FINISH_INIT,
                     CommandFunc<Config>(this, &Config::_cmdFinishInit),
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_EXIT,
                     CommandFunc<Config>( this, &Config::_cmdExit ),
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_CREATE_REPLY,
                     CommandFunc<Config>( this, &Config::_cmdCreateReply ),
                     commandQueue );
    registerCommand( eq::CMD_CONFIG_CREATE_NODE_REPLY,
                     CommandFunc<Config>( this, &Config::_cmdCreateNodeReply ),
                     commandQueue );
    registerCommand( eq::CMD_CONFIG_START_FRAME, 
                     CommandFunc<Config>( this, &Config::_cmdStartFrame ),
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_FINISH_FRAME, 
                     CommandFunc<Config>( this, &Config::_cmdFinishFrame ),
                     serverQueue );
    registerCommand( eq::CMD_CONFIG_FINISH_ALL_FRAMES, 
                     CommandFunc<Config>( this, &Config::_cmdFinishAllFrames ),
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
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        const Node*       node  = *i;
        const PipeVector& pipes = node->getPipes();
        for( PipeVector::const_iterator j = pipes.begin();
             j != pipes.end(); ++j )
        {
            const Pipe*         pipe    = *j;
            const WindowVector& windows = pipe->getWindows();
            for( WindowVector::const_iterator k = windows.begin(); 
                 k != windows.end(); ++k )
            {
                const Window*        window   = *k;
                const ChannelVector& channels = window->getChannels();
                for( ChannelVector::const_iterator l = channels.begin();
                     l != channels.end(); ++l )
                {
                    const Channel* channel = *l;
                    if( channel->getName() == name )
                        return *l;
                }
            }
        }
    }
    return 0;
}

void Config::addApplicationNode( Node* node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERTINFO( !_appNode, "Only one application node per config possible" );

    _appNode = node;
    addNode( node );
}

void Config::setApplicationNetNode( eqBase::RefPtr<eqNet::Node> node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERT( !_appNetNode );

    _appNetNode = node;
}

//===========================================================================
// operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
bool Config::_startInit( const uint32_t initID, 
                         vector< eqNet::NodeID::Data >& nodeIDs )
{
    EQASSERT( _state == STATE_STOPPED );
    _currentFrame  = 0;
    _finishedFrame = 0;

    for( vector< Compound* >::const_iterator i = _compounds.begin();
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;
        compound->init();
    }

    if( !_connectNodes() || !_initNodes( initID, nodeIDs ))
    {
        exit();
        return false;
    }

    _state = STATE_INITIALIZED;
    return true;
}

static RefPtr<eqNet::Node> _createNode( Node* node )
{
    RefPtr<eqNet::Node> netNode = new eqNet::Node;

    const ConnectionDescriptionVector& descriptions = 
        node->getConnectionDescriptions();
    for( ConnectionDescriptionVector::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        const eqNet::ConnectionDescription* desc = (*i).get();
        
        netNode->addConnectionDescription( 
            new eqNet::ConnectionDescription( *desc ));
    }

    netNode->setAutoLaunch( true );
    return netNode;
}

bool Config::_connectNodes()
{
    RefPtr< eqNet::Node > localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( !node->isUsed( ))
            continue;

        RefPtr<eqNet::Node> netNode;

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

bool Config::_initNodes( const uint32_t initID,
                         vector< eqNet::NodeID::Data >& nodeIDs )
{
    const string& name    = getName();
    bool          success = true;

    RefPtr< eqNet::Node > localNode = getLocalNode();
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
        
        RefPtr<eqNet::Node> netNode = node->getNode();

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

        RefPtr<eqNet::Node> netNode = node->getNode();

        createNodePacket.nodeID = node->getID();
        createNodePacket.requestID = _requestHandler.registerRequest();
        createNodeRequests.push_back( createNodePacket.requestID );
        send( netNode, createNodePacket );

        // start node-pipe-window-channel init in parallel on all nodes
        node->startConfigInit( initID );

        nodeIDs.resize( nodeIDs.size() + 1 );
        netNode->getNodeID().getData( nodeIDs.back( ));
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
        RefPtr<eqNet::Node> netNode = node->getNode();
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
        RefPtr<eqNet::Node> netNode = node->getNode();
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
    NodeVector exitingNodes;
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node*               node = *i;
        RefPtr<eqNet::Node> netNode = node->getNode();

        if( !node->isUsed() || !netNode.isValid() ||
            netNode->getState() == eqNet::Node::STATE_STOPPED )

            continue;

        exitingNodes.push_back( node );
        node->startConfigExit();
    }

    eq::ServerDestroyConfigPacket destroyConfigPacket;
    destroyConfigPacket.configID  = getID();

    eq::ConfigDestroyNodePacket destroyNodePacket;
    eq::ClientExitPacket        clientExitPacket;
    RefPtr< eqNet::Node >       localNode         = getLocalNode();
    EQASSERT( localNode.isValid( ));

    bool success = true;
    for( NodeVector::const_iterator i = exitingNodes.begin();
         i != exitingNodes.end(); ++i )
    {
        Node*               node    = *i;
        RefPtr<eqNet::Node> netNode = node->getNode();

        if( !node->syncConfigExit( ))
        {
            EQERROR << "Could not exit cleanly: " << node << endl;
            success = false;
        }
        
        destroyNodePacket.nodeID = node->getID();
        send( netNode, destroyNodePacket );
        node->setNode( 0 );

        if( node != _appNode )
        {
            netNode->send( destroyConfigPacket );
            netNode->send( clientExitPacket );
            // connection will be closed by Client::_reqExit command handler.
            // Ref count should be one, but often commands still hold a
            // reference. 
            //EQASSERTINFO( netNode->getRefCount() == 1, netNode->getRefCount( ));
        }

        deregisterObject( node );
    }
    return success;
}

void Config::_updateHead()
{
    _headMatrix.sync();

    const float         eyeBase_2 = .5f * getFAttribute(Config::FATTR_EYE_BASE);
    const eq::Matrix4f& head      = _headMatrix;

    // eye_world = (+-eye_base/2., 0, 0 ) x head_matrix
    // OPT: don't use vector operator* due to possible simplification

    _eyePosition[eq::EYE_CYCLOP].x = head.m03;
    _eyePosition[eq::EYE_CYCLOP].y = head.m13;
    _eyePosition[eq::EYE_CYCLOP].z = head.m23;
    _eyePosition[eq::EYE_CYCLOP]  /= head.m33;

    _eyePosition[eq::EYE_LEFT].x = ( -eyeBase_2 * head.m00 + head.m03 );
    _eyePosition[eq::EYE_LEFT].y = ( -eyeBase_2 * head.m10 + head.m13 );
    _eyePosition[eq::EYE_LEFT].z = ( -eyeBase_2 * head.m20 + head.m23 );
    _eyePosition[eq::EYE_LEFT]  /= ( -eyeBase_2 * head.m30 + head.m33 ); // w

    _eyePosition[eq::EYE_RIGHT].x = ( eyeBase_2 * head.m00 + head.m03 );
    _eyePosition[eq::EYE_RIGHT].y = ( eyeBase_2 * head.m10 + head.m13 );
    _eyePosition[eq::EYE_RIGHT].z = ( eyeBase_2 * head.m20 + head.m23 );
    _eyePosition[eq::EYE_RIGHT]  /= ( eyeBase_2 * head.m30 + head.m33 ); // w
}

void Config::_prepareFrame( vector< eqNet::NodeID >& nodeIDs )
{
    // Note: If you add sending commands to the app node, please consider that
    // the start frame reply sent directly after this function is a priority
    // packet!

    EQASSERT( _state == STATE_INITIALIZED );
    ++_currentFrame;
    EQLOG( LOG_ANY ) << "----- Start Frame ----- " << _currentFrame << endl;

    _updateHead();

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isUsed( ))
        {
            RefPtr< eqNet::Node > netNode = node->getNode();
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

    for( vector< Node* >::const_iterator i = _nodes.begin(); 
         i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isUsed( ))
            node->update( frameID, _currentFrame );
    }
}

uint32_t Config::_finishFrame()
{
    if( _currentFrame <= _latency ) // nothing to finish yet, says latency
        return 0;

    const uint32_t finishFrame = _currentFrame - _latency;
    if( _finishedFrame == finishFrame ) // already called?
        return finishFrame;

    _finishFrame( finishFrame );
    return finishFrame;
}

void Config::_finishFrame( const uint32_t frame )
{
    EQASSERT( _finishedFrame+1 == frame ); // otherwise app skipped finishFrame

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isUsed( ))
            node->finishFrame( frame );
    }

    _finishedFrame = frame;
    EQLOG( LOG_ANY ) << "----- Finish Frame ---- " << frame << endl;
}

uint32_t Config::_finishAllFrames()
{
    if( _currentFrame == 0 )
        return 0;

    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i )
    {
        Node* node = *i;
        if( node->isUsed( ))
            node->flushFrames( _currentFrame );
    }

    while( _finishedFrame < _currentFrame )
        _finishFrame( _finishedFrame + 1 );

    EQLOG( LOG_ANY ) << "-- Finish All Frames -- " << endl;
    return _currentFrame;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Config::_cmdStartInit( eqNet::Command& command )
{
    const eq::ConfigStartInitPacket* packet = 
        command.getPacket<eq::ConfigStartInitPacket>();
    eq::ConfigStartInitReplyPacket   reply( packet );
    EQINFO << "handle config start init " << packet << endl;

    _error.clear();
    vector< eqNet::NodeID::Data > nodeIDs;
    reply.result   = _startInit( packet->initID, nodeIDs );
    reply.latency  = _latency;

    EQINFO << "Config start init " << (reply.result ? "successful": "failed: ") 
           << _error << endl;

    send( command.getNode(), reply, _error );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishInit( eqNet::Command& command )
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
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdExit( eqNet::Command& command ) 
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
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdStartFrame( eqNet::Command& command ) 
{
    const eq::ConfigStartFramePacket* packet = 
        command.getPacket<eq::ConfigStartFramePacket>();
    eq::ConfigStartFrameReplyPacket   reply( packet );
    EQVERB << "handle config frame start " << packet << endl;

    vector< eqNet::NodeID > nodeIDs;
    _prepareFrame( nodeIDs );

    reply.frameNumber = _currentFrame;
    reply.nNodeIDs    = nodeIDs.size();

    for( vector< eqNet::NodeID >::iterator i = nodeIDs.begin(); 
         i != nodeIDs.end(); ++i )

         (*i).convertToNetwork();

    command.getNode()->send( reply, nodeIDs );

    _startFrame( packet->frameID );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishFrame( eqNet::Command& command ) 
{
    const eq::ConfigFinishFramePacket* packet = 
        command.getPacket<eq::ConfigFinishFramePacket>();
    EQVERB << "handle config frame finish " << packet << endl;

    eq::ConfigFinishFrameReplyPacket reply;
    reply.frameNumber = _finishFrame();
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishAllFrames( eqNet::Command& command ) 
{
    const eq::ConfigFinishAllFramesPacket* packet = 
        command.getPacket<eq::ConfigFinishAllFramesPacket>();
    EQVERB << "handle config all frames finish " << packet << endl;

    eq::ConfigFinishAllFramesReplyPacket reply;
    reply.frameNumber = _finishAllFrames();
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdCreateReply( eqNet::Command& command ) 
{
    const eq::ConfigCreateReplyPacket* packet = 
        command.getPacket<eq::ConfigCreateReplyPacket>();

    _requestHandler.serveRequest( packet->requestID );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdCreateNodeReply( eqNet::Command& command ) 
{
    const eq::ConfigCreateNodeReplyPacket* packet = 
        command.getPacket<eq::ConfigCreateNodeReplyPacket>();

    _requestHandler.serveRequest( packet->requestID );
    return eqNet::COMMAND_HANDLED;
}


ostream& operator << ( ostream& os, const Config* config )
{
    if( !config )
        return os;

    os << disableFlush << disableHeader << "config " << endl;
    os << "{" << endl << indent;

    if( config->getLatency() != 1 )
        os << "latency " << config->getLatency() << endl;
    os << endl;

    const float value = config->getFAttribute( Config::FATTR_EYE_BASE );
    if( value != 
        Global::instance()->getConfigFAttribute( Config::FATTR_EYE_BASE ))
    {
        os << "attributes" << endl << "{" << endl << indent;
        os << "eyeBase " << value << endl;
        os << exdent << "}" << endl;
    }

    const NodeVector& nodes = config->getNodes();
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
        os << *i;

    const CompoundVector& compounds = config->getCompounds();
    for( CompoundVector::const_iterator i = compounds.begin(); 
         i != compounds.end(); ++i )

        os << *i;

    os << exdent << "}" << endl << enableHeader << enableFlush;

    return os;
}
}
