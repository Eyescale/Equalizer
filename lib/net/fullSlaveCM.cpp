
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
typedef CommandFunc<FullSlaveCM> CmdFunc;

FullSlaveCM::FullSlaveCM( Object* object, uint32_t masterInstanceID )
        : StaticSlaveCM( object )
        , _version( Object::VERSION_NONE )
        , _mutex( 0 )
        , _currentDeltaStream( 0 )
        , _masterInstanceID( masterInstanceID )
{
    registerCommand( CMD_OBJECT_DELTA_DATA,
                     CmdFunc( this, &FullSlaveCM::_cmdDeltaData ), 0 );
    registerCommand( CMD_OBJECT_DELTA,
                     CmdFunc( this, &FullSlaveCM::_cmdDelta ), 0 );
    registerCommand( CMD_OBJECT_VERSION,
                     CmdFunc( this, &FullSlaveCM::_cmdVersion ), 0 );
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

void FullSlaveCM::makeThreadSafe()
{
    if( _mutex ) return;

    _mutex = new Lock;
}

uint32_t FullSlaveCM::sync( const uint32_t version )
{
    EQLOG( LOG_OBJECTS ) << "sync to v" << version << ", id " 
                         << _object->getID() << "." << _object->getInstanceID()
                         << endl;
    if( _version == version )
        return _version;

    if( !_mutex )
        CHECK_THREAD( _thread );

    ScopedMutex mutex( _mutex );

    if( version == Object::VERSION_HEAD )
    {
        _syncToHead();
        return _version;
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
    return _version;
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
                  << _version + 1 << ", got " << is->getVersion() );
    
    _object->unpack( *is );
    _version = is->getVersion();
    EQLOG( LOG_OBJECTS ) << "applied v" << _version << ", id "
                         << _object->getID() << "." << _object->getInstanceID()
                         << endl;

    if( is->getRemainingBufferSize() > 0 || is->nRemainingBuffers() > 0 )
        EQWARN << "Object " << typeid( *_object ).name() 
            << " did not unpack all data" << endl;
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
                         << "." << _object->getInstanceID() << " v" << _version
                         << " ready" << std::endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------

CommandResult FullSlaveCM::_cmdDeltaData( Command& command )
{
    if( !_currentDeltaStream )
        _currentDeltaStream = new ObjectDeltaDataIStream;

    _currentDeltaStream->addDataPacket( command );
    return COMMAND_HANDLED;
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
    _object->notifyNewHeadVersion( packet->version );
    _currentDeltaStream = 0;

    return COMMAND_HANDLED;
}

CommandResult FullSlaveCM::_cmdVersion( Command& command )
{
    const ObjectVersionPacket* packet = 
        command.getPacket< ObjectVersionPacket >();
    _version = packet->version;
    return COMMAND_HANDLED;
}

}
}
