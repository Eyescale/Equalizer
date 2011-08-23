
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "masterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectPackets.h"
#include "objectDataIStream.h"

namespace co
{
typedef CommandFunc<MasterCM> CmdFunc;

MasterCM::MasterCM( Object* object )
        : ObjectCM( object )
        , _version( VERSION_NONE )
{
    EQASSERT( object );
    EQASSERT( object->getLocalNode( ));

    // sync commands are send to all instances, even the master gets it
    object->registerCommand( CMD_OBJECT_INSTANCE,
                             CmdFunc( this, &MasterCM::_cmdDiscard ), 0 );
    object->registerCommand( CMD_OBJECT_SLAVE_DELTA,
                             CmdFunc( this, &MasterCM::_cmdSlaveDelta ), 0 );
}

MasterCM::~MasterCM()
{
    EQASSERTINFO( _pendingDeltas.empty(), "Incomplete slave commits pending" );
    EQASSERTINFO( _queuedDeltas.isEmpty(), _queuedDeltas.getSize() <<
                  " unapplied slave commits on " << typeid( *_object ).name( ));

    for( PendingStreams::const_iterator i = _pendingDeltas.begin();
         i != _pendingDeltas.end(); ++i )
    {
        delete i->second;
    }
    _pendingDeltas.clear();

    ObjectDataIStream* is = 0;
    while( _queuedDeltas.tryPop( is ))
        delete is;
    _slaves.clear();
}

uint32_t MasterCM::commitNB( const uint32_t incarnation )
{
    LocalNodePtr localNode = _object->getLocalNode();
    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = localNode->registerRequest();
    packet.incarnation = incarnation;

    _object->send( localNode, packet );
    return packet.requestID;
}

uint128_t MasterCM::commitSync( const uint32_t commitID )
{
    LocalNodePtr localNode = _object->getLocalNode();
    uint128_t version = VERSION_NONE;
    localNode->waitRequest( commitID, version );
    return version;
}

uint128_t MasterCM::sync( const uint128_t& version )
{
    EQASSERTINFO( version.high() != 0 || version == VERSION_NEXT ||
                  version == VERSION_HEAD, version );
#if 0
    EQLOG( LOG_OBJECTS ) << "sync to v" << version << ", id " 
                         << _object->getID() << "." << _object->getInstanceID()
                         << std::endl;
#endif

    if( version == VERSION_NEXT )
    {
        ObjectDataIStream* is = _queuedDeltas.pop();
        _apply( is );
        return _version;
    }
    // else
    if( version == VERSION_HEAD )
    {
        ObjectDataIStream* is = 0;
        while( _queuedDeltas.tryPop( is ))
            _apply( is );
        return _version;
    }
    // else apply only concrete slave commit

    ObjectDataIStream* is = 0;
    ObjectDataIStreams unusedStreams;
    while( !is )
    {
        ObjectDataIStream* candidate = _queuedDeltas.pop();
        if( candidate->getVersion() == version )
            is = candidate;
        else
            unusedStreams.push_back( candidate );
    }

    _apply( is );
    _queuedDeltas.pushFront( unusedStreams );
    return version;
}

void MasterCM::_apply( ObjectDataIStream* is )
{
    EQASSERT( !is->hasInstanceData( ));
    _object->unpack( *is );
    EQASSERTINFO( is->getRemainingBufferSize() == 0 && 
                  is->nRemainingBuffers()==0,
                  "Object " << base::className( _object ) <<
                  " did not unpack all data" );
    is->reset();
    _iStreamCache.release( is );
}

void MasterCM::_sendEmptyVersion( NodePtr node, const uint32_t instanceID )
{
    ObjectInstancePacket instancePacket;
    instancePacket.type = PACKETTYPE_CO_OBJECT;
    instancePacket.command = CMD_OBJECT_INSTANCE;
    instancePacket.last = true;
    instancePacket.version = _version;
    instancePacket.instanceID = instanceID;
    instancePacket.masterInstanceID = _object->getInstanceID();
    _object->send( node, instancePacket );
}

void MasterCM::removeSlaves( NodePtr node )
{
    EQ_TS_THREAD( _cmdThread );

    const NodeID& nodeID = node->getNodeID();
    SlavesCount::iterator i = _slavesCount.find( nodeID );
    if( i == _slavesCount.end( ))
        return;

    Nodes::iterator j = stde::find( _slaves, node );
    EQASSERT( j != _slaves.end( ));
    _slaves.erase( j );
    _slavesCount.erase( i );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool MasterCM::_cmdSlaveDelta( Command& command )
{
    EQ_TS_THREAD( _rcvThread );
    const ObjectSlaveDeltaPacket* packet = 
        command.get< ObjectSlaveDeltaPacket >();

    EQASSERTINFO( _pendingDeltas.size() < 100, "More than 100 pending commits");

    ObjectDataIStream* istream = 0;
    PendingStreams::iterator i = _pendingDeltas.find( packet->commit );
    if( i == _pendingDeltas.end( ))
        istream = _iStreamCache.alloc();
    else
        istream = i->second;

    istream->addDataPacket( command );
    if( istream->isReady( ))
    {
        if( i != _pendingDeltas.end( ))
            _pendingDeltas.erase( i );

        _queuedDeltas.push( istream );
        _object->notifyNewVersion();
        EQASSERTINFO( _queuedDeltas.getSize() < 100,
                      "More than 100 queued commits" );
#if 0
        EQLOG( LOG_OBJECTS )
            << "Queued slave commit " << packet->commit << " object "
            << _object->getID() << " " << base::className( _object )
            << std::endl;
#endif
    }
    else if( i == _pendingDeltas.end( ))
    {
        _pendingDeltas[ packet->commit ] = istream;
#if 0
        EQLOG( LOG_OBJECTS )
            << "New incomplete slave commit " << packet->commit << " object "
            << _object->getID() << " " << base::className( _object )
            << std::endl;
#endif
    }
#if 0
    else
        EQLOG( LOG_OBJECTS )
            << "Got data for incomplete slave commit " << packet->commit
            << " object " << _object->getID() << " "
            << base::className( _object ) << std::endl;
#endif

    return true;
}

}
