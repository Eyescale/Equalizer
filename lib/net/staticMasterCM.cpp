
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

StaticMasterCM::StaticMasterCM( Object* object )
        : _object( object )
{}

StaticMasterCM::~StaticMasterCM()
{}

void StaticMasterCM::addSlave( RefPtr<Node> node, const uint32_t instanceID )
{
    ObjectInitPacket initPacket;
    initPacket.instanceID = instanceID;
    initPacket.dataSize   = 0;
    initPacket.version    = Object::VERSION_NONE;

    const void* data   = _object->getInstanceData( &initPacket.dataSize );

    EQLOG( LOG_OBJECTS ) << "send " << &initPacket << endl;
    _object->send( node, initPacket, data, initPacket.dataSize );
    _object->releaseInstanceData( data );
}
