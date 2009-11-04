
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

#include "fullMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"
#include "session.h"

namespace eq
{
namespace net
{
typedef CommandFunc<FullMasterCM> CmdFunc;

FullMasterCM::FullMasterCM( Object* object )
        : _object( object ),
          _version( VERSION_NONE ),
          _commitCount( 0 ),
          _nVersions( 0 ),
          _obsoleteFlags( Object::AUTO_OBSOLETE_COUNT_VERSIONS )
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
    // sync commands are send to all instances, even the master gets it
    registerCommand( CMD_OBJECT_INSTANCE_DATA,
                     CmdFunc( this, &FullMasterCM::_cmdDiscard ), 0 );
    registerCommand( CMD_OBJECT_INSTANCE,
                     CmdFunc( this, &FullMasterCM::_cmdDiscard ), 0 );
}

FullMasterCM::~FullMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject of "
               << typeid( *_object ).name() << std::endl;
    _slaves.clear();

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

uint32_t FullMasterCM::commitNB()
{
    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = _requestHandler.registerRequest();

    _object->send( _object->getLocalNode(), packet );
    return packet.requestID;
}

uint32_t FullMasterCM::commitSync( const uint32_t commitID )
{
    uint32_t version = VERSION_NONE;
    _requestHandler.waitRequest( commitID, version );
    return version;
}

// Obsoletes old changes based on number of commits or number of versions,
// depending on the obsolete flags.
void FullMasterCM::_obsolete()
{
    if( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS )
    {
        InstanceData* lastInstanceData = _instanceDatas.front();
        if( lastInstanceData->commitCount < (_commitCount - _nVersions) &&
            _instanceDatas.size() > 1 )
        {
            _instanceDataCache.push_back( lastInstanceData );
            _instanceDatas.pop_front();
        }
        _checkConsistency();
        return;
    }
    // else count versions
    while( _instanceDatas.size() > (_nVersions+1) )
    {
        _instanceDataCache.push_back( _instanceDatas.front( ));
        _instanceDatas.pop_front();
        _checkConsistency();
    }
}

uint32_t FullMasterCM::getOldestVersion() const
{
    if( _version == VERSION_NONE )
        return VERSION_NONE;

    return _instanceDatas.front()->os.getVersion();
}

uint32_t FullMasterCM::addSlave( Command& command )
{
    CHECK_THREAD( _thread );
    EQASSERT( command->datatype == DATATYPE_EQNET_SESSION );
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

    const uint32_t oldest  = getOldestVersion();
    uint32_t start = (requested == VERSION_OLDEST) ? oldest : requested;
    uint32_t end   = _version;
    const uint32_t result = start;

    if( packet->useCache )
    {
        if( packet->minCachedVersion <= start && 
            packet->maxCachedVersion >= start )
        {
            start = packet->maxCachedVersion + 1;
        }
        else if( packet->maxCachedVersion == end )
        {
            end = EQ_MAX( start, packet->minCachedVersion - 1 );
        }
        // TODO else cached block in the middle, send head and tail elements
    }

    EQLOG( LOG_OBJECTS ) << "Object " << _object->_id << " v" << _version
                         << ", instantiate on " << node->getNodeID() 
                         << " with v" << result << " sending v" << start
                         << ".." << end << std::endl;

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
    }

    return result;
}

void FullMasterCM::removeSlave( NodePtr node )
{
    CHECK_THREAD( _thread );
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

void FullMasterCM::addOldMaster( NodePtr node, const uint32_t instanceID )
{
    EQASSERT( _version != VERSION_NONE );

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    ObjectVersionPacket packet;
    packet.instanceID = instanceID;
    packet.version    = _version;
    _object->send( node, packet );
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
        EQASSERT( data->os.getVersion() == version );
        EQASSERT( data->os.getVersion() > 0 );
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
    instanceData->os.enableBuffering();
    instanceData->os.setInstanceID( EQ_ID_NONE );
    instanceData->os.setNodeID( NodeID::ZERO );
    return instanceData;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult FullMasterCM::_cmdCommit( Command& command )
{
    CHECK_THREAD( _thread );
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command 
                         << std::endl;

    EQASSERT( _version != VERSION_NONE );

    ++_commitCount;

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
    
        _instanceDatas.push_back( instanceData );
        EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                             << _object->getID() << std::endl;
    }
    else
        _instanceDataCache.push_back( instanceData );

    _obsolete();
    _checkConsistency();
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}

}
}
