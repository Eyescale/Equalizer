
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
        : _server( server )
{
    for( int i=0; i<eq::CMD_CONFIG_ALL; i++ )
        _cmdHandler[i] = &eqs::Config::_cmdUnknown;

    _cmdHandler[eq::CMD_CONFIG_INIT] = &eqs::Config::_cmdRequest;
    _cmdHandler[eq::REQ_CONFIG_INIT] = &eqs::Config::_cmdInit;
    _cmdHandler[eq::CMD_CONFIG_EXIT] = &eqs::Config::_cmdRequest;
    _cmdHandler[eq::REQ_CONFIG_EXIT] = &eqs::Config::_cmdExit;
}

void Config::addNode( Node* node )
{
    node->_config = this; 
    node->_id     = genIDs( 1 );
    _nodes[node->_id] = node; 
}

bool Config::removeNode( Node* node )
{
    NodeHash::iterator iter = _nodes.find( node->_id );
    if( iter == _nodes.end( ))
        return false;

    _nodes.erase( iter );

    node->_config = NULL; 
    freeIDs( node->_id, 1 );
    node->_id = 0;

    return true;
}

//===========================================================================
// command handling
//===========================================================================
void Config::handlePacket( eqNet::Node* node, const eq::ConfigPacket* packet )
{
    ASSERT( node );
    switch( packet->datatype )
    {
        case eq::DATATYPE_EQ_CONFIG:
            ASSERT( packet->command < eq::CMD_CONFIG_ALL );

            (this->*_cmdHandler[packet->command])(node, packet);
            break;

        case eq::DATATYPE_EQ_NODE:
        {
            const eq::NodePacket* nodePacket = (eq::NodePacket*)packet;
            const uint            nodeID     = nodePacket->nodeID;
            Node*                 node       = _nodes[nodeID];
            ASSERT( node );
            
            node->handlePacket( node, nodePacket );
            break;
        }

        default:
            ERROR << "unimplemented" << endl;
            break;
    }
}

// pushes the request to the main thread to be handled asynchronously
void Config::_cmdRequest( eqNet::Node* node, const eqNet::Packet* packet )
{
    _server->pushRequest( node, packet );
}

void Config::_cmdInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    const eq::ConfigInitPacket* packet = (eq::ConfigInitPacket*)pkg;
    eq::ConfigInitReplyPacket   reply( packet );
    INFO << "handle config init " << packet << endl;

    reply.result = _init();
    INFO << "config init result: " << reply.result << endl;
    node->send( reply );
}

void Config::_cmdExit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    const eq::ConfigExitPacket* packet = (eq::ConfigExitPacket*)pkg;
    eq::ConfigExitReplyPacket   reply( packet );
    INFO << "handle config exit " << packet << endl;

    reply.result = _exit();
    INFO << "config exit result: " << reply.result << endl;
    node->send( reply );
}

//===========================================================================
// operations
//===========================================================================

bool Config::_init()
{
    const uint nCompounds = this->nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound = getCompound( i );
        compound->init();
    }

    // connect (and launch) nodes
    vector<Node*> usedNodes;
    for( NodeHash::const_iterator iter = _nodes.begin(); 
         iter != _nodes.end(); ++iter )
    {
        Node* node = (*iter).second;
        if( node->isUsed( ))
            usedNodes.push_back( node );
    }

    const uint nNodes = usedNodes.size();
    for( uint i=0; i<nNodes; i++ )
    {
        if( !usedNodes[i]->initConnect( ))
        {
            ERROR << "Connection to " << usedNodes[i] << " failed." << endl;
            _exit();
            return false;
        }
    }

    for( uint i=0; i<nNodes; i++ )
    {
        Node* node = usedNodes[i];

        if( !node->syncConnect( ))
        {
            ERROR << "Connection of " << node << " failed." << endl;
            _exit();
            return false;
        }
        
        // initialize nodes
        node->sendInit();
    }

    for( uint i=0; i<nNodes; i++ )
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

    for( NodeHash::const_iterator iter = _nodes.begin(); 
         iter != _nodes.end(); ++iter )
    {
        Node* node = (*iter).second;
        if( !node->isUsed( ) || !node->isConnected( ))
            continue;
        
        connectedNodes.push_back( node );
    }
    
    const uint nNodes = connectedNodes.size();
    for( uint i=0; i<nNodes; i++ )
        connectedNodes[i]->sendExit();

    for( uint i=0; i<nNodes; i++ )
    {
        Node* node = connectedNodes[i];

        if( !node->syncExit( ))
        {
            ERROR << "Exit of " << node << " failed." << endl;
            cleanExit = false;
        }

        node->stop();
    }

    const uint nCompounds = this->nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound = getCompound( i );
        compound->exit();
    }

    return cleanExit;
}

std::ostream& eqs::operator << ( std::ostream& os, const Config* config )
{
    if( !config )
    {
        os << "NULL config";
        return os;
    }
    
    const NodeHash& nodes = config->getNodes();
    const uint nCompounds = config->nCompounds();
    os << "config " << (void*)config << " " << nodes.size() << " nodes "
           << nCompounds << " compounds";
    
    for( NodeHash::const_iterator iter = nodes.begin(); 
         iter != nodes.end(); ++iter )
        os << std::endl << "    " << (*iter).second;

    for( uint i=0; i<nCompounds; i++ )
        os << std::endl << "    " << config->getCompound(i);
    
    return os;
}
