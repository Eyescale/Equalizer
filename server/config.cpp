
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "node.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Config::_construct()
{
    _latency     = 1;
    _frameNumber = 0;
    _state       = STATE_STOPPED;
    _appNode     = NULL;

    registerCommand( eq::CMD_CONFIG_INIT, this, reinterpret_cast<CommandFcn>(
                         &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_INIT, this, reinterpret_cast<CommandFcn>(
                         &eqs::Config::_reqInit ));
    registerCommand( eq::CMD_CONFIG_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eqs::Config::_reqExit ));
    registerCommand( eq::CMD_CONFIG_FRAME_BEGIN, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_FRAME_BEGIN, this,
                     reinterpret_cast<CommandFcn>(
                         &eqs::Config::_reqBeginFrame ));
    registerCommand( eq::CMD_CONFIG_FRAME_END, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_FRAME_END, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_reqEndFrame));

    EQINFO << "New config @" << (void*)this << endl;
}

Config::Config()
        : eqNet::Session( eq::CMD_CONFIG_CUSTOM )
{
    _construct();
}

Config::~Config()
{
    _server     = NULL;
    _appNode    = NULL;
    _compounds.clear();
    _nodes.clear();
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

    const uint32_t nCompounds = from.nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound      = from.getCompound(i);
        Compound* compoundClone = new Compound( *compound );

        addCompound( compoundClone );
        // channel is replaced below
    }

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
    node->adjustLatency( _latency );
}

bool Config::removeNode( Node* node )
{
    vector<Node*>::iterator iter = find( _nodes.begin(), _nodes.end(), node );
    if( iter == _nodes.end( ))
        return false;

    _nodes.erase( iter );

    node->adjustLatency( -_latency );
    node->_config = NULL; 

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
                    if( channel->getName() == name )
                        return channel;
                }
            }
        }
    }
    return NULL;
}

void Config::setLatency( const uint32_t latency )
{
    if( _latency == latency )
        return;

    const int delta = latency - _latency;
    _latency = latency;

    for( vector<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end();
         ++iter )

        (*iter)->adjustLatency( delta );
}

void Config::addApplicationNode( Node* node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERT( !_appNode );

    _appNode = node;
    addNode( node );
}

void Config::setApplicationNode( eqBase::RefPtr<eqNet::Node> node )
{
    EQASSERT( _state == STATE_STOPPED );
    EQASSERT( !_appNetNode );

    _appNetNode = node;
}

// pushes the request to the main thread to be handled asynchronously
eqNet::CommandResult Config::_cmdRequest( eqNet::Node* node,
                                          const eqNet::Packet* packet )
{
    _server->pushRequest( node, packet );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqInit( eqNet::Node* node,
                                       const eqNet::Packet* pkg )
{
    const eq::ConfigInitPacket* packet = (eq::ConfigInitPacket*)pkg;
    eq::ConfigInitReplyPacket   reply( packet );
    EQINFO << "handle config init " << packet << endl;

    reply.result = _init( packet->initID );
    EQINFO << "config init " << (reply.result ? "successfulq":"failed") << endl;
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqExit( eqNet::Node* node, 
                                       const eqNet::Packet* pkg )
{
    const eq::ConfigExitPacket* packet = (eq::ConfigExitPacket*)pkg;
    eq::ConfigExitReplyPacket   reply( packet );
    EQINFO << "handle config exit " << packet << endl;

    reply.result = _exit();
    EQINFO << "config exit result: " << reply.result << endl;
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqBeginFrame( eqNet::Node* node, 
                                             const eqNet::Packet* pkg )
{
    const eq::ConfigBeginFramePacket* packet = (eq::ConfigBeginFramePacket*)pkg;
    eq::ConfigBeginFrameReplyPacket   reply( packet );
    EQVERB << "handle config frame begin " << packet << endl;

    vector<Node*> nodes;
    reply.frameNumber = _beginFrame( packet->frameID, nodes );
    reply.nNodeIDs    = nodes.size();
    
    node->send( reply, nodes );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqEndFrame( eqNet::Node* node, 
                                           const eqNet::Packet* pkg )
{
    const eq::ConfigEndFramePacket* packet = (eq::ConfigEndFramePacket*)pkg;
    eq::ConfigEndFrameReplyPacket   reply( packet );
    EQVERB << "handle config frame end " << packet << endl;

    reply.result = _endFrame();
    send( node, reply );
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
        _exit();
        return false;
    }

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
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
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
    
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
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

    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
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
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
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
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
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

bool Config::_exit()
{
    if( _state != STATE_INITIALIZED )
        return false;
//     foreach compound
//         call exit compound cb's
//     foreach compound
//         sync exit

    const bool cleanExit = ( _exitPipes() &&
                             _exitNodes()  );

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound = getCompound( i );
        compound->exit();
    }

    _frameNumber = 0;
    _state       = STATE_STOPPED;
    return cleanExit;
}

bool Config::_exitPipes()
{
    // start pipe-window-channel exit in parallel
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
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
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
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
                success = false;

            destroyPipePacket.pipeID = pipe->getID();
            node->send( destroyPipePacket );
            deregisterObject( pipe );
        }
    }

    return success;
}

bool Config::_exitNodes()
{
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed() || 
            node->getNode()->getState() == eqNet::Node::STATE_STOPPED )
            continue;

        node->startExit();
    }

    eq::ConfigDestroyNodePacket destroyNodePacket;
    bool success = true;

    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node*               node    = *iter;
        RefPtr<eqNet::Node> netNode = node->getNode();
        if( !node->isUsed() || 
            netNode->getState() == eqNet::Node::STATE_STOPPED )
            continue;

        if( !node->syncExit( ))
        {
            EQERROR << "Exit of " << node << " failed." << endl;
            success = false;
        }

        destroyNodePacket.nodeID = node->getID();
        send( netNode, destroyNodePacket );
        node->setNode( NULL );
        deregisterObject( node );
    }
    return success;
}


uint32_t Config::_beginFrame( const uint32_t frameID, vector<Node*>& nodes )
{
    EQASSERT( _state == STATE_INITIALIZED );
    ++_frameNumber;
    
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
    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        Node* node = getNode( i );
        if( node->isUsed( ))
            node->syncUpdate();
    }

    return (_frameNumber > _latency ? _frameNumber-_latency : 0);
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

    const uint32_t nNodes = config->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
        os << config->getNode(i);

    const uint32_t nCompounds = config->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
        os << config->getCompound(i);
    
    os << exdent << "}" << endl << enableHeader << enableFlush;

    return os;
}
