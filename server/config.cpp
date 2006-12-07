
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "node.h"
#include "server.h"
#include "global.h"

#include <eq/net/command.h>
#include <eq/net/global.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_CONFIG_") + #attr )
std::string Config::_fAttributeStrings[FATTR_ALL] = {
    MAKE_ATTR_STRING( FATTR_EYE_BASE )
};

void Config::_construct()
{
    _latency     = 1;
    _frameNumber = 0;
    _state       = STATE_STOPPED;
    _appNode     = 0;

    registerCommand( eq::CMD_CONFIG_INIT,
                     eqNet::CommandFunc<Config>( this, &Config::_cmdPush ));
    registerCommand( eq::REQ_CONFIG_INIT,
                     eqNet::CommandFunc<Config>( this, &Config::_reqInit ));
    registerCommand( eq::CMD_CONFIG_EXIT, 
                     eqNet::CommandFunc<Config>( this, &Config::_cmdPush ));
    registerCommand( eq::REQ_CONFIG_EXIT, 
                     eqNet::CommandFunc<Config>( this, &Config::_reqExit ));
    registerCommand( eq::CMD_CONFIG_FRAME_BEGIN, 
                     eqNet::CommandFunc<Config>( this, &Config::_cmdPush ));
    registerCommand( eq::REQ_CONFIG_FRAME_BEGIN,
                     eqNet::CommandFunc<Config>( this, &Config::_reqBeginFrame));
    registerCommand( eq::CMD_CONFIG_FRAME_END, 
                     eqNet::CommandFunc<Config>( this, &Config::_cmdPush ));
    registerCommand( eq::REQ_CONFIG_FRAME_END, 
                     eqNet::CommandFunc<Config>( this, &Config::_reqEndFrame));

    const Global* global = Global::instance();
    
    for( int i=0; i<FATTR_ALL; ++i )
        _fAttributes[i] = global->getConfigFAttribute( (FAttribute)i );
        
    EQINFO << "New config @" << (void*)this << endl;
}

Config::Config()
{
    _construct();
}

Config::~Config()
{
    EQINFO << "Delete config @" << (void*)this << endl;
    _server     = 0;
    _appNode    = 0;

    for( vector<Node*>::const_iterator i = _nodes.begin(); i != _nodes.end();
         ++i )
    {
        Node* node = *i;
        EQASSERT( node->getRefCount( ) == 1 );

        node->_config = 0;
        node->unref(); // a.k.a delete
    }
    _nodes.clear();

    for( vector<Compound*>::const_iterator i = _compounds.begin(); 
         i != _compounds.end(); ++i )
    {
        Compound* compound = *i;

        compound->_config = 0;
        delete compound;
    }
    _compounds.clear();
}

struct ReplaceChannelData
{
    Channel* oldChannel;
    Channel* newChannel;
};
static TraverseResult replaceChannelCB(Compound* compound, void* userData )
{
    ReplaceChannelData* data = (ReplaceChannelData*)userData;
                            
    if( compound->getChannel() == data->oldChannel )
        compound->setChannel( data->newChannel );
    
    return TRAVERSE_CONTINUE;
}

Config::Config( const Config& from )
        : eqNet::Session( eq::CMD_CONFIG_CUSTOM ),
          _server( from._server )
{
    _construct();
    _appNetNode = from._appNetNode;
    _latency    = from._latency;

    const uint32_t nCompounds = from.nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound      = from.getCompound(i);
        Compound* compoundClone = new Compound( *compound );

        addCompound( compoundClone );
        // channel is replaced below
    }

    for( int i=0; i<FATTR_ALL; ++i )
        _fAttributes[i] = from.getFAttribute( (FAttribute)i );
        
    const uint32_t nNodes = from.nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        eqs::Node* node      = from.getNode(i);
        eqs::Node* nodeClone = new eqs::Node( *node );
        
        (node == from._appNode) ? 
            addApplicationNode( nodeClone ) : addNode( nodeClone );

        // replace channels in compounds
        const uint32_t nPipes = node->nPipes();
        for( uint32_t j=0; j<nPipes; j++ )
        {
            Pipe* pipe      = node->getPipe(j);
            Pipe* pipeClone = nodeClone->getPipe(j);
            
            const uint32_t nWindows = pipe->nWindows();
            for( uint32_t k=0; k<nWindows; k++ )
            {
                Window* window      = pipe->getWindow(k);
                Window* windowClone = pipeClone->getWindow(k);
            
                const uint32_t nChannels = window->nChannels();
                for( uint32_t l=0; l<nChannels; l++ )
                {
                    Channel* channel      = window->getChannel(l);
                    Channel* channelClone = windowClone->getChannel(l);

                    ReplaceChannelData data;
                    data.oldChannel = channel;
                    data.newChannel = channelClone;

                    for( uint32_t m=0; m<nCompounds; m++ )
                    {
                        Compound* compound = getCompound(m);

                        Compound::traverse( compound, replaceChannelCB, 
                                            replaceChannelCB, NULL, &data );
                    }
                }
            }
        }
    }
}

void Config::addNode( Node* node )
{
    _nodes.push_back( node ); 
    
    node->_config = this; 
}

bool Config::removeNode( Node* node )
{
    vector<Node*>::iterator iter = find( _nodes.begin(), _nodes.end(), node );
    if( iter == _nodes.end( ))
        return false;

    _nodes.erase( iter );
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

Channel* Config::findChannel( const std::string& name ) const
{
    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        eqs::Node*     node   = getNode(i);
        const uint32_t nPipes = node->nPipes();
        for( uint32_t j=0; j<nPipes; j++ )
        {
            Pipe*          pipe     = node->getPipe(j);
            const uint32_t nWindows = pipe->nWindows();
            for( uint32_t k=0; k<nWindows; k++ )
            {
                Window*        window    = pipe->getWindow(k);
                const uint32_t nChannels = window->nChannels();
                for( uint32_t l=0; l<nChannels; l++ )
                {
                    Channel* channel = window->getChannel(l);
                    if( channel && channel->getName() == name )
                        return channel;
                }
            }
        }
    }
    return NULL;
}

void Config::addApplicationNode( Node* node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERTINFO( !_appNode, "Only one application node per config possible" );

    _appNode = node;
    addNode( node );
}

void Config::setApplicationNode( eqBase::RefPtr<eqNet::Node> node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERT( !_appNetNode );

    _appNetNode = node;
}

eqNet::CommandResult Config::_reqInit( eqNet::Command& command )
{
    const eq::ConfigInitPacket* packet = 
        command.getPacket<eq::ConfigInitPacket>();
    eq::ConfigInitReplyPacket   reply( packet );
    EQINFO << "handle config init " << packet << endl;

    reply.result       = _init( packet->initID );
    reply.headMatrixID = reply.result ? _headMatrix->getID() : EQ_ID_INVALID;

    EQINFO << "Config init " << (reply.result ? "successful":"failed") << endl;
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqExit( eqNet::Command& command ) 
{
    const eq::ConfigExitPacket* packet = 
        command.getPacket<eq::ConfigExitPacket>();
    eq::ConfigExitReplyPacket   reply( packet );
    EQINFO << "handle config exit " << packet << endl;

    reply.result = exit();
    EQINFO << "config exit result: " << reply.result << endl;
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqBeginFrame( eqNet::Command& command ) 
{
    const eq::ConfigBeginFramePacket* packet = 
        command.getPacket<eq::ConfigBeginFramePacket>();
    eq::ConfigBeginFrameReplyPacket   reply( packet );
    EQVERB << "handle config frame begin " << packet << endl;

    vector<Node*> nodes;
    reply.frameNumber = _beginFrame( packet->frameID, nodes );
    reply.nNodeIDs    = nodes.size();
    
    command.getNode()->send( reply, nodes );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqEndFrame( eqNet::Command& command ) 
{
    const eq::ConfigEndFramePacket* packet = 
        command.getPacket<eq::ConfigEndFramePacket>();
    eq::ConfigEndFrameReplyPacket   reply( packet );
    EQVERB << "handle config frame end " << packet << endl;

    reply.result = _endFrame();
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}


//===========================================================================
// operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
bool Config::_init( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _frameNumber = 0;

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound = getCompound( i );
        compound->init();
    }

    if( !_connectNodes()  ||
        !_initNodes( initID ) ||
        !_initPipes( initID ) )
    {
        exit();
        return false;
    }

    _headMatrix = new eq::Matrix4f;
    registerObject( _headMatrix.get(), _appNetNode );

    _state = STATE_INITIALIZED;
    return true;
}

static RefPtr<eqNet::Node> _createNode( Node* node )
{
    RefPtr<eqNet::Node> netNode = new eqNet::Node;

    const uint32_t nConnectionDescriptions = node->nConnectionDescriptions();
    for( uint32_t i=0; i<nConnectionDescriptions; i++ )
    {
        eqNet::ConnectionDescription* desc = 
            node->getConnectionDescription(i).get();
        
        netNode->addConnectionDescription( 
            new eqNet::ConnectionDescription( *desc ));
    }

    return netNode;
}

bool Config::_connectNodes()
{
    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;

        RefPtr<eqNet::Node> netNode;

        if( node == _appNode )
            netNode = _appNetNode;
        else
        {
            netNode = _createNode( node );
            netNode->setAutoLaunch( true );
            netNode->setProgramName( _renderClient );
            netNode->setWorkDir( _workDir );
        }

        if( !netNode->initConnect( ))
        {
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

    eq::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID  = _id;
    createConfigPacket.appNodeID = _appNetNode->getNodeID();

    eq::ConfigCreateNodePacket createNodePacket;
    
    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        RefPtr<eqNet::Node> netNode = node->getNode();

        if( !netNode->syncConnect( ))
        {
            EQERROR << "Connection of node " << netNode->getNodeID() 
                    << " failed." << endl;
            success = false;
        }
        
        if( !success ) 
            continue;

        // initialize nodes
        netNode->send( createConfigPacket, name );

        registerObject( node, _server.get( ));
        createNodePacket.nodeID = node->getID();
        send( netNode, createNodePacket );

        node->startInit( initID );
    }

    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        if( !node->syncInit( ))
        {
            EQERROR << "Init of node " << (void*)node << " failed." 
                    << endl;
            success = false;
        }
    }
    return success;
}

bool Config::_initPipes( const uint32_t initID )
{
    // start pipe-window-channel init in parallel on all nodes
    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;

        // XXX move to node?
        eq::NodeCreatePipePacket createPipePacket;

        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( !pipe->isUsed( ))
                continue;
            
            registerObject( pipe, _server.get( ));
            createPipePacket.pipeID = pipe->getID();
            node->send( createPipePacket );
            pipe->startInit( initID ); // recurses down
        }
    }

    // sync init of all pipe-window-channel entities
    bool success = true;
    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( !pipe->isUsed( ))
                continue;

            if( !pipe->syncInit( ))
                success = false;
        }
    }
    return success;
}

bool Config::exit()
{
    if( _state != STATE_INITIALIZED )
        return false;
//     foreach compound
//         call exit compound cb's
//     foreach compound
//         sync exit

    bool cleanExit = _exitPipes();
    if( !cleanExit )
        EQERROR << "pipes exit failed" << endl;

    if( !_exitNodes( ))
    {
        EQERROR << "nodes exit failed" << endl;
        cleanExit = false;
    }

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound = getCompound( i );
        compound->exit();
    }

    deregisterObject( _headMatrix.get( ));
    EQASSERT( _headMatrix->getRefCount( ) == 1 );
    _headMatrix  = NULL;

    _frameNumber = 0;
    _state       = STATE_STOPPED;
    return cleanExit;
}

bool Config::_exitPipes()
{
    // start pipe-window-channel exit in parallel
    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;

        if( !node->isUsed( ))
            continue;
            
        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( pipe->getState() == Pipe::STATE_STOPPED )
                continue;

            pipe->startExit();
        }
    }

    // sync exit of all pipe-window-channel entities
    bool success = true;
    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        eq::NodeDestroyPipePacket destroyPipePacket;

        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( pipe->getState() != Pipe::STATE_STOPPING )
                continue;

            if( !pipe->syncExit( ))
            {
                EQWARN << "Could not exit cleanly: " << pipe << endl;
                success = false;
            }

            destroyPipePacket.pipeID = pipe->getID();
            node->send( destroyPipePacket );
            deregisterObject( pipe );
        }
    }

    return success;
}

bool Config::_exitNodes()
{
    vector<Node*> exitingNodes;
    for( NodeHashIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed() || 
            node->getNode()->getState() == eqNet::Node::STATE_STOPPED )
            continue;

        exitingNodes.push_back( node );
        node->startExit();
    }

    bool success = true;
    for( vector<Node*>::const_iterator i = exitingNodes.begin();
         i != exitingNodes.end(); ++i )
    {
        Node*               node    = *i;
        RefPtr<eqNet::Node> netNode = node->getNode();

        if( !node->syncExit( ))
        {
            EQERROR << "Could not exit cleanly: " << node << endl;
            success = false;
        }

        if( netNode->getState() != eqNet::Node::STATE_STOPPED &&
            node != _appNode )

            EQWARN << "Config::exit: Node should have exited by now" << endl;

        node->setNode( NULL );
        deregisterObject( node );
    }
    return success;
}

void Config::_updateHead()
{
    _headMatrix->sync();
    const float         eyeBase_2 = .5f * getFAttribute(Config::FATTR_EYE_BASE);
    const eq::Matrix4f* head      = _headMatrix.get();

    // eye_world = (+-eye_base/2., 0, 0 ) x head_matrix
    // Don't use vector operator due to possible simplification

    _eyePosition[EYE_INDEX_CYCLOP].x = head->m03;
    _eyePosition[EYE_INDEX_CYCLOP].y = head->m13;
    _eyePosition[EYE_INDEX_CYCLOP].z = head->m23;
    _eyePosition[EYE_INDEX_CYCLOP]  /= head->m33;

    _eyePosition[EYE_INDEX_LEFT].x = (-eyeBase_2 * head->m00 + head->m03);
    _eyePosition[EYE_INDEX_LEFT].y = (-eyeBase_2 * head->m10 + head->m13);
    _eyePosition[EYE_INDEX_LEFT].z = (-eyeBase_2 * head->m20 + head->m23);
    _eyePosition[EYE_INDEX_LEFT]  /= (-eyeBase_2 * head->m30 + head->m33); // w

    _eyePosition[EYE_INDEX_RIGHT].x = (eyeBase_2 * head->m00 + head->m03);
    _eyePosition[EYE_INDEX_RIGHT].y = (eyeBase_2 * head->m10 + head->m13);
    _eyePosition[EYE_INDEX_RIGHT].z = (eyeBase_2 * head->m20 + head->m23);
    _eyePosition[EYE_INDEX_RIGHT]  /= (eyeBase_2 * head->m30 + head->m33); // w
}

const vmml::Vector3f& Config::getEyePosition( const uint32_t eye )
{
    switch( eye )
    {
        case Compound::EYE_CYCLOP:
            return _eyePosition[EYE_INDEX_CYCLOP];
        case Compound::EYE_LEFT:
            return _eyePosition[EYE_INDEX_LEFT];
        case Compound::EYE_RIGHT:
            return _eyePosition[EYE_INDEX_RIGHT];
        default:
            EQERROR << "Unknown eye position" << endl;
            return _eyePosition[EYE_INDEX_CYCLOP];
    }
}

uint32_t Config::_beginFrame( const uint32_t frameID, vector<Node*>& nodes )
{
    EQASSERT( _state == STATE_INITIALIZED );
    ++_frameNumber;
    EQLOG( LOG_ANY ) << "----- Begin Frame ----- " << _frameNumber << endl;

    _updateHead();

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound = getCompound( i );
        compound->update();
    }

    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        Node* node = getNode( i );
        if( node->isUsed( ))
        {
            node->update( frameID );
            nodes.push_back( node );
        }
    }
    
    return _frameNumber;
}

uint32_t Config::_endFrame()
{
    if( _frameNumber <= _latency )
        return 0;

    const uint32_t finishFrame = _frameNumber - _latency;
    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        Node* node = getNode( i );
        if( node->isUsed( ))
            node->syncUpdate( finishFrame );
    }

    EQLOG( LOG_ANY ) << "------ End Frame ------ " << finishFrame << endl;
    return finishFrame;
}


ostream& eqs::operator << ( ostream& os, const Config* config )
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

    const uint32_t nNodes = config->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
        os << config->getNode(i);

    const uint32_t nCompounds = config->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
        os << config->getCompound(i);

    os << exdent << "}" << endl << enableHeader << enableFlush;

    return os;
}
