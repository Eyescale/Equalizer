
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectInstanceDataIStream.h"

#include <eq/base/scopedMutex.h>

using namespace eqBase;
using namespace std;

namespace eqNet
{
StaticSlaveCM::StaticSlaveCM( Object* object )
        : _object( object )
        , _currentIStream( 0 )
{
    registerCommand( CMD_OBJECT_INSTANCE_DATA,
          CommandFunc<StaticSlaveCM>( this, &StaticSlaveCM::_cmdInstanceData ));
    registerCommand( CMD_OBJECT_INSTANCE,
              CommandFunc<StaticSlaveCM>( this, &StaticSlaveCM::_cmdInstance ));
}

StaticSlaveCM::~StaticSlaveCM()
{
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult StaticSlaveCM::_cmdInstanceData( Command& command )
{
    if( !_currentIStream )
        _currentIStream = new ObjectInstanceDataIStream;

    _currentIStream->addDataPacket( command );
    return eqNet::COMMAND_HANDLED;
}

CommandResult StaticSlaveCM::_cmdInstance( Command& command )
{
    if( !_currentIStream )
        _currentIStream = new ObjectInstanceDataIStream;

    _currentIStream->addDataPacket( command );
 
    const ObjectInstancePacket* packet = 
        command.getPacket<ObjectInstancePacket>();
    _currentIStream->setVersion( packet->version );

    EQLOG( LOG_OBJECTS ) << "id " << _object->getID() << "." 
                         << _object->getInstanceID() << " ready" << endl;

    _object->applyInstanceData( *_currentIStream );
    delete _currentIStream;
    _currentIStream = 0;

    return eqNet::COMMAND_HANDLED;
}

}
