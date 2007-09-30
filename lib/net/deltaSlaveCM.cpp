
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "deltaSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"

#include <eq/base/scopedMutex.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

DeltaSlaveCM::DeltaSlaveCM( Object* object )
        : FullSlaveCM( object )
{
    registerCommand( REQ_OBJECT_DELTA_DATA,
               CommandFunc<DeltaSlaveCM>( this, &DeltaSlaveCM::_reqDeltaData ));
}

DeltaSlaveCM::~DeltaSlaveCM()
{
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult DeltaSlaveCM::_reqDeltaData( Command& command )
{
    const ObjectDeltaDataPacket* packet = 
        command.getPacket<ObjectDeltaDataPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd delta " << command << endl;

    _object->unpack( packet->delta, packet->deltaSize );
    _version = packet->version;
    return eqNet::COMMAND_HANDLED;
}
