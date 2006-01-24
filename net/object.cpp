
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "packets.h"

#include <eq/base/log.h>
#include <iostream>

using namespace eqNet;
using namespace std;

Object::Object()
        : _id( INVALID_ID ),
          _sessionID( INVALID_ID )
{
}

Object::~Object()
{
}

CommandResult Object::handleCommand( Node* node, const ObjectPacket* packet )
{
    Base* baseThis = dynamic_cast<Base*>( this );

    if( baseThis )
        return baseThis->handleCommand( node, (Packet*)packet );

    WARN << "Unhandled command " << packet << endl;
    return COMMAND_ERROR;
}
