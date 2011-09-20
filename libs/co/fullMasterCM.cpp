
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "nodePackets.h"
#include "object.h"
#include "objectDataIStream.h"
#include "objectPackets.h"

//#define EQ_INSTRUMENT

namespace co
{
namespace
{
#ifdef EQ_INSTRUMENT
base::a_int32_t _bytesBuffered;
#endif
}

typedef CommandFunc<FullMasterCM> CmdFunc;

FullMasterCM::FullMasterCM( Object* object )
        : MasterCM( object )
        , _commitCount( 0 )
        , _nVersions( 0 )
{
    EQASSERT( object );
    EQASSERT( object->getLocalNode( ));
    CommandQueue* q = object->getLocalNode()->getCommandThreadQueue();

    object->registerCommand( CMD_OBJECT_COMMIT, 
                             CmdFunc( this, &FullMasterCM::_cmdCommit ), q );
    object->registerCommand( CMD_OBJECT_OBSOLETE, 
                             CmdFunc( this, &FullMasterCM::_cmdObsolete ), q );
}

FullMasterCM::~FullMasterCM()
{
    for( InstanceDataDeque::const_iterator i = _instanceDatas.begin();
         i != _instanceDatas.end(); ++i )
    {
        delete *i;
    }
    _instanceDatas.clear();

    for( InstanceDatas::const_iterator i = _instanceDataCache.begin();
         i != _instanceDataCache.end(); ++i )
    {
        delete *i;
    }
    _instanceDataCache.clear();
}

void FullMasterCM::sendInstanceData( Nodes& nodes )
{
    EQ_TS_THREAD( _cmdThread );
    if( !_slaves.empty( ))
       return;

    InstanceData* data = _instanceDatas.back();
    data->os.sendInstanceData( nodes );
}

void FullMasterCM::init()
{
    EQASSERT( _commitCount == 0 );
    MasterCM::init();

    InstanceData* data = _newInstanceData();

    data->os.enableCommit( VERSION_FIRST, _slaves );
    _object->getInstanceData( data->os );
    data->os.disable();
        
    _instanceDatas.push_back( data );
    ++_version;
    ++_commitCount;
}

void FullMasterCM::setAutoObsolete( const uint32_t count )
{
    ObjectObsoletePacket packet( count );
    packet.instanceID = _object->_instanceID;

    NodePtr localNode = _object->getLocalNode();
    _object->send( localNode, packet );    
}

void FullMasterCM::increaseCommitCount( const uint32_t incarnation )
{
    if( _nVersions == 0 )
        return; // OPT: obsoletion not needed

    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID = EQ_UNDEFINED_UINT32;
    packet.incarnation = incarnation;

    NodePtr localNode = _object->getLocalNode();
    _object->send( localNode, packet );    
}

void FullMasterCM::_updateCommitCount( const uint32_t incarnation )
{
    EQASSERT( !_instanceDatas.empty( ));
    if( incarnation == CO_COMMIT_NEXT )
    {
        ++_commitCount;
        return;
    }

    if( incarnation >= _commitCount )
    {
        _commitCount = incarnation;
        return;
    }

    _commitCount = incarnation;

    // obsolete 'future' old packages
    while( _instanceDatas.size() > 1 )
    {
        InstanceData* data = _instanceDatas.back();
        if( data->commitCount <= _commitCount )
            break;

#ifdef EQ_INSTRUMENT
        _bytesBuffered -= data->os.getSaveBuffer().getSize();
        EQINFO << _bytesBuffered << " bytes used" << std::endl;
#endif
        _releaseInstanceData( data );
        _instanceDatas.pop_back();
    }

    InstanceData* data = _instanceDatas.back();
    if( data->commitCount > _commitCount )
        // tweak commitCount of minimum retained version for correct obsoletion
        data->commitCount = 0;
}

void FullMasterCM::_obsolete()
{
    EQASSERT( !_instanceDatas.empty( ));
    while( _instanceDatas.size() > 1 && _commitCount > _nVersions )
    {
        InstanceData* data = _instanceDatas.front();
        if( data->commitCount >= (_commitCount - _nVersions))
            break;

#ifdef EQ_INSTRUMENT
        _bytesBuffered -= data->os.getSaveBuffer().getSize();
        EQINFO << _bytesBuffered << " bytes used" << std::endl;
#endif
#if 0
        EQINFO
            << "Remove v" << data->os.getVersion() << " c" << data->commitCount
            << "@" << _commitCount << "/" << _nVersions << " from "
            << base::className( _object ) << " " << ObjectVersion( _object )
            << std::endl;
#endif
        _releaseInstanceData( data );
        _instanceDatas.pop_front();
    }
    _checkConsistency();
}

void FullMasterCM::addSlave( Command& command, NodeMapObjectReplyPacket& reply )
{
    EQ_TS_THREAD( _cmdThread );
    EQASSERT( command->type == PACKETTYPE_CO_NODE );
    EQASSERT( command->command == CMD_NODE_MAP_OBJECT );

    NodePtr node = command.getNode();
    const NodeMapObjectPacket* packet = command.get< NodeMapObjectPacket >();
    const uint128_t requested  = packet->requestedVersion;
    const uint32_t instanceID = packet->instanceID;

    EQASSERT( _version != VERSION_NONE );
    _checkConsistency();

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    if( requested == VERSION_NONE ) // no data to send
    {
        _sendEmptyVersion( node, instanceID );
        reply.version = _version;
        return;
    }

    const uint128_t oldest = _instanceDatas.front()->os.getVersion();
    uint128_t start = (requested == VERSION_OLDEST || requested < oldest ) ?
                          oldest : requested;
    uint128_t end = _version;
    const bool useCache = packet->masterInstanceID == _object->getInstanceID();

#ifndef NDEBUG
    if( requested != VERSION_OLDEST && requested < start )
        EQINFO << "Mapping version " << start
               << " instead of requested version " << requested << std::endl;
#endif

    reply.version = start;
    reply.useCache = packet->useCache && useCache;

    if( reply.useCache )
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

#if 0
    EQLOG( LOG_OBJECTS )
        << *_object << ", instantiate on " << node->getNodeID() << " with v"
        << ((requested == VERSION_OLDEST) ? oldest : requested) << " ("
        << requested << ") sending " << start << ".." << end << " have "
        << _version - _nVersions << ".." << _version << " "
        << _instanceDatas.size() << std::endl;
#endif
    EQASSERT( start >= oldest );

    // send all instance datas from start..end
    InstanceDataDeque::iterator i = _instanceDatas.begin();
    while( i != _instanceDatas.end() && (*i)->os.getVersion() < start )
        ++i;

    for( ; i != _instanceDatas.end() && (*i)->os.getVersion() <= end; ++i )
    {
        InstanceData* data = *i;
        EQASSERT( data );
        data->os.sendMapData( node, instanceID );

#ifdef EQ_INSTRUMENT_MULTICAST
        ++_miss;
#endif
    }

#ifdef EQ_INSTRUMENT_MULTICAST
    if( _miss % 100 == 0 )
        EQINFO << "Cached " << _hit << "/" << _hit + _miss
               << " instance data transmissions" << std::endl;
#endif
}

void FullMasterCM::removeSlave( NodePtr node )
{
    EQ_TS_THREAD( _cmdThread );
    _checkConsistency();

    // remove from subscribers
    const NodeID& nodeID = node->getNodeID();
    EQASSERT( _slavesCount[ nodeID ] != 0 );

    if( --_slavesCount[ nodeID ] == 0 )
    {
        Nodes::iterator i = stde::find( _slaves, node );
        EQASSERT( i != _slaves.end( ));
        _slaves.erase( i );
        _slavesCount.erase( nodeID );
    }
}

void FullMasterCM::_checkConsistency() const
{
#ifndef NDEBUG
    EQASSERT( !_instanceDatas.empty( ));
    EQASSERT( _object->isAttached() );

    if( _version == VERSION_NONE )
        return;

    uint128_t version = _version;
    for( InstanceDataDeque::const_reverse_iterator i = _instanceDatas.rbegin();
         i != _instanceDatas.rend(); ++i )
    {
        const InstanceData* data = *i;
        EQASSERT( data->os.getVersion() != VERSION_NONE );
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
    EQ_TS_SCOPED( _cmdThread );
    InstanceData* instanceData;

    if( _instanceDataCache.empty( ))
        instanceData = new InstanceData( this );
    else
    {
        instanceData = _instanceDataCache.back();
        _instanceDataCache.pop_back();
    }

    instanceData->commitCount = _commitCount;
    instanceData->os.reset();
    instanceData->os.enableSave();
    return instanceData;
}

void FullMasterCM::_addInstanceData( InstanceData* data )
{
    EQ_TS_THREAD( _cmdThread );
    EQASSERT( data->os.getVersion() != VERSION_NONE );
    EQASSERT( data->os.getVersion() != VERSION_INVALID );

    _instanceDatas.push_back( data );
#ifdef EQ_INSTRUMENT
    _bytesBuffered += data->os.getSaveBuffer().getSize();
    EQINFO << _bytesBuffered << " bytes used" << std::endl;
#endif
}

void FullMasterCM::_releaseInstanceData( InstanceData* data )
{
    EQ_TS_THREAD( _cmdThread );
#ifdef CO_AGGRESSIVE_CACHING
    _instanceDataCache.push_back( data );
#else
    delete data;
#endif
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool FullMasterCM::_cmdCommit( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    const ObjectCommitPacket* packet = command.get< ObjectCommitPacket >();
#if 0
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command 
                         << std::endl;
#endif
    EQASSERT( _version != VERSION_NONE );

    _updateCommitCount( packet->incarnation );

    if( packet->requestID != EQ_UNDEFINED_UINT32 )
    {
        InstanceData* instanceData = _newInstanceData();

        instanceData->os.enableCommit( _version + 1, _slaves );
        _object->getInstanceData( instanceData->os );
        instanceData->os.disable();

        if( instanceData->os.hasSentData( ))
        {
            ++_version;
            EQASSERT( _version != VERSION_NONE );
#if 0
            EQINFO << "Committed v" << _version << "@" << _commitCount
                   << ", id " << _object->getID() << std::endl;
#endif
            _addInstanceData( instanceData );
        }
        else
            _instanceDataCache.push_back( instanceData );

        _object->getLocalNode()->serveRequest( packet->requestID, _version );
    }

    _obsolete();
    return true;
}

bool FullMasterCM::_cmdObsolete( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    const ObjectObsoletePacket* packet = command.get< ObjectObsoletePacket >();

    _nVersions = packet->count;
    _obsolete();
    return true;
}
}
