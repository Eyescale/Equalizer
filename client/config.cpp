
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "packets.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eq;
using namespace std;

Config::Config( const uint id, Server* server )
        : _id(id),
          _server(server)
{
    ASSERT( id != INVALID_ID );
    ASSERT( server );
}


bool Config::init()
{
    ConfigInitPacket packet( this );
    packet.requestID = _requestHandler.registerRequest();
    _server->send( packet );
    return ( _requestHandler.waitRequest( packet.requestID ) != 0 );
}

bool Config::exit()
{
}
