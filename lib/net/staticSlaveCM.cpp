
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"

#include <eq/base/scopedMutex.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

StaticSlaveCM::StaticSlaveCM( Object* object )
        : _object( object ),
          _ready( false )
{
    registerCommand( CMD_OBJECT_INSTANCE_DATA,
                  CommandFunc<StaticSlaveCM>( this, &StaticSlaveCM::_cmdInit ));
}

StaticSlaveCM::~StaticSlaveCM()
{
}

bool StaticSlaveCM::syncInitial()
{
    _ready.waitEQ( true );
    EQASSERT( _initCommand.isValid( ));

    // OPT shortcut around invokeCommand()
    EQASSERT( _initCommand->command == CMD_OBJECT_INSTANCE_DATA );

    _reqInit( _initCommand );
    _initCommand.release();
    return true;
}    

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult StaticSlaveCM::_cmdInit( Command& command )
{
    _initCommand = command;
    _ready       = true;
    return eqNet::COMMAND_HANDLED;
}

CommandResult StaticSlaveCM::_reqInit( Command& command )
{
    const ObjectInstanceDataPacket* packet = 
        command.getPacket<ObjectInstanceDataPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd init " << command << endl;
    EQASSERT( packet->version == Object::VERSION_NONE );

    _object->applyInstanceData( packet->data, packet->dataSize );
    return eqNet::COMMAND_HANDLED;
}
