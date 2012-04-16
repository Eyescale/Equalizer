
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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
    LBASSERT( object );
    LBASSERT( object->getLocalNode( ));

    // sync commands are send to all instances, even the master gets it
    object->registerCommand( CMD_OBJECT_INSTANCE,
                             CmdFunc( this, &MasterCM::_cmdDiscard ), 0 );
    object->registerCommand( CMD_OBJECT_DELTA,
                             CmdFunc( this, &MasterCM::_cmdDiscard ), 0 );

    object->registerCommand( CMD_OBJECT_SLAVE_DELTA,
                             CmdFunc( this, &MasterCM::_cmdSlaveDelta ), 0 );
}

MasterCM::~MasterCM()
{
    _slaves->clear();
}

uint128_t MasterCM::sync( const uint128_t& inVersion )
{
    LBASSERTINFO( inVersion.high() != 0 || inVersion == VERSION_NEXT ||
                  inVersion == VERSION_HEAD, inVersion );
#if 0
    LBLOG( LOG_OBJECTS ) << "sync to v" << inVersion << ", id " 
                         << _object->getID() << "." << _object->getInstanceID()
                         << std::endl;
#endif

    if( inVersion == VERSION_NEXT )
        return _apply( _slaveCommits.pop( ));

    if( inVersion == VERSION_HEAD )
    {
        uint128_t version = VERSION_NONE;
        for( ObjectDataIStream* is = _slaveCommits.tryPop(); is;
             is = _slaveCommits.tryPop( ))
        {
            version = _apply( is );
        }
        return version;
    }
    // else apply only concrete slave commit

    return _apply( _slaveCommits.pull( inVersion ));
}

uint128_t MasterCM::_apply( ObjectDataIStream* is )
{
    LBASSERT( !is->hasInstanceData( ));
    _object->unpack( *is );
    LBASSERTINFO( is->getRemainingBufferSize() == 0 && 
                  is->nRemainingBuffers()==0,
                  "Object " << lunchbox::className( _object ) <<
                  " did not unpack all data" );

    const uint128_t version = is->getVersion();
    is->reset();
    _slaveCommits.recycle( is );
    return version;
}

void MasterCM::addSlave( Command& command )
{
    LB_TS_THREAD( _cmdThread );
    Mutex mutex( _slaves );
    ObjectCM::_addSlave( command, _version );
}

void MasterCM::_addSlave( NodePtr node )
{
    // OPT: use tr1::unordered_set
    ++_slavesCount[ node->getNodeID() ];
    _slaves->push_back( node );
    stde::usort( *_slaves );
}

void MasterCM::removeSlave( NodePtr node )
{
    LB_TS_THREAD( _cmdThread );
    const NodeID& nodeID = node->getNodeID();

    Mutex mutex( _slaves );
    // remove from subscribers
    LBASSERTINFO( _slavesCount[ nodeID ] != 0, lunchbox::className( _object ));

    --_slavesCount[ nodeID ];
    if( _slavesCount[ nodeID ] == 0 )
    {
        Nodes::iterator i = find( _slaves->begin(), _slaves->end(), node );
        LBASSERT( i != _slaves->end( ));
        _slaves->erase( i );
        _slavesCount.erase( nodeID );
    }
}

void MasterCM::removeSlaves( NodePtr node )
{
    LB_TS_THREAD( _cmdThread );

    const NodeID& nodeID = node->getNodeID();

    Mutex mutex( _slaves );
    SlavesCount::iterator i = _slavesCount.find( nodeID );
    if( i == _slavesCount.end( ))
        return;

    NodesIter j = stde::find( *_slaves, node );
    LBASSERT( j != _slaves->end( ));
    _slaves->erase( j );
    _slavesCount.erase( i );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool MasterCM::_cmdSlaveDelta( Command& command )
{
    LB_TS_THREAD( _rcvThread );
    const ObjectSlaveDeltaPacket* packet = 
        command.get< ObjectSlaveDeltaPacket >();

    if( _slaveCommits.addDataPacket( packet->commit, command ))
        _object->notifyNewVersion();
    return true;
}

}
