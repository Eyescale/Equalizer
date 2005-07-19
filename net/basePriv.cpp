
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "basePriv.h"
#include "packet.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

void Base::_cmdUnknown( const Packet* packet )
{
    ERROR << "Unknown command " << packet << endl;
    exit( EXIT_FAILURE );
}
