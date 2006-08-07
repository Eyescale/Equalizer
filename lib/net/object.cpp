
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

// TODO:
// - thread-safety: right now, commits and obsoletions are done by the app
// thread, whereas the instanciation requests are served by the recv thread.

#include "object.h"

#include "packets.h"
#include "session.h"

#include <eq/base/log.h>
#include <eq/base/scopedMutex.h>
#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

void Object::_construct()
{
    _mutex         = NULL;
    _session       = NULL;
    _id            = EQ_INVALID_ID;
    _policy        = SHARE_UNDEFINED;
    _version       = 0;
    _commitCount   = 0;
    _nVersions     = 0;
    _obsoleteFlags = AUTO_OBSOLETE_COUNT_VERSIONS;
    _distributedData     = NULL;
    _distributedDataSize = 0;

    registerCommand( CMD_OBJECT_SYNC, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Object::_cmdSync ));
    registerCommand( REQ_OBJECT_SYNC, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Object::_reqSync ));

    CHECK_THREAD_INIT( _threadID );
}

Object::Object( const uint32_t typeID, const uint32_t nCommands )
        : Base( nCommands ),
          _typeID( typeID )
{
    EQASSERT( nCommands >= CMD_OBJECT_CUSTOM );
    _construct();
}

Object::Object( const Object& from )
        : Base( from.getNCommands( )),
          _typeID( from._typeID )
{
    _construct();
    _distributedData     = from._distributedData;
    _distributedDataSize = from._distributedDataSize;
}

Object::~Object()
{
    for( list<InstanceData>::iterator iter = _instanceData.begin(); 
         iter != _instanceData.end(); ++iter )
    {
        if( (*iter).data )
            free( (*iter).data );
    }
    for( list<ChangeData>::iterator iter = _changeData.begin(); 
         iter != _changeData.end(); ++iter )
    {
        if( (*iter).data )
            free( (*iter).data );
    }
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_INVALID_ID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const std::string& string )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_INVALID_ID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const void* data, const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_INVALID_ID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

bool Object::send( NodeVector& nodes, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_INVALID_ID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet );
}
bool Object::send( NodeVector nodes, ObjectPacket& packet, const void* data,
                   const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_INVALID_ID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet, data, size );
}

void Object::instanciateOnNode( RefPtr<Node> node, const SharePolicy policy, 
                                const uint32_t version )
{
    EQASSERT( _session );
    EQASSERT( _id != EQ_INVALID_ID );

    addSlave( node );

    SessionInstanciateObjectPacket packet;
    packet.sessionID      =  _session->getID();
    packet.objectID       = _id;
    packet.objectType     = _typeID;
    packet.objectDataSize = 0;
    packet.isMaster       = !_master;
    packet.policy         = policy;

    if( _typeID < TYPE_VERSIONED )
    {
        const void* data = getInstanceData( &packet.objectDataSize );
        node->send( packet, data, packet.objectDataSize );
        releaseInstanceData( data );
        return;
    }

    if( _version == VERSION_NONE )
        _commitInitial();

    if( _version <= version )
    {
        InstanceData& data = _instanceData.front();
        packet.objectDataSize = data.size;
        node->send( packet, data.data, packet.objectDataSize );
        return;
    }

    const uint32_t age = _version - version;
    if( age > _instanceData.size( ))
    {
        EQWARN << "Instanciation request for obsolete version " << version
               << " for object of type " << typeid(*this).name() << endl;
        packet.error = true;
        node->send( packet );
    }

    list<InstanceData>::iterator iter = _instanceData.begin();
    for( uint32_t i=0; i<age; ++i ) ++iter;
    InstanceData& data = *iter;

    packet.objectDataSize = data.size;
    node->send( packet, data.data, packet.objectDataSize );    

    // send deltas since version
    for( list<ChangeData>::iterator iter = _changeData.end(); 
         iter !=_changeData.begin(); --iter )
    {
        ChangeData& data = (*iter);
        if( data.version < version )
            continue;

        ObjectSyncPacket  packet;

        packet.version   = data.version;
        packet.deltaSize = data.size;

        send( node, packet, data.data, data.size );
    }
}

uint32_t Object::commit()
{
    if( !isMaster( ) || _typeID < TYPE_VERSIONED )
        return VERSION_NONE;

    if( !_mutex )
        CHECK_THREAD( _threadID );

    ScopedMutex mutex( _mutex );
    if( _version == VERSION_NONE )
        return _commitInitial();

    EQASSERT( _instanceData.size() == _changeData.size() + 1 );

    ++_commitCount;

    uint64_t    deltaSize = 0;
    const void* delta     = pack( &deltaSize );

    if( !delta || deltaSize == 0 )
    {
        _obsolete();
        return _version;
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
        _changeData.push_front( changeData );
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
    _instanceData.push_front( instanceData );
    
    _obsolete();

    // send delta to subscribed slaves
    const NodeVector& slaves = getSlaves();
    ObjectSyncPacket  packet;

    packet.version   = _version;
    packet.deltaSize = deltaSize;

    send( slaves, packet, delta, deltaSize );

    releasePackData( delta );
    EQVERB << "Committed version " << _version << ", id " << getID() << endl;
    return _version;
}

uint32_t Object::_commitInitial()
{
    CHECK_THREAD( _threadID );
    EQASSERT( _version == VERSION_NONE );
    EQASSERT( _instanceData.empty( ));
    EQASSERT( _changeData.empty( ));

    // compute base version
    InstanceData data;
    const void*  ptr  = getInstanceData( &data.size );

    if( ptr )
    {
        data.data    = malloc( data.size );
        data.maxSize = data.size;
        memcpy( data.data, ptr, data.size );
    }
    _instanceData.push_front( data );
    releaseInstanceData( ptr );
        
    ++_version;
    ++_commitCount;
    return _version;
}

// Obsoletes old changes based on number of commits or number of versions,
// depending on the obsolete flags.
void Object::_obsolete()
{
    if( _obsoleteFlags & AUTO_OBSOLETE_COUNT_COMMITS )
    {
        InstanceData& lastInstanceData = _instanceData.back();
        if( lastInstanceData.commitCount < (_commitCount - _nVersions) &&
            _instanceData.size() > 1 )
        {
            EQASSERT( !_changeData.empty( ));
            _instanceDataCache.push_back( lastInstanceData );
            _instanceData.pop_back();

            _changeDataCache.push_back( _changeData.back( ));        
            _changeData.pop_back();
        }
        EQASSERT( _instanceData.size() == _changeData.size() + 1 );
        return;
    }

    while( _instanceData.size() > (_nVersions+1) )
    {
        _instanceDataCache.push_back( _instanceData.back( ));
        _instanceData.pop_back();
        
        if( _nVersions > 0 )
        {
            EQASSERT( !_changeData.empty( ));
            _changeDataCache.push_back( _changeData.back( ));        
            _changeData.pop_back();
            EQASSERT( _instanceData.size() == _changeData.size() + 1 );
        }
    }
}

bool Object::sync( const uint32_t version, const float timeout )
{
    EQVERB << "Sync to version " << version << ", id " << getID() << endl;
    if( _version == version )
        return true;

    if( !_mutex )
        CHECK_THREAD( _threadID );

    ScopedMutex mutex( _mutex );

    if( version == VERSION_HEAD )
    {
        _syncToHead();
        return true;
    }

    if( isMaster( ) || _version > version )
        return false;

    while( _version < version )
    {
        eqNet::Node*   node;
        eqNet::Packet* packet;
        _syncQueue.pop( &node, &packet );

         // OPT shortcut around handleCommand()
        EQASSERT( packet->command == REQ_OBJECT_SYNC );
        _reqSync( node, packet );
    }

    EQVERB << "Sync'ed to version " << version << ", id " << getID() << endl;
    return true;
}

void Object::_syncToHead()
{
    if( isMaster( ))
        return;

    eqNet::Node*   node;
    eqNet::Packet* packet;

    while( _syncQueue.tryPop( &node, &packet ))
    {
        EQASSERT( packet->command == REQ_OBJECT_SYNC );
        _reqSync( node, packet ); // XXX shortcut around handleCommand()
    }
    EQVERB << "Sync'ed to head version " << _version << ", id " << getID() 
           << endl;
}

uint32_t Object::getHeadVersion() const
{
    if( isMaster( ))
        return _version;

    eqNet::Node*      node;
    ObjectSyncPacket* packet;

    if( !_syncQueue.back( &node, (Packet**)&packet ))
        return _version;
    
    EQASSERT( packet->command == REQ_OBJECT_SYNC );
    return packet->version;
}

void Object::addSlave( RefPtr<Node> slave )
{
    _slaves.push_back( slave ); 
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
void Object::_reqSync( Node* node, const Packet* pkg )
{
    ObjectSyncPacket* packet = (ObjectSyncPacket*)pkg;
    EQVERB << "req sync " << packet << endl;
    
    unpack( packet->delta, packet->deltaSize );
    _version = packet->version;
    //return eqNet::COMMAND_HANDLED;
}
