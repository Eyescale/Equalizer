
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "fullSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectDeltaDataIStream.h"
#include "objectInstanceDataIStream.h"

#include <eq/base/scopedMutex.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

FullSlaveCM::FullSlaveCM( Object* object )
        : StaticSlaveCM( object ),
          _version( Object::VERSION_NONE ),
          _mutex( 0 )
{
    registerCommand( CMD_OBJECT_INSTANCE,
                  CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_cmdInstance ));
    registerCommand( CMD_OBJECT_DELTA_DATA,
                 CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_cmdDeltaData ));
    registerCommand( CMD_OBJECT_DELTA,
                     CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_cmdDelta ));
}

FullSlaveCM::~FullSlaveCM()
{
    delete _mutex;
    _mutex = 0;

    while( !_queuedVersions.empty( ))
        delete _queuedVersions.pop();

    delete _currentIStream;
    _currentIStream = 0;

    _version = Object::VERSION_NONE;
}

void FullSlaveCM::makeThreadSafe()
{
    if( _mutex ) return;

    _mutex = new eqBase::Lock;
}

bool FullSlaveCM::sync( const uint32_t version )
{
    EQLOG( LOG_OBJECTS ) << "sync to v" << version << ", id " 
                         << _object->getID() << "." << _object->getInstanceID()
                         << endl;
    if( _version == version )
        return true;

    if( !_mutex )
        CHECK_THREAD( _thread );

    ScopedMutex< Lock > mutex( _mutex );

    if( version == Object::VERSION_HEAD )
    {
        _syncToHead();
        return true;
    }

    EQASSERTINFO( _version <= version, "can't sync to older version of object");

    while( _version < version )
    {
        ObjectDataIStream* is = _queuedVersions.pop();
        _unpackOneVersion( is );
        EQASSERTINFO( _version == is->getVersion(), "Have version " 
                      << _version << " instead of " << is->getVersion( ));
        delete is;
    }

    _object->getLocalNode()->flushCommands();
    return true;
}

void FullSlaveCM::_syncToHead()
{
    if( _queuedVersions.empty( ))
        return;

    for( ObjectDataIStream* is = _queuedVersions.tryPop(); 
         is; is = _queuedVersions.tryPop( ))
    {
        _unpackOneVersion( is );
        EQASSERTINFO( _version == is->getVersion(), "Have version " 
                      << _version << " instead of " << is->getVersion( ));
        delete is;
    }

    _object->getLocalNode()->flushCommands();
}


uint32_t FullSlaveCM::getHeadVersion() const
{
    ObjectDataIStream* is = _queuedVersions.back();
    if( is )
        return is->getVersion();

    return _version;    
}

void FullSlaveCM::_unpackOneVersion( ObjectDataIStream* is )
{
    EQASSERT( is );

    EQASSERTINFO( _version == is->getVersion() - 1, "Expected version " 
                  << _version << ", got " << is->getVersion() - 1 );
    
    _object->applyInstanceData( *is );
    _version = is->getVersion();
    EQLOG( LOG_OBJECTS ) << "applied v" << _version << ", id "
                         << _object->getID() << "." << _object->getInstanceID()
                         << endl;

    if( is->getRemainingBufferSize() > 0 || is->nRemainingBuffers() > 0 )
        EQWARN << "Object did not unpack all data" << endl;
}


//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult FullSlaveCM::_cmdInstance( Command& command )
{
    if( !_currentIStream )
        _currentIStream = new ObjectInstanceDataIStream;

    _currentIStream->addDataPacket( command );
 
    const ObjectInstancePacket* packet = 
        command.getPacket<ObjectInstancePacket>();
    _currentIStream->setVersion( packet->version );

    _object->applyInstanceData( *_currentIStream );
    _version = packet->version;

    delete _currentIStream;
    _currentIStream = 0;

    EQLOG( LOG_OBJECTS ) << "v" << packet->version << ", id "
                         << _object->getID() << "." << _object->getInstanceID()
                         << " instantiated" << endl;

    return eqNet::COMMAND_HANDLED;
}

CommandResult FullSlaveCM::_cmdDeltaData( Command& command )
{
    if( !_currentIStream )
        _currentIStream = new ObjectDeltaDataIStream;

    _currentIStream->addDataPacket( command );
    return eqNet::COMMAND_HANDLED;
}

CommandResult FullSlaveCM::_cmdDelta( Command& command )
{
    if( !_currentIStream )
        _currentIStream = new ObjectDeltaDataIStream;

    const ObjectDeltaPacket* packet = command.getPacket<ObjectDeltaPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd delta " << command << endl;

    _currentIStream->addDataPacket( command );
    _currentIStream->setVersion( packet->version );
    
    EQLOG( LOG_OBJECTS ) << "v" << packet->version << ", id "
                         << _object->getID() << "." << _object->getInstanceID()
                         << " ready" << endl;

    _queuedVersions.push( _currentIStream );
    _currentIStream = 0;

    return eqNet::COMMAND_HANDLED;
}
