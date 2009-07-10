
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

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
typedef CommandFunc<FullMasterCM> CmdFunc;

FullMasterCM::FullMasterCM( Object* object )
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _commitCount( 0 ),
          _nVersions( 0 ),
          _obsoleteFlags( Object::AUTO_OBSOLETE_COUNT_VERSIONS )
{
    DeltaData* data = _newDeltaData();
    data->os.setVersion( 1 );

    data->os.enable();
    _object->getInstanceData( data->os );
    data->os.disable();
        
    _deltaDatas.push_front( data );
    ++_version;
    ++_commitCount;

    registerCommand( CMD_OBJECT_COMMIT, 
                     CmdFunc( this, &FullMasterCM::_cmdCommit ), 0 );
    // sync commands are send to any instance, even the master gets the command
    registerCommand( CMD_OBJECT_DELTA_DATA,
                     CmdFunc( this, &FullMasterCM::_cmdDiscard ), 0 );
    registerCommand( CMD_OBJECT_DELTA,
                     CmdFunc( this, &FullMasterCM::_cmdDiscard ), 0 );
}

FullMasterCM::~FullMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject of "
               << typeid( *_object ).name() << std::endl;
    _slaves.clear();

    for( std::deque< DeltaData* >::const_iterator i = _deltaDatas.begin();
         i != _deltaDatas.end(); ++i )

        delete *i;

    _deltaDatas.clear();

    for(std::vector<DeltaData*>::const_iterator i=_deltaDataCache.begin();
         i != _deltaDataCache.end(); ++i )

        delete *i;
    
    _deltaDataCache.clear();
}

uint32_t FullMasterCM::commitNB()
{
    EQASSERTINFO( _object->getChangeType() == Object::INSTANCE,
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

// Obsoletes old changes based on number of commits or number of versions,
// depending on the obsolete flags.
void FullMasterCM::_obsolete()
{
    if( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS )
    {
        DeltaData* lastDeltaData = _deltaDatas.back();
        if( lastDeltaData->commitCount < (_commitCount - _nVersions) &&
            _deltaDatas.size() > 1 )
        {
            _deltaDataCache.push_back( lastDeltaData );
            _deltaDatas.pop_back();
        }
        _checkConsistency();
        return;
    }
    // else count versions
    while( _deltaDatas.size() > (_nVersions+1) )
    {
        _deltaDataCache.push_back( _deltaDatas.back( ));
        _deltaDatas.pop_back();
        _checkConsistency();
    }
}

uint32_t FullMasterCM::getOldestVersion() const
{
    if( _version == Object::VERSION_NONE )
        return Object::VERSION_NONE;

    return _deltaDatas.back()->os.getVersion();
}

void FullMasterCM::addSlave( NodePtr node, const uint32_t instanceID, 
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

    const uint32_t version = (inVersion == Object::VERSION_OLDEST) ?
                                 getOldestVersion() : inVersion;
    EQLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                         << ", instantiate on " << node->getNodeID() 
                         << " with v" << version << endl;

    EQASSERT( _object->getChangeType() == Object::INSTANCE );
    EQASSERT( version >= getOldestVersion( ));

    deque< DeltaData* >::reverse_iterator i = _deltaDatas.rbegin();
    while( (*i)->os.getVersion() < version && i != _deltaDatas.rend( ))
        ++i;

    const DeltaData* data = (i == _deltaDatas.rend()) ? _deltaDatas.back() : *i;
         
    // first packet has to be an instance packet, to be applied immediately
    const Bufferb&       buffer     = data->os.getSaveBuffer();
    ObjectInstancePacket instPacket;
    instPacket.instanceID = instanceID;
    instPacket.dataSize   = buffer.getSize();
    instPacket.version    = data->os.getVersion();
    instPacket.sequence   = 0;

    _object->send( node, instPacket, buffer.getData(), buffer.getSize() );

    if( i == _deltaDatas.rend( ))
        return;

    // versions oldest-1..newest are delta packets
    for( ++i; i != _deltaDatas.rend(); ++i )
    {
        DeltaData* deltaData = *i;
        deltaData->os.setInstanceID( instanceID );
        deltaData->os.resend( node );
        EQASSERT( ++instPacket.version == deltaData->os.getVersion( ));
    }
    EQASSERT( instPacket.version == _version );
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

void FullMasterCM::_checkConsistency() const
{
#ifndef NDEBUG
    EQASSERT( _object->_id != EQ_ID_INVALID );
    EQASSERT( _object->getChangeType() == Object::INSTANCE );

    if( _version == Object::VERSION_NONE )
        return;

    if( !( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS ))
    {   // count versions
        if( _version <= _nVersions )
            EQASSERT( _deltaDatas.size() == _version );
    }
#endif
}

//---------------------------------------------------------------------------
// cache handling
//---------------------------------------------------------------------------
FullMasterCM::DeltaData* FullMasterCM::_newDeltaData()
{
    DeltaData* deltaData;

    if( _deltaDataCache.empty( ))
        deltaData = new DeltaData( _object );
    else
    {
        deltaData = _deltaDataCache.back();
        _deltaDataCache.pop_back();
    }

    deltaData->os.enableSave();
    deltaData->os.enableBuffering();
    deltaData->os.setInstanceID( EQ_ID_ANY );
    return deltaData;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult FullMasterCM::_cmdCommit( Command& command )
{
    CHECK_THREAD( _thread );
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command << endl;

    EQASSERT( _version != Object::VERSION_NONE );

    ++_commitCount;

    DeltaData* deltaData = _newDeltaData();

    deltaData->commitCount = _commitCount;
    deltaData->os.setVersion( _version + 1 );

    deltaData->os.enable( _slaves );
    _object->pack( deltaData->os );
    deltaData->os.disable();

    if( deltaData->os.hasSentData( ))
    {
        ++_version;
        EQASSERT( _version );
    
        _deltaDatas.push_front( deltaData );
        EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                             << _object->getID() << endl;
    }
    else
        _deltaDataCache.push_back( deltaData );

    _obsolete();
    _checkConsistency();
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}

}
}
