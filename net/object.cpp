
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "base.h"
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

void Object::handleCommand( Node* node, const ObjectPacket* packet )
{
    Base* baseThis = dynamic_cast<Base*>( this );

    if( baseThis )
        baseThis->handleCommand( node, (Packet*)packet );
    else
        WARN << "Unhandled command " << packet << endl;
}
