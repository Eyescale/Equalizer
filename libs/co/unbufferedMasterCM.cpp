
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

#include "unbufferedMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "nodePackets.h"
#include "object.h"
#include "objectDataIStream.h"
#include "objectDeltaDataOStream.h"
#include "objectInstanceDataOStream.h"
#include "objectPackets.h"

namespace co
{
typedef CommandFunc<UnbufferedMasterCM> CmdFunc;

UnbufferedMasterCM::UnbufferedMasterCM( Object* object )
        : MasterCM( object )
{
    _version = VERSION_FIRST;
    EQASSERT( object );
    EQASSERT( object->getLocalNode( ));
}

UnbufferedMasterCM::~UnbufferedMasterCM()
{}

void UnbufferedMasterCM::addSlave( Command& command, 
                                   NodeMapObjectReplyPacket& reply )
{
    LB_TS_THREAD( _cmdThread );
    EQASSERT( command->type == PACKETTYPE_CO_NODE );
    EQASSERT( command->command == CMD_NODE_MAP_OBJECT );

    NodePtr node = command.getNode();
    const NodeMapObjectPacket* packet = 
        command.get<const NodeMapObjectPacket>();
    const uint128_t version = packet->requestedVersion;
    const uint32_t instanceID = packet->instanceID;

    {
        Mutex mutex( _slaves );
        reply.version = _version;

        // add to subscribers
        ++_slavesCount[ node->getNodeID() ];
        _slaves->push_back( node );
        stde::usort( *_slaves );

#if 0
        EQLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                             << ", instantiate on " << node->getNodeID()
                             << std::endl;
#endif
    }

    if( version == VERSION_NONE )
    {
        // no data, send empty packet to set version
        _sendEmptyVersion( node, instanceID, reply.version );
        return;
    }

#ifndef NDEBUG
    if( version != VERSION_OLDEST && version < reply.version )
        EQINFO << "Mapping version " << reply.version
               << " instead of requested " << version << std::endl;
#endif

    const bool useCache = packet->masterInstanceID == _object->getInstanceID();

    if( useCache && 
        packet->minCachedVersion <= reply.version && 
        packet->maxCachedVersion >= reply.version )
    {
#ifdef EQ_INSTRUMENT_MULTICAST
        ++_hit;
#endif
        reply.useCache = true;
        return;
    }

#ifdef EQ_INSTRUMENT_MULTICAST
    ++_miss;
#endif

    // send instance data
    ObjectInstanceDataOStream os( this );

    os.enableMap( reply.version, node, instanceID );
    _object->getInstanceData( os );
    os.disable();
    if( os.hasSentData( ))
        return;

    // no data, send empty packet to set version
    _sendEmptyVersion( node, instanceID, reply.version );
}

uint128_t UnbufferedMasterCM::commit( const uint32_t incarnation )
{
    Mutex mutex( _slaves );
#if 0
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command
                         << std::endl;
#endif
    if( !_object->isDirty() || _slaves->empty( ))
        return _version;

    ObjectDeltaDataOStream os( this );
    os.enableCommit( _version + 1, *_slaves );
    _object->pack( os );
    os.disable();

    if( os.hasSentData( ))
    {
        ++_version;
        EQASSERT( _version != VERSION_NONE );
#if 0
        EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                             << _object->getID() << std::endl;
#endif
    }

    return _version;
}

}
