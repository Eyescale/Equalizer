
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"
#include "objectInstanceDataOStream.h"

using namespace eqBase;
using namespace std;

namespace eqNet
{
StaticMasterCM::StaticMasterCM( Object* object )
        : _object( object )
{}

StaticMasterCM::~StaticMasterCM()
{}

void StaticMasterCM::addSlave( NodePtr node, const uint32_t instanceID )
{
    ObjectInstanceDataOStream os( _object );
    os.setInstanceID( instanceID );

    os.enable( node );
    _object->getInstanceData( os );
    os.disable();
}
}
