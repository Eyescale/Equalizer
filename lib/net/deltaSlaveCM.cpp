
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
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _mutex( 0 )
{
    registerCommand( CMD_OBJECT_DELTA_DATA, 
                CommandFunc<DeltaSlaveCM>( this, &DeltaSlaveCM::_cmdPushData ));
}

DeltaSlaveCM::~DeltaSlaveCM()
{
    delete _mutex;
}

void DeltaSlaveCM::makeThreadSafe()
{
    if( _mutex ) return;

    _mutex = new eqBase::Lock;
#ifdef EQ_CHECK_THREADSAFETY
    _syncQueue._thread.extMutex = true;
#endif
}

bool DeltaSlaveCM::sync( const uint32_t version )
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

         // OPT shortcut around invokeCommand()
        EQASSERT( (*command)->command == REQ_OBJECT_DELTA_DATA );
        _reqSync( *command );
    }
    _object->getLocalNode()->flushCommands();

    EQVERB << "Sync'ed to v" << version << ", id " << _object->getID() << endl;
    return true;
}

void DeltaSlaveCM::_syncToHead()
{
    if( _syncQueue.empty( ))
        return;

    for( Command* command = _syncQueue.tryPop(); command; 
         command = _syncQueue.tryPop( ))
    {
        EQASSERT( (*command)->command == REQ_OBJECT_DELTA_DATA );
        _reqSync( *command ); // XXX shortcut around invokeCommand()
    }

    _object->getLocalNode()->flushCommands();
    EQVERB << "Sync'ed to head v" << _version << ", id " << _object->getID() 
           << endl;
}

void DeltaSlaveCM::applyInitialData( const void* data, const uint64_t size,
                                     const uint32_t version )
{
    EQASSERT( _version == Object::VERSION_NONE );

    _object->applyInstanceData( data, size );
    _version = version;
}

uint32_t DeltaSlaveCM::getHeadVersion() const
{
    Command* command = _syncQueue.back();
    if( command && (*command)->command == REQ_OBJECT_DELTA_DATA )
    {
        const ObjectDeltaDataPacket* packet = 
            command->getPacket<ObjectDeltaDataPacket>();
        return packet->version;
    }

    return _version;    
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult DeltaSlaveCM::_cmdPushData( Command& command )
{
    if( command->command == CMD_OBJECT_DELTA_DATA )
    {
        // sync sent to all instances: make copy
        Command copy( command );
        _syncQueue.push( copy );
    }
    else
        _syncQueue.push( command );

    return eqNet::COMMAND_HANDLED;
}

CommandResult DeltaSlaveCM::_reqSync( Command& command )
{
    const ObjectDeltaDataPacket* packet = 
        command.getPacket<ObjectDeltaDataPacket>();
    EQLOG( LOG_OBJECTS ) << "req sync v" << _version << " " << command << endl;
    EQASSERT( _version == packet->version-1 );

    _object->unpack( packet->delta, packet->deltaSize );
    _version = packet->version;
    return eqNet::COMMAND_HANDLED;
}
