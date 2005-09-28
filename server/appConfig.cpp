
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "appConfig.h"

#include <eq/net/global.h>

using namespace eqs;
using namespace std;

AppConfig::AppConfig(const Config& from)
        : Config(from),
          _id(INVALID_ID)
{
}

void AppConfig::handleCommand( eqNet::Node* node,
                               const eq::ConfigPacket* packet )
{
    ERROR << "unimplemented" << endl;
    abort();
}
