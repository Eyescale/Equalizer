
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"

using namespace eqBase;
using namespace std;

namespace eqNet
{
StaticMasterCM::StaticMasterCM( Object* object )
        : _object( object )
{}

StaticMasterCM::~StaticMasterCM()
{}

void StaticMasterCM::addSlave( eqBase::RefPtr<Node> node,
                               const uint32_t instanceID )
{
    ObjectInstanceDataPacket instPacket;
    const void* data = _object->getInstanceData( &instPacket.dataSize );

    instPacket.instanceID = instanceID;
    instPacket.version    = Object::VERSION_NONE;

    _object->send( node, instPacket, data, instPacket.dataSize );
    _object->releaseInstanceData( data );
}
}
