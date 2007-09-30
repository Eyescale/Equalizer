
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "fullSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"

#include <eq/base/scopedMutex.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

FullSlaveCM::FullSlaveCM( Object* object )
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _mutex( 0 )
{
    registerCommand( CMD_OBJECT_INSTANCE_DATA,
              CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_cmdInstanceData ));
    registerCommand( CMD_OBJECT_DELTA_DATA,
                  CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_cmdPushData ));
    registerCommand( REQ_OBJECT_DELTA_DATA,
                 CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_reqDeltaData ));
}

FullSlaveCM::~FullSlaveCM()
{
    delete _mutex;
}

void FullSlaveCM::makeThreadSafe()
{
    if( _mutex ) return;

    _mutex = new eqBase::Lock;
#ifdef EQ_CHECK_THREADSAFETY
    _syncQueue._thread.extMutex = true;
#endif
}

bool FullSlaveCM::sync( const uint32_t version )
{
    EQLOG( LOG_OBJECTS ) << "sync to v" << version << ", id " 
                         << _object->getID() << endl;
    if( _version == version )
        return true;

    if( !_mutex )
        CHECK_THREAD( _thread );

    ScopedMutex< Lock > mutex( _mutex );

    if( version ==  Object::VERSION_HEAD )
    {
        _syncToHead();
        return true;
    }

    EQASSERTINFO( _version <= version, "can't sync to older version of object");

    while( _version < version )
    {
        Command* command = _syncQueue.pop();

        EQASSERT( (*command)->command == REQ_OBJECT_DELTA_DATA );
        invokeCommand( *command );
    }
    _object->getLocalNode()->flushCommands();

    EQVERB << "Sync'ed to v" << version << ", id " << _object->getID() << endl;
    return true;
}

void FullSlaveCM::_syncToHead()
{
    if( _syncQueue.empty( ))
        return;

    for( Command* command = _syncQueue.tryPop(); command; 
         command = _syncQueue.tryPop( ))
    {
        EQASSERT( (*command)->command == REQ_OBJECT_DELTA_DATA );
        invokeCommand( *command );
    }

    _object->getLocalNode()->flushCommands();
    EQVERB << "Sync'ed to head v" << _version << ", id " << _object->getID() 
           << endl;
}

void FullSlaveCM::applyInitialData( const void* data, const uint64_t size,
                                     const uint32_t version )
{
    EQASSERT( _version == Object::VERSION_NONE );

    _object->applyInstanceData( data, size );
    _version = version;
}

uint32_t FullSlaveCM::getHeadVersion() const
{
    Command* command = _syncQueue.back();
    if( command && (*command)->command == REQ_OBJECT_INSTANCE_DATA )
    {
        const ObjectInstanceDataPacket* packet = 
            command->getPacket<ObjectInstanceDataPacket>();
        return packet->version;
    }

    return _version;    
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult FullSlaveCM::_cmdInstanceData( Command& command )
{
    const ObjectInstanceDataPacket* packet = 
        command.getPacket<ObjectInstanceDataPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd instance data " << command << endl;

    EQASSERT( packet->version != Object::VERSION_NONE );
    EQASSERT( _version == Object::VERSION_NONE );

    _object->applyInstanceData( packet->data, packet->dataSize );
    _version = packet->version;
    return eqNet::COMMAND_HANDLED;
}

CommandResult FullSlaveCM::_cmdPushData( Command& command )
{
    // init sent to all instances: make copy
    Command copy( command );
    _syncQueue.push( copy );

    return eqNet::COMMAND_HANDLED;
}

CommandResult FullSlaveCM::_reqDeltaData( Command& command )
{
    const ObjectDeltaDataPacket* packet = 
        command.getPacket<ObjectDeltaDataPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd delta " << command << endl;

    _object->applyInstanceData( packet->delta, packet->deltaSize );
    _version = packet->version;
    return eqNet::COMMAND_HANDLED;
}
