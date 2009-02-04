
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"
#include "objectInstanceDataOStream.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
StaticMasterCM::StaticMasterCM( Object* object )
        : _object( object )
{}

StaticMasterCM::~StaticMasterCM()
{}

void StaticMasterCM::addSlave( NodePtr node, const uint32_t instanceID, 
                               const uint32_t version )
{
    EQASSERT( version == Object::VERSION_OLDEST ); // VERSION_NONE is useless
    ObjectInstanceDataOStream os( _object );
    os.setInstanceID( instanceID );

    os.enable( node );
    _object->getInstanceData( os );
    os.disable();
}
}
}
