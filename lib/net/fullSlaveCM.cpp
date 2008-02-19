
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "fullSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectDeltaDataIStream.h"
#include "objectInstanceDataIStream.h"
#include "session.h"

#include <eq/base/scopedMutex.h>

using namespace eqBase;
using namespace std;

namespace eqNet
{
FullSlaveCM::FullSlaveCM( Object* object, uint32_t masterInstanceID )
        : StaticSlaveCM( object )
        , _version( Object::VERSION_NONE )
        , _mutex( 0 )
        , _currentDeltaStream( 0 )
        , _masterInstanceID( masterInstanceID )
{
}

FullSlaveCM::~FullSlaveCM()
{
    delete _mutex;
    _mutex = 0;

    while( !_queuedVersions.empty( ))
        delete _queuedVersions.pop();

    delete _currentDeltaStream;
    _currentDeltaStream = 0;

    _version = Object::VERSION_NONE;
}

void FullSlaveCM::notifyAttached()
{
    StaticSlaveCM::notifyAttached();

    Session* session = _object->getSession();
    EQASSERT( session );
    CommandQueue& queue = session->getCommandThreadQueue();

    registerCommand( CMD_OBJECT_DELTA_DATA,
                  CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_cmdDeltaData ),
                     queue );
    registerCommand( CMD_OBJECT_DELTA,
                     CommandFunc<FullSlaveCM>( this, &FullSlaveCM::_cmdDelta ),
                     queue );
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


void FullSlaveCM::applyMapData()
{
    EQASSERTINFO( _currentIStream, typeid( *_object ).name() << " id " <<
                  _object->getID() << "." << _object->getInstanceID( ));

    _currentIStream->waitReady();

    _object->applyInstanceData( *_currentIStream );
    _version = _currentIStream->getVersion();

    delete _currentIStream;
    _currentIStream = 0;

    EQLOG( LOG_OBJECTS ) << "Mapped initial data for " << _object->getID()
                         << "." << _object->getInstanceID() << " ready" << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------

CommandResult FullSlaveCM::_cmdDeltaData( Command& command )
{
    if( !_currentDeltaStream )
        _currentDeltaStream = new ObjectDeltaDataIStream;

    _currentDeltaStream->addDataPacket( command );
    return eqNet::COMMAND_HANDLED;
}

CommandResult FullSlaveCM::_cmdDelta( Command& command )
{
    if( !_currentDeltaStream )
        _currentDeltaStream = new ObjectDeltaDataIStream;

    const ObjectDeltaPacket* packet = command.getPacket<ObjectDeltaPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd delta " << command << endl;

    _currentDeltaStream->addDataPacket( command );
    _currentDeltaStream->setVersion( packet->version );
    
    EQLOG( LOG_OBJECTS ) << "v" << packet->version << ", id "
                         << _object->getID() << "." << _object->getInstanceID()
                         << " ready" << endl;

    _queuedVersions.push( _currentDeltaStream );
    _currentDeltaStream = 0;

    return eqNet::COMMAND_HANDLED;
}
}
