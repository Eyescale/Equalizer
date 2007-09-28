
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
}

FullMasterCM::~FullMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject" << endl;

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
    if( ptr && size > 0 )
        data.buffer.replace( ptr, size );

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

const void* FullMasterCM::getInitialData( uint64_t* size, uint32_t* version )
{
    if( _version == Object::VERSION_NONE )
        _commitInitial();

    const uint32_t      age  = _instanceDatas.size() - 1;
    const InstanceData& data = _instanceDatas.back();

    *version = _version - age;
    *size    = data.buffer.size;
    return data.buffer.data;
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

    const uint32_t           age        = _instanceDatas.size() - 1;
    ObjectInstanceDataPacket initPacket;
    initPacket.instanceID = instanceID;
    initPacket.dataSize   = 0;
    initPacket.version    = _version - age + 1;

    // send versions oldest-1..newest
    deque<InstanceData>::reverse_iterator i = _instanceDatas.rbegin();
    ++i; // oldest was sent by session using getInitialData()
    for( ; i != _instanceDatas.rend(); ++i )
    {
        const InstanceData& data = *i;

        initPacket.dataSize = data.buffer.size;
        
        EQLOG( LOG_OBJECTS ) << "send " << &initPacket << endl;
        _object->send( node, initPacket, data.buffer.data, data.buffer.size );
        ++initPacket.version;
    }
    EQASSERT( initPacket.version - 1 == _version );
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
    ++_version;
    EQASSERT( _version );

    // save instance data
    uint64_t instanceDataSize = 0;
    const void* ptr = _object->getInstanceData( &instanceDataSize );

    InstanceData instanceData;
    if( !_instanceDataCache.empty( ))
    {
        instanceData = _instanceDataCache.back();
        _instanceDataCache.pop_back();
    }

    if( ptr && instanceDataSize > 0 )
        instanceData.buffer.replace( ptr, instanceDataSize );
    else
        instanceData.buffer.size = 0;

    instanceData.commitCount = _commitCount;
    _instanceDatas.push_front( instanceData );
    
    _obsolete();

    // send new version to subscribed slaves
    if( !_slaves.empty( ))
    {
        ObjectInstanceDataPacket initPacket;

        initPacket.dataSize = instanceDataSize;
        initPacket.version  = _version;
        
        EQLOG( LOG_OBJECTS ) << "send " << &initPacket << " to " 
                             << _slaves.size() << " nodes " << endl;
        _object->send( _slaves, initPacket, ptr, instanceDataSize );
    }

    _object->releaseInstanceData( ptr );

    EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                         << _object->getID() << endl;

    _checkConsistency();
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
