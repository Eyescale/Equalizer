
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

#include "deltaMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "objectDeltaDataOStream.h"
#include "packets.h"
#include "session.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{

typedef ObjectDeltaDataOStream DeltaData;
        
DeltaMasterCM::DeltaMasterCM( Object* object )
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _commitCount( 0 ),
          _nVersions( 0 ),
          _obsoleteFlags( Object::AUTO_OBSOLETE_COUNT_VERSIONS )
{
    InstanceData* data = _newInstanceData();
    data->os.setVersion( 1 );
    data->os.enableSave();

    data->os.enable();
    _object->getInstanceData( data->os );
    data->os.disable();
        
    _instanceDatas.push_front( data );
    ++_version;
    ++_commitCount;

    registerCommand( CMD_OBJECT_COMMIT, 
                 CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdCommit ),
                     0 );
    // sync commands are send to all instances, even the master gets it
    registerCommand( CMD_OBJECT_DELTA_DATA, 
                CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdDiscard ),
                     0 );
    registerCommand( CMD_OBJECT_DELTA, 
                CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdDiscard ),
                     0 );
}

DeltaMasterCM::~DeltaMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject" << endl;
    _slaves.clear();

    for( std::deque< InstanceData* >::const_iterator i = _instanceDatas.begin();
         i != _instanceDatas.end(); ++i )

        delete *i;

    _instanceDatas.clear();

    for(std::vector<InstanceData*>::const_iterator i=_instanceDataCache.begin();
         i != _instanceDataCache.end(); ++i )

        delete *i;
    
    _instanceDataCache.clear();
}

uint32_t DeltaMasterCM::commitNB()
{
    EQASSERTINFO( _object->getChangeType() == Object::DELTA,
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

// Obsoletes old changes based on number of commits or number of versions,
// depending on the obsolete flags.
void DeltaMasterCM::_obsolete()
{
    if( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS )
    {
        InstanceData* lastInstanceData = _instanceDatas.back();
        if( lastInstanceData->commitCount < (_commitCount - _nVersions) &&
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

uint32_t DeltaMasterCM::getOldestVersion() const
{
    if( _version == Object::VERSION_NONE )
        return Object::VERSION_NONE;

    return _instanceDatas.back()->os.getVersion();
}

void DeltaMasterCM::addSlave( NodePtr node, const uint32_t instanceID,
                              const uint32_t inVersion )
{
    CHECK_THREAD( _thread );
    EQASSERT( _version != Object::VERSION_NONE );
    _checkConsistency();

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    if( inVersion == Object::VERSION_NONE ) // no data to send
    {
        ObjectInstancePacket instPacket;
        instPacket.instanceID = instanceID;
        instPacket.dataSize   = 0;
        instPacket.version    = _version;
        instPacket.sequence   = 0;

        _object->send( node, instPacket );
        return;
    }

    const uint32_t oldest  = getOldestVersion();
    const uint32_t version = (inVersion == Object::VERSION_OLDEST) ?
                                 oldest : inVersion;
    EQLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                         << ", instantiate on " << node->getNodeID() 
                         << " with v" << version << endl;

    EQASSERT( version >= oldest );

    // send all instance datas, starting at initial version
    deque< InstanceData* >::reverse_iterator i = _instanceDatas.rbegin();
    while( (*i)->os.getVersion() < version && i != _instanceDatas.rend( ))
    {
        ++i;
    }

    if( i == _instanceDatas.rend( ))
    {
        InstanceData* data = _instanceDatas.back();
        EQASSERT( data );
        EQASSERT( data->os.getVersion() <= version );

        data->os.setInstanceID( instanceID );
        data->os.resend( node );
    }
    else for( ; i != _instanceDatas.rend(); ++i )
    {
        InstanceData* data = *i;
        EQASSERT( data );
        EQASSERTINFO( data->os.getVersion() >= version, 
                      data->os.getVersion() << " < " << version );

        data->os.setInstanceID( instanceID );
        data->os.resend( node );
    }
}

void DeltaMasterCM::removeSlave( NodePtr node )
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

void DeltaMasterCM::addOldMaster( NodePtr node, const uint32_t instanceID )
{
    EQASSERT( _version != Object::VERSION_NONE );

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    ObjectVersionPacket packet;
    packet.instanceID = instanceID;
    packet.version    = _version;
    _object->send( node, packet );
}

void DeltaMasterCM::_checkConsistency() const
{
#ifndef NDEBUG
    EQASSERT( _object->_id != EQ_ID_INVALID );
    EQASSERT( _object->getChangeType() == Object::DELTA );
    if( _version == Object::VERSION_NONE )
        return;

    uint32_t version = _version;
    for( deque< InstanceData* >::const_iterator i = _instanceDatas.begin();
         i != _instanceDatas.end(); ++i )
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
DeltaMasterCM::InstanceData* DeltaMasterCM::_newInstanceData()
{
    InstanceData* instanceData;

    if( _instanceDataCache.empty( ))
        instanceData = new InstanceData( _object );
    else
    {
        instanceData = _instanceDataCache.back();
        _instanceDataCache.pop_back();
    }

    instanceData->os.disableSave();
    instanceData->os.enableBuffering();
    instanceData->os.setInstanceID( EQ_ID_ANY );
    return instanceData;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult DeltaMasterCM::_cmdCommit( Command& command )
{
    CHECK_THREAD( _thread );
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command << endl;

    EQASSERT( _version != Object::VERSION_NONE );

    ++_commitCount;

    DeltaData deltaData( _object );

    deltaData.setVersion( _version + 1 );
    deltaData.enable( _slaves );
    _object->pack( deltaData );
    deltaData.disable();

    if( !deltaData.hasSentData( ))
    {
        _obsolete();
        _checkConsistency();

        _requestHandler.serveRequest( packet->requestID, _version );
        return COMMAND_HANDLED;
    }

    ++_version;
    EQASSERT( _version );
    
    // save instance data
    InstanceData* instanceData = _newInstanceData();
    instanceData->os.enableSave();
    instanceData->os.setVersion( _version );

    instanceData->os.enable();
    _object->getInstanceData( instanceData->os );
    instanceData->os.disable();

    instanceData->commitCount = _commitCount;
    _instanceDatas.push_front( instanceData );
    
    _obsolete();
    _checkConsistency();

    EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                         << _object->getID() << endl;
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
}
}
