
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"
#include "serverPriv.h"

using namespace eqNet;
using namespace std;

int Server::run( const char* address )
{
    return priv::Server::run(address);
}
