/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"

#include "connection.h"
#include "networkPriv.h"
#include "nodePriv.h"
#include "server.h"
#include "sessionPriv.h"

#include <eq/base/log.h>
#include <eq/net/connectionDescription.h>

#include <alloca.h>

using namespace eqNet;
using namespace std;

Session* Session::create( const char* server )
{
    return priv::Session::create(server);
}

uint Session::getLocalNodeID()
{
    return (static_cast<priv::Session*>(this))->getLocalNode()->getID();
}

void Session::exit()
{
}
