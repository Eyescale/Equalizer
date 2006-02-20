
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "node.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eqs;
using namespace std;

void Config::_construct()
{
    _latency     = 1;
    _frameNumber = 0;

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
                         &eqs::Config::_reqFrameBegin ));
    registerCommand( eq::CMD_CONFIG_FRAME_END, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_FRAME_END, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_reqFrameEnd));
}

Config::Config( Server* server )
        : eqNet::Session( eq::CMD_CONFIG_ALL ),
          _server( server )
{
    _construct();
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
        : eqNet::Session( eq::CMD_CONFIG_ALL ),
          _server( from._server )
{
    _construct();

    _server->mapConfig( this );

    const uint32_t nCompounds = from.nCompounds();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        Compound* compound      = from.getCompound(i);
        Compound* compoundClone = new Compound( *compound );

        addCompound( compoundClone );
        // channel is replaced below
    }

    const uint32_t nNodes = from.nNodes();
    for( uint32_t i=0; i<nNodes; i++ )
    {
        eqs::Node* node      = from.getNode(i);
        eqs::Node* nodeClone = new eqs::Node( *node );
        
        addNode( nodeClone );

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

    registerObject( node );

    const int nPipes = node->nPipes();
    for( int k = 0; k<nPipes; ++k )
    {
        Pipe* pipe = node->getPipe( k );
        registerObject( pipe );

        const int nWindows = pipe->nWindows();
        for( int j = 0; j<nWindows; ++j )
        {
            Window* window = pipe->getWindow( j );
            registerObject( window );
            
            const int nChannels = window->nChannels();
            for( int i = 0; i<nChannels; ++i )
            {
                Channel* channel = window->getChannel( i );
                registerObject( channel );
            }
        }
    }
}

bool Config::removeNode( Node* node )
{
    vector<Node*>::iterator iter = _nodes.begin();
    for( ; iter != _nodes.end(); ++iter )
        if( (*iter) == node )
            break;

    if( iter == _nodes.end( ))
        return false;

    _nodes.erase( iter );
    deregisterObject( node );

    node->adjustLatency( -_latency );
    node->_config = NULL; 

    return true;
}

void Config::setLatency( const uint32_t latency )
{
    if( _latency == latency )
        return;

    const int delta = latency - _latency;
    for( vector<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end();
         ++iter )

        (*iter)->adjustLatency( delta );
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

    reply.result = _init();
    EQINFO << "config init result: " << reply.result << endl;
    node->send( reply );
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
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqFrameBegin( eqNet::Node* node, 
                                             const eqNet::Packet* pkg )
{
    const eq::ConfigFrameBeginPacket* packet = (eq::ConfigFrameBeginPacket*)pkg;
    eq::ConfigFrameBeginReplyPacket   reply( packet );
    EQVERB << "handle config frame begin " << packet << endl;

    reply.result = _frameBegin();
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqFrameEnd( eqNet::Node* node, 
                                           const eqNet::Packet* pkg )
{
    const eq::ConfigFrameEndPacket* packet = (eq::ConfigFrameEndPacket*)pkg;
    eq::ConfigFrameEndReplyPacket   reply( packet );
    EQVERB << "handle config frame end " << packet << endl;

    reply.result = _frameEnd();
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}


//===========================================================================
// operations
//===========================================================================

bool Config::_init()
{
    _frameNumber = 0;

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        Compound* compound = getCompound( i );
        compound->init();
    }

    // connect (and launch) nodes
    vector<Node*>                 usedNodes;
    uint32_t nNodes = _nodes.size();
    for( uint32_t i=0; i<nNodes; i++ )
    {
        if( _nodes[i]->isUsed( ))
            usedNodes.push_back( _nodes[i] );
    }

    nNodes = usedNodes.size();
    if( nNodes == 0 )
        return true;

    for( uint32_t i=0; i<nNodes; i++ )
    {
        if( !usedNodes[i]->initConnect( ))
        {
            EQERROR << "Connection to " << usedNodes[i] << " failed." << endl;
            _exit();
            return false;
        }
    }

    const string&              name = getName();
    eq::NodeCreateConfigPacket createConfigPacket;
    createConfigPacket.configID     = _id;

    for( uint32_t i=0; i<nNodes; i++ )
    {
        Node* node = usedNodes[i];
        
        if( !node->syncConnect( ))
        {
            EQERROR << "Connection of " << node << " failed." << endl;
            _exit();
            return false;
        }
        
        // initialize nodes
        node->send( createConfigPacket, name );
        node->startInit();
    }

    for( uint32_t i=0; i<nNodes; i++ )
    {
        if( !usedNodes[i]->syncInit( ))
        {
            EQERROR << "Init of " << usedNodes[i] << " failed." << endl;
            _exit();
            return false;
        }
    }

    return true;
}

bool Config::_exit()
{
//     foreach compound
//         call exit compound cb's
//     foreach compound
//         sync exit

    bool          cleanExit = true;
    vector<Node*> connectedNodes;

    uint32_t nNodes = _nodes.size();
    for( uint32_t i=0; i<nNodes; i++ )
    {
        Node* node = _nodes[i];
        if( !node->isUsed( ) || !node->isConnected( ))
            continue;
        
        connectedNodes.push_back( node );
    }
    
    nNodes = connectedNodes.size();
    for( uint32_t i=0; i<nNodes; i++ )
        connectedNodes[i]->startExit();

    for( uint32_t i=0; i<nNodes; i++ )
    {
        Node* node = connectedNodes[i];

        if( !node->syncExit( ))
        {
            EQERROR << "Exit of " << node << " failed." << endl;
            cleanExit = false;
        }

        node->stop();
    }

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        Compound* compound = getCompound( i );
        compound->exit();
    }

    _frameNumber = 0;
    return cleanExit;
}

uint32_t Config::_frameBegin()
{
    ++_frameNumber;

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        Compound* compound = getCompound( i );
        compound->update();
    }

    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; i++ )
    {
        Node* node = getNode( i );
        if( node->isUsed( ))
            node->update();
    }
    
    return _frameNumber;
}

uint32_t Config::_frameEnd()
{
    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; i++ )
    {
        Node* node = getNode( i );
        if( node->isUsed( ))
            node->syncUpdate();
    }

    return (_frameNumber > _latency ? _frameNumber-_latency : 0);
}


std::ostream& eqs::operator << ( std::ostream& os, const Config* config )
{
    if( !config )
    {
        os << "NULL config";
        return os;
    }
    
    const uint32_t nNodes = config->nNodes();
    const uint32_t nCompounds = config->nCompounds();
    os << "config " << (void*)config << " " << nNodes << " nodes "
           << nCompounds << " compounds";
    
    for( uint32_t i=0; i<nNodes; i++ )
        os << std::endl << "    " << config->getNode(i);
    for( uint32_t i=0; i<nCompounds; i++ )
        os << std::endl << "    " << config->getCompound(i);
    
    return os;
}
