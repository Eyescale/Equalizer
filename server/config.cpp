
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

Config* Config::clone()
{
    Config *clone = new Config();

    const uint nCompounds = this->nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound = getCompound(i);
        clone->_compounds.push_back( new Compound( *compound ));
    }

    const uint nNodes = this->nNodes();
    for( uint i=0; i<nNodes; i++ )
    {
        Node* node = getNode(i);
        clone->_nodes.push_back( new Node( *node ));
    }

    // utilisation data will be shared between copies
    return clone;
}

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
    node->send( reply );
}

bool Config::_init()
{
    const uint nCompounds = this->nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound = getCompound( i );
        compound->init();
    }

//     foreach ref'd node
//         launch node
//     foreach ref'd node
//         sync launch, init node
//     foreach ref'd node
//         sync init

//     foreach compound
//         init compound
//     foreach compound
//         sync init
}
