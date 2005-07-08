
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"
#include "serverPriv.h"
#include "util.h"

using namespace eqNet;
using namespace std;

int Server::run( const char* address )
{
    char   hostname[MAXHOSTNAMELEN];
    ushort port;

    priv::Util::parseAddress( address, hostname, port );
    return priv::Server::run( hostname, port );
}
