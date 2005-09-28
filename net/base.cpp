
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "base.h"

#include "node.h"
#include "packets.h"

#include <eq/base/log.h>

using namespace eqNet;
using namespace std;

void Base::_cmdUnknown( Node* node, const Packet* packet )
{
    ERROR << "Unknown command " << packet << " from " << node << endl;
    abort();
}
