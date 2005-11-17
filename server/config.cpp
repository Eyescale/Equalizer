
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "node.h"

#include <eq/net/global.h>

using namespace eqs;
using namespace std;

Config::Config()
        : _id( INVALID_ID )
{
    for( int i=0; i<eq::CMD_CONFIG_ALL; i++ )
        _cmdHandler[i] = &eqs::Config::_cmdUnknown;

    _cmdHandler[eq::CMD_CONFIG_INIT] = &eqs::Config::_cmdInit;
}

void Config::addNode( Node* node )
{
    _nodes.push_back( node ); 
    node->_config = this; 
}

//===========================================================================
// command handling
//===========================================================================
void Config::handleCommand( eqNet::Node* node,
                               const eq::ConfigPacket* packet )
{
    VERB << "handleCommand " << packet << endl;
    ASSERT( packet->command < eq::CMD_CONFIG_ALL );

    (this->*_cmdHandler[packet->command])(node, packet);
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
