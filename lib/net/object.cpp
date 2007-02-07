
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "command.h"
#include "log.h"
#include "packets.h"
#include "session.h"

#include <eq/base/scopedMutex.h>
#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

void Object::_construct()
{
    _mutex         = 0;
    _session       = 0;
    _id            = EQ_ID_INVALID;
    _instanceID    = EQ_ID_INVALID;
    _master        = false;
    _version       = VERSION_NONE;
    _commitCount   = 0;
    _nVersions     = 0;
    _obsoleteFlags = AUTO_OBSOLETE_COUNT_VERSIONS;
    _instanceData     = 0;
    _instanceDataSize = 0;
    _deltaData     = 0;
    _deltaDataSize = 0;

    registerCommand( CMD_OBJECT_INIT,
                     CommandFunc<Object>( this, &Object::_cmdPushData ));
    registerCommand( REQ_OBJECT_INIT,
                     CommandFunc<Object>( this, &Object::_reqInit ));
    registerCommand( CMD_OBJECT_SYNC, 
                     CommandFunc<Object>( this, &Object::_cmdPushData ));
    registerCommand( REQ_OBJECT_SYNC,
                     CommandFunc<Object>( this, &Object::_reqSync ));
    registerCommand( CMD_OBJECT_COMMIT, 
                     CommandFunc<Object>( this, &Object::_cmdCommit ));
}

Object::Object()
{
    _construct();
}

Object::Object( const Object& from )
{
    _construct();
    _instanceData     = from._instanceData;
    _instanceDataSize = from._instanceDataSize;
    _deltaData        = from._deltaData;
    _deltaDataSize    = from._deltaDataSize;
}

Object::~Object()
{
    if( _session ) // Still registered
        EQERROR << "Object is still registered in session " << _session->getID()
                << " in destructor" << endl;

    for( deque<InstanceData>::const_iterator iter = _instanceDatas.begin(); 
         iter != _instanceDatas.end(); ++iter )
    {
        if( (*iter).data )
            free( (*iter).data );
    }
    _instanceDatas.clear();

    for( deque<ChangeData>::const_iterator iter = _changeDatas.begin(); 
         iter != _changeDatas.end(); ++iter )
    {
        if( (*iter).data )
            free( (*iter).data );
    }
    _changeDatas.clear();
    delete _mutex;
}

void Object::makeThreadSafe()
{
    EQASSERT( _id == EQ_ID_INVALID );

    if( _mutex ) return;
    _mutex = new eqBase::Lock;
#ifdef EQ_CHECK_THREADSAFETY
    _syncQueue._thread.extMutex = true;
#endif
}

RefPtr<Node> Object::getLocalNode()
{ 
    return _session ? _session->getLocalNode() : 0; 
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const std::string& string )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const void* data, const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

bool Object::send( NodeVector& nodes, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet );
}
bool Object::send( NodeVector nodes, ObjectPacket& packet, const void* data,
                   const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet, data, size );
}

uint32_t Object::commit()
{
    const uint32_t requestID = commitNB();
    return commitSync( requestID );
}

uint32_t Object::commitNB()
{
    EQASSERT( isMaster( ));
    EQASSERTINFO( !isStatic( ), "Object type " << typeid(*this).name( ));

    ObjectCommitPacket packet;
    packet.instanceID = _instanceID;
    packet.requestID  = _requestHandler.registerRequest();

    send( getLocalNode(), packet );
    return packet.requestID;
}

uint32_t Object::commitSync( const uint32_t commitID )
{
    return (uint32_t)(long long)_requestHandler.waitRequest( commitID );
}

uint32_t Object::_commitInitial()
{
    CHECK_THREAD( _thread );

    // compute base version
    uint64_t    size;
    const void* ptr  = getInstanceData( &size );

    _setInitialVersion( ptr, size );
    releaseInstanceData( ptr );
        
    return _version;
}

void Object::_setInitialVersion( const void* ptr, const uint64_t size )
{
    EQASSERT( _version == VERSION_NONE );
    EQASSERT( _instanceDatas.empty( ));
    EQASSERT( _changeDatas.empty( ));

    InstanceData data;

    data.size = size;
    if( ptr && size )
    {
        data.data    = malloc( size );
        data.maxSize = size;
        memcpy( data.data, ptr, size );
    }

    _instanceDatas.push_front( data );
    ++_version;
    ++_commitCount;
}

// Obsoletes old changes based on number of commits or number of versions,
// depending on the obsolete flags.
void Object::_obsolete()
{
    if( _obsoleteFlags & AUTO_OBSOLETE_COUNT_COMMITS )
    {
        InstanceData& lastInstanceData = _instanceDatas.back();
        if( lastInstanceData.commitCount < (_commitCount - _nVersions) &&
            _instanceDatas.size() > 1 )
        {
            EQASSERT( !_changeDatas.empty( ));
            _instanceDataCache.push_back( lastInstanceData );
            _instanceDatas.pop_back();

            _changeDataCache.push_back( _changeDatas.back( ));        
            _changeDatas.pop_back();
        }
        _checkConsistency();
        return;
    }
    // else count versions
    while( _instanceDatas.size() > (_nVersions+1) )
    {
        _instanceDataCache.push_back( _instanceDatas.back( ));
        _instanceDatas.pop_back();
        
        if( _nVersions > 0 )
        {
            EQASSERT( !_changeDatas.empty( ));
            _changeDataCache.push_back( _changeDatas.back( ));        
            _changeDatas.pop_back();
        }
        _checkConsistency();
    }
}

bool Object::sync( const uint32_t version )
{
    EQLOG( LOG_OBJECTS ) << "sync to v" << version << ", id " << getID() <<endl;
    if( _version == version )
        return true;

    if( !_mutex )
        CHECK_THREAD( _thread );

    ScopedMutex mutex( _mutex );

    if( version ==  VERSION_HEAD )
    {
        _syncToHead();
        return true;
    }

    EQASSERTINFO( !isMaster(), "can't sync master to a specified version" );
    EQASSERTINFO( _version <= version, "can't sync to older version of object");

    while( _version < version )
    {
        Command* command = _syncQueue.pop();

         // OPT shortcut around invokeCommand()
        EQASSERT( (*command)->command == REQ_OBJECT_SYNC );
        _reqSync( *command );
    }
    getLocalNode()->flushCommands();

    EQVERB << "Sync'ed to v" << version << ", id " << getID() << endl;
    return true;
}

bool Object::_syncInitial()
{
    if( _version != VERSION_NONE )
        return false;

    Command* command = _syncQueue.pop();

    // OPT shortcut around invokeCommand()
    EQASSERT( (*command)->command == REQ_OBJECT_INIT );
    return( _reqInit( *command ) == COMMAND_HANDLED );
}    


void Object::_syncToHead()
{
    if( isMaster( ) || _syncQueue.empty( ))
        return;

    for( Command* command = _syncQueue.tryPop(); command; 
         command = _syncQueue.tryPop( ))
    {
        EQASSERT( (*command)->command == REQ_OBJECT_SYNC );
        _reqSync( *command ); // XXX shortcut around invokeCommand()
    }

    getLocalNode()->flushCommands();
    EQVERB << "Sync'ed to head v" << _version << ", id " << getID() 
           << endl;
}

uint32_t Object::getHeadVersion() const
{
    if( isMaster( ))
        return _version;

    Command* command = _syncQueue.back();
    if( command )
    {
        EQASSERT( (*command)->command == REQ_OBJECT_SYNC );
        const ObjectSyncPacket* packet = command->getPacket<ObjectSyncPacket>();
        return packet->version;
    }

    return _version;    
}

void Object::setInstanceData( void* data, const uint64_t size )
{
    _instanceData     = data;
    _instanceDataSize = size;

    if( _deltaData )
        return;
    _deltaData     = data;
    _deltaDataSize = size;
}

void Object::addSlave( RefPtr<Node> node, const uint32_t instanceID )
{
    EQASSERT( _session );
    EQASSERT( _id != EQ_ID_INVALID );
    _checkConsistency();

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    EQLOG( LOG_OBJECTS ) << "Object id " << _id << " v" << _version
                         << ", instanciate on " << node->getNodeID() << endl;

    ObjectInitPacket initPacket;
    initPacket.instanceID     = instanceID;
    initPacket.dataSize = 0;

    if( isStatic( ))
    {
        const void* data   = getInstanceData( &initPacket.dataSize );
        initPacket.version = _version;
        EQLOG( LOG_OBJECTS ) << "send " << &initPacket << endl;
        send( node, initPacket, data, initPacket.dataSize );
        releaseInstanceData( data );
        return;
    }

    if( _version == VERSION_NONE )
        _commitInitial();

    const uint32_t      age  = _instanceDatas.size() - 1;
    const InstanceData& data = _instanceDatas.back();

    initPacket.version  = _version - age;
    initPacket.dataSize = data.size;

    EQLOG( LOG_OBJECTS ) << "send " << &initPacket << endl;
    send( node, initPacket, data.data, initPacket.dataSize );    

    // send all deltas
    for( deque<ChangeData>::reverse_iterator i = _changeDatas.rbegin();
         i != _changeDatas.rend(); ++i )
    {
        const ChangeData& data = *i;

        ObjectSyncPacket deltaPacket;

        deltaPacket.version   = data.version;
        deltaPacket.deltaSize = data.size;
        
        EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << endl;
        send( node, deltaPacket, data.data, data.size );
    }
}

void Object::removeSlave( RefPtr<Node> node )
{
    EQASSERT( _session );
    EQASSERT( _id != EQ_ID_INVALID );
    _checkConsistency();

    // remove from subscribers
    const NodeID& nodeID = node->getNodeID();
    EQASSERT( _slavesCount[ nodeID ] != 0 );

    --_slavesCount[ nodeID ];
    if( _slavesCount[ nodeID ] == 0 )
    {
        NodeVector::iterator i = find( _slaves.begin(), _slaves.end(), node );
        EQASSERT( i != _slaves.end( ));
        _slaves.erase( i );
        _slavesCount.erase( nodeID );
    }
}

void Object::_checkConsistency() const
{
#ifndef NDEBUG
    if( isStatic() || _version == VERSION_NONE )
        return;

    EQASSERT( _instanceDatas.size() == _changeDatas.size() + 1 );

    if( !( _obsoleteFlags & AUTO_OBSOLETE_COUNT_COMMITS )) // count versions
    {
        if( _version > _nVersions )
        {
            EQASSERT( _changeDatas.size() == _nVersions );
        }
        else
        {
            EQASSERT( _instanceDatas.size() == _version );
        }
    }

    uint32_t version = _version;
    for( deque<ChangeData>::const_iterator i = _changeDatas.begin();
         i != _changeDatas.end(); ++i )
    {
        const ChangeData& data = *i;
        EQASSERT( data.version == version );
        EQASSERT( version > 0 );
        --version;
    }
#endif
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult Object::_cmdPushData( Command& command )
{
    if( command->command == CMD_OBJECT_SYNC )
    {
        // sync sent to all instances: make copy
        Command copy( command );
        _syncQueue.push( copy );
    }
    else
        _syncQueue.push( command );

    return eqNet::COMMAND_HANDLED;
}

CommandResult Object::_reqInit( Command& command )
{
    const ObjectInitPacket* packet = command.getPacket<ObjectInitPacket>();
    EQLOG( LOG_OBJECTS ) << "cmd init " << command << endl;

    applyInstanceData( packet->data, packet->dataSize );
    _version = packet->version;
    return eqNet::COMMAND_HANDLED;
}

CommandResult Object::_reqSync( Command& command )
{
    const ObjectSyncPacket* packet = command.getPacket<ObjectSyncPacket>();
    EQLOG( LOG_OBJECTS ) << "req sync v" << _version << " " << command << endl;
    EQASSERT( _version == packet->version-1 );

    unpack( packet->delta, packet->deltaSize );
    _version = packet->version;
    return eqNet::COMMAND_HANDLED;
}

CommandResult Object::_cmdCommit( Command& command )
{
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command << endl;

    EQASSERT( isMaster( ));

    if( _version == VERSION_NONE )
    {
        _commitInitial();
        EQASSERT( _slaves.empty( ));
        _requestHandler.serveRequest( packet->requestID, 
                                      reinterpret_cast<void*>(_version) );
        _checkConsistency();
        return COMMAND_HANDLED;
    }

    EQASSERT( _instanceDatas.size() == _changeDatas.size() + 1 );

    ++_commitCount;

    uint64_t    deltaSize = 0;
    const void* delta     = pack( &deltaSize );

    if( !delta || deltaSize == 0 )
    {
        _obsolete();
        _requestHandler.serveRequest( packet->requestID, 
                                      reinterpret_cast<void*>(_version) );
        _checkConsistency();
        return COMMAND_HANDLED;
    }

    ++_version;
    EQASSERT( _version );
    
    // save delta
    if( _nVersions > 0 )
    {
        ChangeData changeData;
        if( !_changeDataCache.empty( ))
        {
            changeData = _changeDataCache.back();
            _changeDataCache.pop_back();
        }

        if( deltaSize > changeData.maxSize )
        {
            if( changeData.data )
                free( changeData.data );
            
            changeData.data    = malloc( deltaSize );
            changeData.maxSize = deltaSize;
        }
        memcpy( changeData.data, delta, deltaSize );
        changeData.size        = deltaSize;
        changeData.commitCount = _commitCount;
        changeData.version     = _version;
        _changeDatas.push_front( changeData );
    }

    // save instance data
    InstanceData instanceData;
    if( !_instanceDataCache.empty( ))
    {
        instanceData = _instanceDataCache.back();
        _instanceDataCache.pop_back();
    }
    const void* ptr = getInstanceData( &instanceData.size );
    if( instanceData.size > instanceData.maxSize )
    {
        if( instanceData.data )
            free( instanceData.data );
        instanceData.data    = malloc( instanceData.size );
        instanceData.maxSize = instanceData.size;
    }
    if( ptr )
        memcpy( instanceData.data, ptr, instanceData.size );
    releaseInstanceData( ptr );
    instanceData.commitCount = _commitCount;
    _instanceDatas.push_front( instanceData );
    
    _obsolete();

    // send delta to subscribed slaves
    if( !_slaves.empty( ))
    {
        ObjectSyncPacket  deltaPacket;

        deltaPacket.version   = _version;
        deltaPacket.deltaSize = deltaSize;
        
        EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << " to " 
                             << _slaves.size() << " nodes " << endl;
        send( _slaves, deltaPacket, delta, deltaSize );
    }

    releasePackData( delta );
    EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " << getID() 
                         << endl;

    _requestHandler.serveRequest( packet->requestID, 
                                  reinterpret_cast<void*>(_version) );
    _checkConsistency();
    return COMMAND_HANDLED;
}
