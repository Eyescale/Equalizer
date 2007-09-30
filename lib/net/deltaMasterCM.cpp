
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "deltaMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

DeltaMasterCM::DeltaMasterCM( Object* object )
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _commitCount( 0 ),
          _nVersions( 0 ),
          _obsoleteFlags( Object::AUTO_OBSOLETE_COUNT_VERSIONS )
{
    registerCommand( CMD_OBJECT_COMMIT, 
                CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdCommit ));
    // sync commands are send to any instance, even the master gets the command
    registerCommand( CMD_OBJECT_DELTA_DATA, 
               CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdDiscard ));
}

DeltaMasterCM::~DeltaMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject" << endl;

    for( deque<InstanceData>::const_iterator i = _instanceDatas.begin(); 
         i != _instanceDatas.end(); ++i )
    {
        if( (*i).data )
            free( (*i).data );
    }
    _instanceDatas.clear();

    for( deque<ChangeData>::const_iterator i = _changeDatas.begin(); 
         i != _changeDatas.end(); ++i )
    {
        if( (*i).data )
            free( (*i).data );
    }
    _changeDatas.clear();
}

uint32_t DeltaMasterCM::commitNB()
{
    EQASSERTINFO( !_object->isStatic( ), 
                  "Object type " << typeid(*this).name( ));

    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = _requestHandler.registerRequest();

    _object->send( _object->getLocalNode(), packet );
    return packet.requestID;
}

uint32_t DeltaMasterCM::commitSync( const uint32_t commitID )
{
    uint32_t version = Object::VERSION_NONE;
    _requestHandler.waitRequest( commitID, version );
    return version;
}

uint32_t DeltaMasterCM::_commitInitial()
{
    CHECK_THREAD( _thread );

    // compute base version
    uint64_t    size;
    const void* ptr  = _object->getInstanceData( &size );

    _setInitialVersion( ptr, size );
    _object->releaseInstanceData( ptr );
        
    return _version;
}

void DeltaMasterCM::_setInitialVersion( const void* ptr, const uint64_t size )
{
    EQASSERT( _version == Object::VERSION_NONE );
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
void DeltaMasterCM::_obsolete()
{
    if( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS )
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

const void* DeltaMasterCM::_getInitialData( uint64_t* size, uint32_t* version )
{
    if( _version == Object::VERSION_NONE )
        _commitInitial();

    const uint32_t      age  = _instanceDatas.size() - 1;
    const InstanceData& data = _instanceDatas.back();

    *version = _version - age;
    *size    = data.size;
    return data.data;
}

void DeltaMasterCM::addSlave( RefPtr<Node> node, const uint32_t instanceID )
{
    _checkConsistency();

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    EQLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                         << ", instanciate on " << node->getNodeID() << endl;

    // send initial instance data
    ObjectInstanceDataPacket instPacket;
    instPacket.instanceID = instanceID;

    const void* data = _getInitialData( &instPacket.dataSize, 
                                        &instPacket.version );
    _object->send( node, instPacket, data, instPacket.dataSize );

    // send all deltas
    for( deque<ChangeData>::reverse_iterator i = _changeDatas.rbegin();
         i != _changeDatas.rend(); ++i )
    {
        const ChangeData& data = *i;

        ObjectDeltaDataPacket deltaPacket;
        deltaPacket.instanceID = instanceID;
        deltaPacket.version   = data.version;
        deltaPacket.deltaSize = data.size;
        
        EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << endl;
        _object->send( node, deltaPacket, data.data, data.size );
    }
}

void DeltaMasterCM::removeSlave( RefPtr<Node> node )
{
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

void DeltaMasterCM::_checkConsistency() const
{
#ifndef NDEBUG
    EQASSERT( _object->_id != EQ_ID_INVALID );
    EQASSERT( !_object->isStatic( ));
    if( _version == Object::VERSION_NONE )
        return;

    EQASSERT( _instanceDatas.size() == _changeDatas.size() + 1 );

    if( !( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS ))
    {   // count versions
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
CommandResult DeltaMasterCM::_cmdCommit( Command& command )
{
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command << endl;

    if( _version == Object::VERSION_NONE )
    {
        _commitInitial();
        EQASSERT( _slaves.empty( ));
        _checkConsistency();

        _requestHandler.serveRequest( packet->requestID, _version );
        return COMMAND_HANDLED;
    }

    EQASSERT( _instanceDatas.size() == _changeDatas.size() + 1 );

    ++_commitCount;

    uint64_t    deltaSize = 0;
    const void* delta     = _object->pack( &deltaSize );

    if( !delta || deltaSize == 0 )
    {
        _obsolete();
        _checkConsistency();

        _requestHandler.serveRequest( packet->requestID, _version );
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

    instanceData.size = 0;
    const void* ptr = _object->getInstanceData( &instanceData.size );

    if( instanceData.size > instanceData.maxSize )
    {
        if( instanceData.data )
            free( instanceData.data );
        instanceData.data    = malloc( instanceData.size );
        instanceData.maxSize = instanceData.size;
    }
    if( ptr )
        memcpy( instanceData.data, ptr, instanceData.size );
    _object->releaseInstanceData( ptr );

    instanceData.commitCount = _commitCount;
    _instanceDatas.push_front( instanceData );
    
    _obsolete();

    // send delta to subscribed slaves
    if( !_slaves.empty( ))
    {
        ObjectDeltaDataPacket deltaPacket;

        deltaPacket.version   = _version;
        deltaPacket.deltaSize = deltaSize;
        
        EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << " to " 
                             << _slaves.size() << " nodes " << endl;
        _object->send( _slaves, deltaPacket, delta, deltaSize );
    }

    _object->releasePackData( delta );
    EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                         << _object->getID() << endl;

    _checkConsistency();
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
