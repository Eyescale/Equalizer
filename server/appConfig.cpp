
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "appConfig.h"

#include <eq/net/global.h>
#include <eq/net/node.h>

using namespace eqs;
using namespace std;

AppConfig::AppConfig(const Config& from)
        : Config(from),
          _id(INVALID_ID)
{
    for( int i=0; i<eq::CMD_CONFIG_ALL; i++ )
        _cmdHandler[i] = &eqs::AppConfig::_cmdUnknown;

    _cmdHandler[eq::CMD_CONFIG_INIT] = &eqs::AppConfig::_cmdInit;
}

void AppConfig::handleCommand( eqNet::Node* node,
                               const eq::ConfigPacket* packet )
{
    VERB << "handleCommand " << packet << endl;
    ASSERT( packet->command < eq::CMD_CONFIG_ALL );

    (this->*_cmdHandler[packet->command])(node, packet);
}

void AppConfig::_cmdInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    const eq::ConfigInitPacket* packet = (eq::ConfigInitPacket*)pkg;
    eq::ConfigInitReplyPacket   reply( packet );
    INFO << "handle config init " << packet << endl;

    // TODO
    reply.result = false;
    node->send( reply );
}
