
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "node.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eqs;
using namespace std;

Config::Config()
{
    for( int i=0; i<eq::CMD_CONFIG_ALL; i++ )
        _cmdHandler[i] = &eqs::Config::_cmdUnknown;

    _cmdHandler[eq::CMD_CONFIG_INIT] = &eqs::Config::_cmdRequest;
    _cmdHandler[eq::REQ_CONFIG_INIT] = &eqs::Config::_cmdInit;
}

void Config::map( Server* server, const uint id, const std::string& name,
                  const bool isMaster )
{
    _server = server;
    eqNet::Session::map( server, id, name, isMaster);
}

void Config::addNode( Node* node )
{
    _nodes.push_back( node ); 
    node->_config = this; 
    node->_id     = genIDs( 1 );
}

bool Config::removeNode( Node* node )
{
    for( vector<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end( );
         ++iter )
    {
        if( *iter == node )
        {
            _nodes.erase( iter );

            node->_config = NULL; 
            freeIDs( node->_id, 1 );
            node->_id = 0;

            return true;
        }
    }
    return false;
}

//===========================================================================
// command handling
//===========================================================================
void Config::handlePacket( eqNet::Node* node, const eq::ConfigPacket* packet )
{
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
    const uint nNodes = this->nNodes();
    for( uint i=0; i<nNodes; i++ )
    {
        Node* node = getNode( i );
        if( !node->isUsed( ))
            continue;

        if( !node->initConnect( ))
        {
            ERROR << "Connection to " << node << " failed." << endl;
            _exit();
            return false;
        }
    }

    for( uint i=0; i<nNodes; i++ )
    {
        Node* node = getNode( i );
        if( !node->isUsed( ))
            continue;

        if( !node->syncConnect( ))
        {
            ERROR << "Connection of " << node << " failed." << endl;
            _exit();
            return false;
        }

        node->sendInit();
    }

    for( uint i=0; i<nNodes; i++ )
    {
        Node* node = getNode( i );
        if( !node->isUsed( ))
            continue;

        if( !node->syncInit( ))
        {
            ERROR << "Init of " << node << " failed." << endl;
            _exit();
            return false;
        }
    }

//     foreach compound
//         init compound
//     foreach compound
//         sync init

    return true;
}

bool Config::_exit()
{
    return false;
}

std::ostream& eqs::operator << ( std::ostream& os, const Config* config )
{
    if( !config )
    {
        os << "NULL config";
        return os;
    }
    
    const uint nNodes     = config->nNodes();
    const uint nCompounds = config->nCompounds();
    os << "config " << (void*)config << " " << nNodes << " nodes "
           << nCompounds << " compounds";
    
    for( uint i=0; i<nNodes; i++ )
            os << std::endl << "    " << config->getNode(i);
    
    for( uint i=0; i<nCompounds; i++ )
        os << std::endl << "    " << config->getCompound(i);
    
    return os;
}
