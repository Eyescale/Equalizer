
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "objectCM.h"

#include "command.h"
#include "nullCM.h"
#include "node.h"
#include "nodePackets.h"
#include "object.h"
#include "objectInstanceDataOStream.h"
#include "objectPackets.h"

co::ObjectCM* co::ObjectCM::ZERO = new co::NullCM;

#ifdef EQ_INSTRUMENT_MULTICAST
lunchbox::a_int32_t co::ObjectCM::_hit( 0 );
lunchbox::a_int32_t co::ObjectCM::_miss( 0 );
#endif

namespace co
{
ObjectCM::ObjectCM( Object* object )
        : _object( object )
{}

void ObjectCM::push( const uint128_t& groupID, const uint128_t& typeID,
                     const Nodes& nodes )
{
    LBASSERT( _object );
    LBASSERT( !nodes.empty( ));
    if( nodes.empty( ))
        return;

    ObjectInstanceDataOStream os( this );
    os.enablePush( getVersion(), nodes );
    _object->getInstanceData( os );

    NodeObjectPushPacket pushPacket( _object->getID(), groupID, typeID );
    os.disable( pushPacket );
}

void ObjectCM::_addSlave( Command& command, const uint128_t& version )
{
    LBASSERT( version != VERSION_NONE );
    LBASSERT( command->type == PACKETTYPE_CO_NODE );
    LBASSERT( command->command == CMD_NODE_MAP_OBJECT );

    NodePtr node = command.getNode();
    const NodeMapObjectPacket* packet = command.get< NodeMapObjectPacket >();
    const uint128_t& requested  = packet->requestedVersion;

    // Prepare reply packets
    NodeMapObjectSuccessPacket successPacket( packet );
    successPacket.changeType       = _object->getChangeType();
    successPacket.masterInstanceID = _object->getInstanceID();
    successPacket.nodeID = node->getNodeID();

    NodeMapObjectReplyPacket replyPacket( packet );
    replyPacket.nodeID = node->getNodeID();
    replyPacket.version = version;
    replyPacket.result = true;

    // process request
    const uint32_t instanceID = packet->instanceID;
    if( requested == VERSION_NONE ) // no data to send, send empty version
    {
        node->send( successPacket );
        _sendEmptyVersion( node, instanceID, version, false /* mc */ );
        node->send( replyPacket );
        return;
    }

    const bool useCache = packet->masterInstanceID == _object->getInstanceID();
    replyPacket.useCache = packet->useCache && useCache;
    _initSlave( node, requested, packet, successPacket, replyPacket );
}

void ObjectCM::_initSlave( NodePtr node, const uint128_t& version,
                           const NodeMapObjectPacket* packet,
                           NodeMapObjectSuccessPacket& success,
                           NodeMapObjectReplyPacket& reply )
{
#if 0
    LBLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                         << ", instantiate on " << node->getNodeID()
                         << std::endl;
#endif

#ifndef NDEBUG
    if( version != VERSION_OLDEST && version < reply.version )
        LBINFO << "Mapping version " << reply.version << " instead of "
               << version << std::endl;
#endif

    if( reply.useCache && 
        packet->minCachedVersion <= reply.version && 
        packet->maxCachedVersion >= reply.version )
    {
#ifdef EQ_INSTRUMENT_MULTICAST
        ++_hit;
#endif
        node->send( success );
        node->send( reply );
        return;
    }

#ifdef EQ_INSTRUMENT_MULTICAST
    ++_miss;
#endif
    reply.useCache = false;

    if( !node->multicast( success ))
        node->send( success );

    // send instance data
    ObjectInstanceDataOStream os( this );
    const uint32_t instanceID = packet->instanceID;

    os.enableMap( reply.version, node, instanceID );
    _object->getInstanceData( os );
    os.disable();
    if( !os.hasSentData( ))
        // no data, send empty packet to set version
        _sendEmptyVersion( node, instanceID, reply.version, true /* mc */ );

    if( !node->multicast( reply ))
        node->send( reply );
}

void ObjectCM::_sendEmptyVersion( NodePtr node, const uint32_t instanceID,
                                  const uint128_t& version,
                                  const bool multicast )
{
    ObjectInstancePacket packet( NodeID::ZERO, _object->getInstanceID( ));
    packet.type = PACKETTYPE_CO_OBJECT;
    packet.command = CMD_OBJECT_INSTANCE;
    packet.last = true;
    packet.version = version;
    packet.objectID = _object->getID();
    packet.instanceID = instanceID;

    if( !multicast || !node->multicast( packet ))
        node->send( packet );
}

}
