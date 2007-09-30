
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"

#include <eq/base/scopedMutex.h>

using namespace eqBase;
using namespace std;

namespace eqNet
{
StaticSlaveCM::StaticSlaveCM( Object* object )
        : _object( object )
{
    registerCommand( CMD_OBJECT_INSTANCE_DATA,
          CommandFunc<StaticSlaveCM>( this, &StaticSlaveCM::_cmdInstanceData ));
}

StaticSlaveCM::~StaticSlaveCM()
{
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult StaticSlaveCM::_cmdInstanceData( Command& command )
{
    const ObjectInstanceDataPacket* packet = 
        command.getPacket<ObjectInstanceDataPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd instance data " << command << endl;

    EQASSERT( packet->version == Object::VERSION_NONE );
    _object->applyInstanceData( packet->data, packet->dataSize );
    return eqNet::COMMAND_HANDLED;
}
}
