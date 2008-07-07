
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectInstanceDataIStream.h"
#include "session.h"

#include <eq/base/scopedMutex.h>

using namespace eq::base;
using namespace std;

namespace eqNet
{
StaticSlaveCM::StaticSlaveCM( Object* object )
        : _object( object )
        , _currentIStream( new ObjectInstanceDataIStream )
{
}

StaticSlaveCM::~StaticSlaveCM()
{
    delete _currentIStream;
    _currentIStream = 0;
}

void StaticSlaveCM::notifyAttached()
{
    Session* session = _object->getSession();
    EQASSERT( session );
    CommandQueue* queue = session->getCommandThreadQueue();

    registerCommand( CMD_OBJECT_INSTANCE_DATA,
           CommandFunc<StaticSlaveCM>( this, &StaticSlaveCM::_cmdInstanceData ),
                     queue );
    registerCommand( CMD_OBJECT_INSTANCE,
               CommandFunc<StaticSlaveCM>( this, &StaticSlaveCM::_cmdInstance ),
                     queue );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult StaticSlaveCM::_cmdInstanceData( Command& command )
{
    EQASSERT( _currentIStream );
    _currentIStream->addDataPacket( command );
    return eqNet::COMMAND_HANDLED;
}

CommandResult StaticSlaveCM::_cmdInstance( Command& command )
{
    EQASSERT( _currentIStream );
    _currentIStream->addDataPacket( command );
 
    const ObjectInstancePacket* packet = 
        command.getPacket<ObjectInstancePacket>();
    _currentIStream->setVersion( packet->version );

    EQLOG( LOG_OBJECTS ) << "id " << _object->getID() << "." 
                         << _object->getInstanceID() << " ready" << endl;
    return eqNet::COMMAND_HANDLED;
}

void StaticSlaveCM::applyMapData()
{
    EQASSERT( _currentIStream );
    _currentIStream->waitReady();

    _object->applyInstanceData( *_currentIStream );

    delete _currentIStream;
    _currentIStream = 0;

    EQLOG( LOG_OBJECTS ) << "Mapped initial data for " << _object->getID()
                         << "." << _object->getInstanceID() << " ready" << endl;
}

}
