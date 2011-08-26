
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
        _apply( _slaveCommits.pop( ));
        return _version;
    }
    // else
    if( version == VERSION_HEAD )
    {
        for( ObjectDataIStream* is = _slaveCommits.tryPop(); is;
             is = _slaveCommits.tryPop( ))
        {
            _apply( is );
        }
        return _version;
    }
    // else apply only concrete slave commit

    _apply( _slaveCommits.pull( version ));
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
    _slaveCommits.recycle( is );
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

    if( _slaveCommits.addDataPacket( packet->commit, command ))
        _object->notifyNewVersion();
    return true;
}

}
