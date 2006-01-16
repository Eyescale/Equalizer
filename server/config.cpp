
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "node.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eqs;
using namespace std;

Config::Config( Server* server )
        : eqNet::Session( eq::CMD_CONFIG_ALL ),
          _server( server ),
          _frameNumber(0)
{
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
                     reinterpret_cast<CommandFcn>(
                         &eqs::Config::_reqFrameEnd ));
}

void Config::addNode( Node* node )
{
    node->_config = this; 
    registerObject( node );
    _nodes.push_back( node ); 
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
    node->_config = NULL; 

    return true;
}

// pushes the request to the main thread to be handled asynchronously
void Config::_cmdRequest( eqNet::Node* node, const eqNet::Packet* packet )
{
    _server->pushRequest( node, packet );
}

void Config::_reqInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    const eq::ConfigInitPacket* packet = (eq::ConfigInitPacket*)pkg;
    eq::ConfigInitReplyPacket   reply( packet );
    INFO << "handle config init " << packet << endl;

    reply.result = _init();
    INFO << "config init result: " << reply.result << endl;
    node->send( reply );
}

void Config::_reqExit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    const eq::ConfigExitPacket* packet = (eq::ConfigExitPacket*)pkg;
    eq::ConfigExitReplyPacket   reply( packet );
    INFO << "handle config exit " << packet << endl;

    reply.result = _exit();
    INFO << "config exit result: " << reply.result << endl;
    node->send( reply );
}

void Config::_reqFrameBegin( eqNet::Node* node, const eqNet::Packet* pkg )
{
    const eq::ConfigFrameBeginPacket* packet = (eq::ConfigFrameBeginPacket*)pkg;
    eq::ConfigFrameBeginReplyPacket   reply( packet );
    INFO << "handle config frame begin " << packet << endl;

    reply.result = _frameBegin();
    node->send( reply );
}

void Config::_reqFrameEnd( eqNet::Node* node, const eqNet::Packet* pkg )
{
    const eq::ConfigFrameEndPacket* packet = (eq::ConfigFrameEndPacket*)pkg;
    eq::ConfigFrameEndReplyPacket   reply( packet );
    INFO << "handle config frame end " << packet << endl;

    reply.result = _frameEnd();
    node->send( reply );
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
            ERROR << "Connection to " << usedNodes[i] << " failed." << endl;
            _exit();
            return false;
        }
    }

    const string&              name = getName();
    eq::NodeCreateConfigPacket createConfigPacket;
    createConfigPacket.configID     = _id;
    createConfigPacket.nameLength   = name.size()+1;

    for( uint32_t i=0; i<nNodes; i++ )
    {
        Node* node = usedNodes[i];
        
        if( !node->syncConnect( ))
        {
            ERROR << "Connection of " << node << " failed." << endl;
            _exit();
            return false;
        }
        
        // initialize nodes
        node->send( createConfigPacket );
        node->send( name.c_str(), createConfigPacket.nameLength );
        node->startInit();
    }

    for( uint32_t i=0; i<nNodes; i++ )
    {
        if( !usedNodes[i]->syncInit( ))
        {
            ERROR << "Init of " << usedNodes[i] << " failed." << endl;
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
            ERROR << "Exit of " << node << " failed." << endl;
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
    // TODO
    return 0;
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
