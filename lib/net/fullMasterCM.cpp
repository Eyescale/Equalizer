
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "fullMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

FullMasterCM::FullMasterCM( Object* object )
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _commitCount( 0 ),
          _nVersions( 0 ),
          _obsoleteFlags( Object::AUTO_OBSOLETE_COUNT_VERSIONS )
{
    registerCommand( CMD_OBJECT_COMMIT, 
                  CommandFunc<FullMasterCM>( this, &FullMasterCM::_cmdCommit ));
    // sync commands are send to any instance, even the master gets the command
    registerCommand( CMD_OBJECT_INSTANCE_DATA,
                 CommandFunc<FullMasterCM>( this, &FullMasterCM::_cmdDiscard ));
    registerCommand( CMD_OBJECT_DELTA_DATA,
                 CommandFunc<FullMasterCM>( this, &FullMasterCM::_cmdDiscard ));
}

FullMasterCM::~FullMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject" << endl;

    for( deque<InstanceData>::const_iterator iter = _instanceDatas.begin(); 
         iter != _instanceDatas.end(); ++iter )
    {
        if( (*iter).data )
            free( (*iter).data );
    }
    _instanceDatas.clear();
}

uint32_t FullMasterCM::commitNB()
{
    EQASSERTINFO( !_object->isStatic( ), 
                  "Object type " << typeid(*this).name( ));

    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = _requestHandler.registerRequest();

    _object->send( _object->getLocalNode(), packet );
    return packet.requestID;
}

uint32_t FullMasterCM::commitSync( const uint32_t commitID )
{
    uint32_t version = Object::VERSION_NONE;
    _requestHandler.waitRequest( commitID, version );
    return version;
}

uint32_t FullMasterCM::_commitInitial()
{
    CHECK_THREAD( _thread );

    // compute base version
    uint64_t    size;
    const void* ptr  = _object->getInstanceData( &size );

    _setInitialVersion( ptr, size );
    _object->releaseInstanceData( ptr );
        
    return _version;
}

void FullMasterCM::_setInitialVersion( const void* ptr, const uint64_t size )
{
    EQASSERT( _version == Object::VERSION_NONE );
    EQASSERT( _instanceDatas.empty( ));

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
void FullMasterCM::_obsolete()
{
    if( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS )
    {
        InstanceData& lastInstanceData = _instanceDatas.back();
        if( lastInstanceData.commitCount < (_commitCount - _nVersions) &&
            _instanceDatas.size() > 1 )
        {
            _instanceDataCache.push_back( lastInstanceData );
            _instanceDatas.pop_back();
        }
        _checkConsistency();
        return;
    }
    // else count versions
    while( _instanceDatas.size() > (_nVersions+1) )
    {
        _instanceDataCache.push_back( _instanceDatas.back( ));
        _instanceDatas.pop_back();
        _checkConsistency();
    }
}

void FullMasterCM::addSlave( RefPtr<Node> node, const uint32_t instanceID )
{
    _checkConsistency();

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    EQLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                         << ", instanciate on " << node->getNodeID() << endl;

    EQASSERT( !_object->isStatic( ))

    if( _version == Object::VERSION_NONE )
        _commitInitial();

    const uint32_t                        age = _instanceDatas.size() - 1;
    deque<InstanceData>::reverse_iterator i   = _instanceDatas.rbegin();
         
    ObjectInstanceDataPacket instPacket;
    const InstanceData&      data      = *i;
    instPacket.instanceID = instanceID;
    instPacket.dataSize   = data.size;
    instPacket.version    = _version - age;

    _object->send( node, instPacket, data.data, data.size );

    // send versions oldest-1..newest
    for( ++i; i != _instanceDatas.rend(); ++i )
    {
        const InstanceData& data = *i;

        ObjectDeltaDataPacket deltaPacket;
        deltaPacket.instanceID = instanceID;
        deltaPacket.version    = ++instPacket.version;
        deltaPacket.deltaSize  = data.size;
        EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << endl;
        _object->send( node, deltaPacket, data.data, data.size );
    }
    EQASSERT( instPacket.version == _version );
}

void FullMasterCM::removeSlave( RefPtr<Node> node )
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

void FullMasterCM::_checkConsistency() const
{
#ifndef NDEBUG
    EQASSERT( _object->_id != EQ_ID_INVALID );

    if( _object->isStatic() || _version == Object::VERSION_NONE )
        return;

    if( !( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS ))
    {   // count versions
        if( _version <= _nVersions )
            EQASSERT( _instanceDatas.size() == _version );
    }
#endif
}
//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult FullMasterCM::_cmdCommit( Command& command )
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

    ++_commitCount;

    // save new version of instance data
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
    
    ++_version;
    EQASSERT( _version );
    
    _obsolete();

    // send new version to subscribed slaves
    if( !_slaves.empty( ))
    {
        ObjectDeltaDataPacket initPacket;

        initPacket.deltaSize = instanceData.size;
        initPacket.version  = _version;
        
        EQLOG( LOG_OBJECTS ) << "send " << &initPacket << " to " 
                             << _slaves.size() << " nodes " << endl;
        _object->send( _slaves, initPacket, instanceData.data, 
                       instanceData.size );
    }

    EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                         << _object->getID() << endl;

    _checkConsistency();
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
