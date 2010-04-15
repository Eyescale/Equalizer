
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "fullMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "objectSlaveDataIStream.h"
#include "packets.h"
#include "session.h"

//#define EQ_INSTRUMENT

namespace eq
{
namespace net
{
namespace
{
#ifdef EQ_INSTRUMENT
base::a_int32_t _bytesBuffered;
#endif
}

typedef CommandFunc<FullMasterCM> CmdFunc;

FullMasterCM::FullMasterCM( Object* object )
        : MasterCM( object ),
          _commitCount( 0 ),
          _nVersions( 0 )
{
    InstanceData* data = _newInstanceData();
    data->os.setVersion( 1 );

    data->os.enable();
    _object->getInstanceData( data->os );
    data->os.disable();
        
    _instanceDatas.push_back( data );
    ++_version;
    ++_commitCount;

    registerCommand( CMD_OBJECT_COMMIT, 
                     CmdFunc( this, &FullMasterCM::_cmdCommit ), 0 );
}

FullMasterCM::~FullMasterCM()
{
    for( InstanceDataDeque::const_iterator i = _instanceDatas.begin();
         i != _instanceDatas.end(); ++i )
    {
        delete *i;
    }
    _instanceDatas.clear();

    for( InstanceDataVector::const_iterator i = _instanceDataCache.begin();
         i != _instanceDataCache.end(); ++i )
    {
        delete *i;
    }
    _instanceDataCache.clear();
}

void FullMasterCM::increaseCommitCount()
{
    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID = EQ_ID_INVALID;

    NodePtr localNode = _object->getLocalNode();
    _object->send( localNode, packet );    
}

void FullMasterCM::_obsolete()
{
    if( _instanceDatas.size() > 1 )
    {
        InstanceData* data = _instanceDatas[1];
        
        if( data->commitCount < (_commitCount - _nVersions))
        {
            data = _instanceDatas.front();
#ifdef EQ_INSTRUMENT
            _bytesBuffered -= data->os.getSaveBuffer().getSize();
            EQINFO << _bytesBuffered << " bytes used" << std::endl;
#endif
            EQLOG( LOG_OBJECTS )
                << "Remove v" << data->os.getVersion() << " c"
                << data->commitCount << "[" << _commitCount << "] from "
                << ObjectVersion( _object ) << std::endl;

            _instanceDataCache.push_back( data );
            _instanceDatas.pop_front();
        }
    }
    _checkConsistency();
}

uint32_t FullMasterCM::getOldestVersion() const
{
    if( _version == VERSION_NONE )
        return VERSION_NONE;

    return _instanceDatas.front()->os.getVersion();
}

uint32_t FullMasterCM::addSlave( Command& command )
{
    CHECK_THREAD( _cmdThread );
    EQASSERT( command->type == PACKETTYPE_EQNET_SESSION );
    EQASSERT( command->command == CMD_SESSION_SUBSCRIBE_OBJECT );

    NodePtr node = command.getNode();
    SessionSubscribeObjectPacket* packet =
        command.getPacket<SessionSubscribeObjectPacket>();
    const uint32_t requested = packet->requestedVersion;
    const uint32_t instanceID = packet->instanceID;

    EQASSERT( _version != VERSION_NONE );
    _checkConsistency();

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    if( requested == VERSION_NONE ) // no data to send
    {
        ObjectInstanceDataOStream stream( _object );
        stream.setInstanceID( instanceID );
        stream.setVersion( _version );
        stream.setNodeID( node->getNodeID( ));
        
        stream.resend( node );
        return VERSION_INVALID;
    }

    const uint32_t oldest = getOldestVersion();
    uint32_t start = (requested == VERSION_OLDEST) ? oldest : requested;
    uint32_t end   = _version;
    const bool useCache = packet->masterInstanceID == _object->getInstanceID();
    const uint32_t result = useCache ? start : VERSION_INVALID;

    if( packet->useCache && useCache )
    {
        if( packet->minCachedVersion <= start && 
            packet->maxCachedVersion >= start )
        {
#ifdef EQ_INSTRUMENT_MULTICAST
            _hit += packet->maxCachedVersion + 1 - start;
#endif
            start = packet->maxCachedVersion + 1;
        }
        else if( packet->maxCachedVersion == end )
        {
            end = EQ_MAX( start, packet->minCachedVersion - 1 );
#ifdef EQ_INSTRUMENT_MULTICAST
            _hit += _version - end;
#endif
        }
        // TODO else cached block in the middle, send head and tail elements
    }

    EQLOG( LOG_OBJECTS ) << "Object " << _object->_id << " (" 
                         << typeid( *_object).name() << ") v" << _version
                         << ", instantiate on " << node->getNodeID() 
                         << " with v" 
                         << ((requested == VERSION_OLDEST) ? oldest : requested)
                         << " (" << requested << ") sending " << start << ".."
                         << end << " have " << _version - _nVersions << ".."
                         << _version << " " << _instanceDatas.size()
                         << std::endl;

    EQASSERT( start >= oldest );

    // send all instance datas from start..end
    InstanceDataDeque::iterator i = _instanceDatas.begin();
    while( i != _instanceDatas.end() && (*i)->os.getVersion() < start )
        ++i;

    for( ; i != _instanceDatas.end() && (*i)->os.getVersion() <= end; ++i )
    {
        InstanceData* data = *i;
        EQASSERT( data );

        data->os.setInstanceID( instanceID );
        data->os.setNodeID( node->getNodeID( ));
        data->os.resend( node );
#ifdef EQ_INSTRUMENT_MULTICAST
        ++_miss;
#endif
    }

#ifdef EQ_INSTRUMENT_MULTICAST
    if( _miss % 100 == 0 )
        EQINFO << "Cached " << _hit << "/" << _hit + _miss
               << " instance data transmissions" << std::endl;
#endif
    return result;
}

void FullMasterCM::removeSlave( NodePtr node )
{
    CHECK_THREAD( _cmdThread );
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

    if( _version == VERSION_NONE )
        return;

    uint32_t version = _version;
    for( InstanceDataDeque::const_reverse_iterator i = _instanceDatas.rbegin();
         i != _instanceDatas.rend(); ++i )
    {
        const InstanceData* data = *i;
        EQASSERT( data->os.getVersion() > 0 );
        EQASSERTINFO( data->os.getVersion() == version,
                      data->os.getVersion() << " != " << version );
        if( data != _instanceDatas.front( ))
        {
            EQASSERTINFO( data->commitCount + _nVersions >= _commitCount,
                          data->commitCount << ", " << _commitCount << " [" <<
                          _nVersions << "]" );
        }
        --version;
    }
#endif
}

//---------------------------------------------------------------------------
// cache handling
//---------------------------------------------------------------------------
FullMasterCM::InstanceData* FullMasterCM::_newInstanceData()
{
    InstanceData* instanceData;

    if( _instanceDataCache.empty( ))
        instanceData = new InstanceData( _object );
    else
    {
        instanceData = _instanceDataCache.back();
        _instanceDataCache.pop_back();
    }

    instanceData->os.enableSave();
    instanceData->os.setInstanceID( EQ_ID_NONE );
    instanceData->os.setNodeID( NodeID::ZERO );
    return instanceData;
}

void FullMasterCM::_addInstanceData( InstanceData* data )
{
    _instanceDatas.push_back( data );
#ifdef EQ_INSTRUMENT
    _bytesBuffered += data->os.getSaveBuffer().getSize();
    EQINFO << _bytesBuffered << " bytes used" << std::endl;
#endif
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult FullMasterCM::_cmdCommit( Command& command )
{
    CHECK_THREAD( _cmdThread );
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command 
                         << std::endl;

    EQASSERT( _version != VERSION_NONE );

    ++_commitCount;

    if( packet->requestID != EQ_ID_INVALID )
    {
        InstanceData* instanceData = _newInstanceData();
        instanceData->commitCount = _commitCount;
        instanceData->os.setVersion( _version + 1 );

        instanceData->os.enable( _slaves );
        _object->getInstanceData( instanceData->os );
        instanceData->os.disable();

        if( instanceData->os.hasSentData( ))
        {
            ++_version;
            EQASSERT( _version );
            EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                                 << _object->getID() << std::endl;
        }
        _addInstanceData( instanceData );
        _object->getLocalNode()->serveRequest( packet->requestID, _version );
    }

    _obsolete();
    return COMMAND_HANDLED;
}

}
}
